#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>


#define MAXFILES 50
#define FILESIZE 1024 

#define READ 4
#define WRITE 2

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

void restoreData();

struct SuperBlock
{
	int total_inode;
	int free_inode;
}SUPERBLOCK;

typedef struct Inode
{
	char file_name[32];
	int inode;
	int file_size;
	int file_type;
	int actual_size;
	int link_count;
	int reference_count;
	int mode;
	char *data;
	struct Inode *next;
}INODE, *PINODE, **PPINODE;

typedef struct FileTable
{
	int mode;
	int readoffset;
	int writeoffset;
	int count;
	PINODE INODEPTR;
}FILETABLE, *PFILETABLE;

struct Ufdt
{
	PFILETABLE ufdt[MAXFILES];
}UFDT;

PINODE head = NULL;

void createDILB()
{
	PINODE temp = head;
	PINODE newn = NULL;
	int i =1;

	while(i <= MAXFILES)
	{
		newn = (PINODE)malloc(sizeof(INODE));

		newn->inode = i;
		newn->file_size = FILESIZE;
		newn->file_type = 0;
		newn->actual_size = 0;
		newn->link_count = 0;
		newn->reference_count = 0;
		newn->mode = 0;
		newn->data = NULL;
		newn->next = NULL;

		if(head == NULL)
		{
			head = newn;
			temp = head;
		}
		else
		{
			temp->next = newn;
			temp = temp->next;
		}
		i++;
	}
}

void createSuperBlock()
{
	SUPERBLOCK.total_inode = MAXFILES;
	SUPERBLOCK.free_inode = MAXFILES;
}

void createUFDT()
{
	int i = 0;
	for(i = 0;i < MAXFILES;i++)
	{
		UFDT.ufdt[i] = NULL;
	}
}

void Help()
{
	printf("-----------------------------------------------------\n");
	printf("open	: It is used to open the existing file\n");
	printf("close	: It is used to close the opened file\n");
	printf("read	: It is used to read the contents of file\n");
	printf("write	: It is used to write the data into file\n");
	printf("lseek	: It is used to change the offset of file\n");
	printf("stat	: It is used to display the information of file\n");
	printf("ls	: It is used to List out all files\n");
	printf("clear	: It is used to clear console\n");
	printf("closeall: It is used to close all open files\n");
	printf("exit	: It is used to terminate file system\n");
	printf("truncate: It is used to remove all data from file\n");
	printf("delete	: It is used to delete the file\n");
	printf("-----------------------------------------------------\n");
}

void manPage(char *name)
{
	if(name == NULL)
	{
		return;
	}
	if(strcmp(name,"create") == 0)
	{
		printf("Description : Used to create new regular file\n");
		printf("Usage : create File_name Permission\n");
	}
	else if(strcmp(name,"read") == 0)
	{
		printf("Description : used to read the contents of file\n");
		printf("Usage : read file_name number_of_bytes_to_read\n");
	}
	else if(strcmp(name,"write") == 0)
	{
		printf("Description : used to write the data into file\n");
		printf("Usage : write File_name\n");
	}
	else if(strcmp(name,"ls") == 0)
	{
		printf("Description : Used to list computer files\n");
		printf("Usage : ls\n");
	}
	else if(strcmp(name,"stat") == 0)
	{
		printf("Description : used to display the information of file\n");
		printf("Usage : stat file_name\n");
	}
	else if(strcmp(name,"truncate") == 0)
	{
		printf("Description : shrink or extend the size of a file to the specified size\n");
		printf("Usage : truncate file_name\n");
	}
	else if(strcmp(name,"open") == 0)
	{
		printf("Description : used to open the existing file\n");
		printf("Usage : open file_name mode\n");
	}
	else if(strcmp(name,"close") == 0)
	{
		printf("Description : used to close the opened file\n");
		printf("Usage : close file_name\n");
	}
	else if(strcmp(name,"closeall") == 0)
	{
		printf("Description : Used to close all open files\n");
		printf("Usage : closeall\n");
	}
	else if(strcmp(name,"lseek") == 0)
	{
		printf("Description : used to change the offset of file\n");
		printf("Usage : lseek File_Name ChangeInOffset StartPoint\n");
	}
	else if(strcmp(name,"delete") == 0)
	{
		printf("Description : Used to create new regular file\n");
		printf("Usage : delete file_name\n");
	}
	else
	{
		printf("ERROR : No manual entry available.\n");
	}
}

void setEnv()
{
	createDILB();
	createUFDT();
	createSuperBlock();
	restoreData();
	printf("Environment is set for VFS\n");
}

bool checkFile(char *name)
{
	PINODE temp = head;

	while(temp != NULL)
	{
		if(temp->file_type != 0)
		{
			if(strcmp(name,temp->file_name) == 0)
			{
				break;
			}
		}
		temp = temp->next;
	}

	if(temp == NULL)
	{
		return false;
	}
	else
	{
		return true;
	}
}

int creatFile(char *name,int permission)
{
	if(name == NULL)
	{
		return -1;
	}

	if((permission > READ+WRITE) || (permission < WRITE))
	{
		return -2;
	}

	bool isChk = false;
	isChk = checkFile(name);

	if(isChk == true)
	{
		return -3;
	}

	int i = 0;
	for(i = 0;i < MAXFILES;i++)
	{
		if(UFDT.ufdt[i] == NULL)
		{
			break;
		}
	}

	if(i == MAXFILES)
	{
		return -4;
	}

	if(SUPERBLOCK.free_inode == 0)
	{
		return -5;
	}

	UFDT.ufdt[i] = (PFILETABLE)malloc(sizeof(FILETABLE));

	UFDT.ufdt[i]->mode = permission;
	UFDT.ufdt[i]->readoffset = 0;
	UFDT.ufdt[i]->writeoffset = 0;
	UFDT.ufdt[i]->count = 1;

	PINODE temp = head;

	while(temp != NULL)
	{
		if(temp->file_type == 0)
		{
			break;
		}
		temp = temp->next;
	}

	UFDT.ufdt[i]->INODEPTR = temp;
	strcpy(UFDT.ufdt[i]->INODEPTR->file_name,name);
	UFDT.ufdt[i]->INODEPTR->file_size = FILESIZE;
	UFDT.ufdt[i]->INODEPTR->file_type = REGULAR;
	UFDT.ufdt[i]->INODEPTR->actual_size = 0;
	UFDT.ufdt[i]->INODEPTR->link_count = 1; 
	UFDT.ufdt[i]->INODEPTR->reference_count = 1;
	UFDT.ufdt[i]->INODEPTR->mode = permission;
	UFDT.ufdt[i]->INODEPTR->data = (char *)malloc(sizeof(FILESIZE));

	SUPERBLOCK.free_inode--;
	return i;
}

int openFile(char *name,int permission)
{
	if(name == NULL)
	{
		return -1;
	}

	if((permission > READ+WRITE) || (permission < WRITE))
	{
		return -2;
	}

	bool isChk = false;
	isChk = checkFile(name);

	if(isChk == false)
	{
		return -3;
	}

	PINODE temp = head;

	while(temp != NULL)
	{
		if(strcmp(temp->file_name,name) == 0)
		{
			break;
		}
		temp = temp->next;
	}

	int i = 0;
	for(i = 0;i < MAXFILES;i++)
	{
		if(UFDT.ufdt[i] == NULL)
		{
			break;
		}
	}

	if(i == MAXFILES)
	{
		return -4;
	}

	UFDT.ufdt[i] = (PFILETABLE)malloc(sizeof(FILETABLE));

	if(UFDT.ufdt[i] == NULL)
	{
		return -4;
	}

	if(temp->mode != permission)
	{
		return -2;
	}

	UFDT.ufdt[i]->mode = permission;
	UFDT.ufdt[i]->count = 1;

	if(permission == READ+WRITE)
	{
		UFDT.ufdt[i]->readoffset = 0;
		UFDT.ufdt[i]->writeoffset = 0;
	}
	else if(permission == READ)
	{
		UFDT.ufdt[i]->readoffset = 0;
	}
	else
	{
		UFDT.ufdt[i]->writeoffset = 0;
	}

	UFDT.ufdt[i]->INODEPTR = temp;
	(UFDT.ufdt[i]->INODEPTR->reference_count)++;

	return i;
}

int readFile(char *fname,char *fdata,int size)
{
	int i = 0,j = 0,k = 0;

	while(i < MAXFILES)
	{
		if(UFDT.ufdt[i] != NULL)
		{
			if(strcmp(UFDT.ufdt[i]->INODEPTR->file_name,fname) == 0)
			{
				break;
			}
		}
		i++;
	}
	if(i == MAXFILES)
	{
		return -1;
	}

	if(UFDT.ufdt[i]->INODEPTR == NULL)
	{
		return -1;
	}
	if(UFDT.ufdt[i]->mode == WRITE || UFDT.ufdt[i]->INODEPTR->mode == WRITE)
	{
		return -2;
	}
	if(UFDT.ufdt[i]->INODEPTR->file_type != REGULAR)
	{
		return -3;
	}
	if(UFDT.ufdt[i]->readoffset >= FILESIZE)
	{
		return -4;
	}
	if(UFDT.ufdt[i]->INODEPTR->reference_count == 0)
	{
		return -5;
	}

	for(j = UFDT.ufdt[i]->readoffset,k = 0;(k < size) && (j < UFDT.ufdt[i]->INODEPTR->actual_size);j++,k++)
	{
		fdata[k] = UFDT.ufdt[i]->INODEPTR->data[j];
	}
}

int writeFile(char *fname,char *fdata,int size)
{
	int i = 0,j = 0,k = 0;

	while(i < MAXFILES)
	{
		if(UFDT.ufdt[i] != NULL)
		{
			if(strcmp(UFDT.ufdt[i]->INODEPTR->file_name,fname) == 0)
			{
				break;
			}
		}
		i++;
	}

	if(i == MAXFILES)
	{
		return -1;
	}
	if(UFDT.ufdt[i]->mode == READ || UFDT.ufdt[i]->INODEPTR->mode == READ)
	{
		return -2;
	}
	if(UFDT.ufdt[i]->INODEPTR->file_type != REGULAR)
	{
		return -3;
	}
	if(UFDT.ufdt[i]->writeoffset >= FILESIZE)
	{
		return -4;
	}
	if(UFDT.ufdt[i]->INODEPTR->reference_count == 0)
	{
		return -5;
	}

	for(j = UFDT.ufdt[i]->writeoffset,k = 0;(k < size) && (j < FILESIZE);j++,k++)
	{
		UFDT.ufdt[i]->INODEPTR->data[j] = fdata[k];
	}
	UFDT.ufdt[i]->INODEPTR->actual_size += size;	
}

void statFile(char *name)
{

	if(name == NULL)
	{
		printf("Missing file name\n");
		return ;
	}
	PINODE temp = head;

	while(temp != NULL)
	{
		if(strcmp(temp->file_name,name) == 0)
		{
			break;
		}
		temp = temp->next;
	}
	if(temp == NULL)
	{
		printf("No such file\n");
		return;
	}
	printf("File Name = %s\n",temp->file_name);
	printf("File Size = %d\n",temp->actual_size);
	if(temp->file_type == REGULAR)
	{
		printf("File Type = Regular File\n");
	}
	else
	{
		printf("File Type = Special File\n");
	}
	printf("link count = %d\n",temp->link_count);
	printf("reference_count = %d\n",temp->reference_count);
	printf("Inode Number = %d\n",temp->inode);
	if(temp->mode == 6)
	{
		printf("File access mode = READ+WRITE\n");
	}
	else if(temp->mode == 4)
	{
		printf("File access mode = READ\n");
	}
	else
	{
		printf("File access mode = WRITE\n");
	}
}

int closeFile(int fd)
{
	if(UFDT.ufdt[fd] == NULL)
	{
		return -1;
	}

	if(UFDT.ufdt[fd]->INODEPTR->reference_count == 0)
	{
		return -1;
	}
	else
	{
		(UFDT.ufdt[fd]->INODEPTR->reference_count)--;
		UFDT.ufdt[fd] = NULL;
	}

	return 0;
}

void closeallFiles()
{
	int i = 0;

	for(i = 0;i < MAXFILES;i++)
	{
		if(UFDT.ufdt[i] != NULL)
		{
			(UFDT.ufdt[i]->INODEPTR->reference_count)--;
			UFDT.ufdt[i] = NULL;
		}
	}
}

int deleteFile(char *name)
{
	int i = 0;

	while(i < MAXFILES)
	{
		if(UFDT.ufdt[i] != NULL)
		{
			if(strcmp(UFDT.ufdt[i]->INODEPTR->file_name,name) == 0)
			{
				break;
			}
		}
		i++;
	}

	if(i == MAXFILES)
	{
		return -1;
	}

	(UFDT.ufdt[i]->INODEPTR->reference_count)--;
	if(UFDT.ufdt[i]->INODEPTR->reference_count == 0)
	{
		UFDT.ufdt[i]->INODEPTR->file_type = 0;
		free(UFDT.ufdt[i]->INODEPTR->data);
	}
	free(UFDT.ufdt[i]->INODEPTR);
	UFDT.ufdt[i] = NULL;

	(SUPERBLOCK.free_inode)--;

	return i;
}

int lseekFile(int fd,int size,int from)
{
	if(fd < 0)
	{
		return -1;
	}
	if(UFDT.ufdt[fd] == NULL)
	{
		return -2;
	}
	if(from < 0 && from > 2)
	{
		return -3;
	}

	if(UFDT.ufdt[fd]->mode != WRITE)
	{
		if(from == START)
		{
			if(size > UFDT.ufdt[fd]->INODEPTR->actual_size)
			{
				return -4;
			}
			if(size < 0)
			{
				return -4;
			}
			UFDT.ufdt[fd]->readoffset = size;
		}
		else if(from == CURRENT)
		{
			if((UFDT.ufdt[fd]->readoffset + size) > UFDT.ufdt[fd]->INODEPTR->actual_size)
			{
				return -4;
			}
			if ((UFDT.ufdt[fd]->readoffset + size) < 0)
			{
				return -4;
			}
			UFDT.ufdt[fd]->readoffset = UFDT.ufdt[fd]->readoffset + size;
		}
		else
		{
			if(UFDT.ufdt[fd]->readoffset + size > FILESIZE)
			{
				return -4;
			}
			if(UFDT.ufdt[fd]->readoffset + size < 0)
			{
				return -4;
			}
			UFDT.ufdt[fd]->readoffset = UFDT.ufdt[fd]->INODEPTR->actual_size + size;
		}
	}

	if (UFDT.ufdt[fd]->mode != READ)
	{
		if(from == START)
		{
			if(size > UFDT.ufdt[fd]->INODEPTR->actual_size)
			{
				return -4;
			}
			if(size < 0)
			{
				return -4;
			}
			UFDT.ufdt[fd]->writeoffset = size;
		}
		else if(from == CURRENT)
		{
			if((UFDT.ufdt[fd]->writeoffset + size) > UFDT.ufdt[fd]->INODEPTR->actual_size)
			{
				return -4;
			}
			if ((UFDT.ufdt[fd]->writeoffset + size) < 0)
			{
				return -4;
			}
			UFDT.ufdt[fd]->writeoffset = UFDT.ufdt[fd]->writeoffset + size;
		}
		else
		{
			if(UFDT.ufdt[fd]->writeoffset + size > FILESIZE)
			{
				return -4;
			}
			if(UFDT.ufdt[fd]->writeoffset + size < 0)
			{
				return -4;
			}
			UFDT.ufdt[fd]->writeoffset = UFDT.ufdt[fd]->INODEPTR->actual_size+size;
		}
	}
	return 0;
}

int truncateFile(char *fname,int len)
{
	int ret = 0;

	PINODE temp = head;

	if(fname == NULL)
	{
		return -1;
	}
	if(len < 0 || len > FILESIZE)
	{
		return -2;
	}
	while(temp != NULL)
	{	
		if(strcmp(temp->file_name,fname) == 0)
		{
			break;
		}
		temp = temp->next;
	}
	if(temp == NULL)
	{
		return -3;
	}
	if(temp->file_type == 0)
	{
		return -3;
	}

	if(len < temp->actual_size)
	{
		temp->actual_size = len;
	}
	else
	{
		int i = 0;
		for(i = temp->actual_size;i < len;i++)
		{
			temp->data[i] = '\0';
		}
		temp->actual_size += len;
	}

	return 0;
}
void ls()
{
	PINODE temp = head;

	while(temp != NULL)
	{
		if(temp->file_type != 0)
		{
			printf("%s\n",temp->file_name);
		}
		temp = temp->next;
	}
}

int backupData()
{
	int fd = 0,written = 0;
	fd = open("Back.txt",O_RDWR | O_CREAT);
	if(fd == -1)
	{
		return -1;
	}
	PINODE temp = head;
	while(temp != NULL)
	{
		if(temp->file_type != 0)
		{
			written = 1;
			write(fd,temp,sizeof(INODE));	
			write(fd,temp->data,FILESIZE);
		}
		temp = temp->next;
	}

	if(written!=1)
	{
		ftruncate(fd,0);
	}
	return 0;
}

void restoreData()
{
	int fd = open("Back.txt",O_RDWR);
	if(fd == -1)
	{
		return;
	}
	PINODE temp = head;
	INODE newn;
	int ret = 0;

	int size = lseek(fd,0,SEEK_END);
	if(size==0)
	{
		return;
	}

	lseek(fd,0,SEEK_SET);
	char fdata[FILESIZE];

	while(ret = (read(fd,&newn,sizeof(INODE))) > 0)
	{
		strcpy(temp->file_name,newn.file_name);
		temp->inode = newn.inode;
		temp->file_size = newn.file_size;
		temp->file_type = newn.file_type;
		temp->actual_size = newn.actual_size;
		temp->link_count = newn.link_count;
		temp->reference_count = newn.reference_count;
		temp->mode = newn.mode;

		read(fd,fdata,FILESIZE);

		temp->data = (char *)malloc(FILESIZE);

		if(temp->actual_size==0)
		{
			temp=temp->next;
			continue;
		}

		memcpy(temp->data,fdata,FILESIZE);

		SUPERBLOCK.free_inode--;
		temp = temp->next;
	}
	close(fd);
	printf("Data Restored\n");
}

int main()
{
	char str[80];
	char command[5][80];
	int count = 0,r = 0;

	printf("\nCustomised Virtual File System\n");
	setEnv();
	
	while(1)
	{
		printf("\nMarvel VFS $ ");
		scanf(" %[^'\n']s",str);
		fflush(stdin);
		count = sscanf(str,"%s %s %s %s %s",command[0],command[1],command[2],command[3],command[4]);

		if(count == 1)
		{
			if(strcmp(command[0],"exit") == 0)
			{
				int ret = 0;
				ret = backupData();
				break;
			}
			else if(strcmp(command[0],"clear") == 0)
			{
				system("clear");
			}
			else if(strcmp(command[0],"help") == 0)
			{
				Help();
			}
			else if(strcmp(command[0],"man") == 0)
			{
				printf("What manual page you want?");
			}
			else if(strcmp(command[0],"ls") == 0)
			{
				ls();
				continue;
			}
			else if(strcmp(command[0],"closeall") == 0)
			{
				closeallFiles();
			}
			else
			{
				printf("Command not found !!\n");
			}
		}
		else if(count == 2)
		{
			if(strcmp(command[0],"man") == 0)  
			{
				manPage(command[1]);
			}
			else if(strcmp(command[0],"write") == 0)
			{
				int ret = 0;
				char arr[FILESIZE];

				printf("Enter data\t");
				scanf(" %[^'\n']s",arr);

				ret = writeFile((command[1]),arr,strlen(arr));
				if(ret == -1)
				{
					printf("File not found\n");
				}
				else if(ret == -2)
				{
					printf("Permission denied\n");
				}
				else if(ret == -3)
				{
					printf("File is not regular\n");
				}
				else if(ret == -4)
				{
					printf("File Max size reached\n");
				}
				else if(ret == -5)
				{
					printf("File is not open\n");
				}
				else
				{
					printf("data write successfully\n");
				}
			}
			else if(strcmp(command[0],"stat") == 0)
			{
				statFile(command[1]);
			}
			else if(strcmp(command[0],"close") == 0)
			{
				int ret = 0;
				ret = closeFile(atoi(command[1]));

				if(ret == -1)
				{
					printf("File is not opened\n");
				}
				else
				{
					printf("File close successfully\n");
				}
			}
			else if(strcmp(command[0],"delete") == 0)
			{
				int ret = 0;
				ret = deleteFile(command[1]);

				if(ret == -1)
				{
					printf("File not found\n");
				}
				else
				{
					printf("File deleted successfully\n");
				}
			}
			else
			{
        		printf("Command not found !!\n");
			}
		}
		else if(count == 3)
		{
			
			if(strcmp(command[0],"create") == 0)
			{
				int fd = 0;
				fd = creatFile(command[1],atoi(command[2]));

				if(fd == -1)
				{
					printf("Cannot create file\n");
				}
				else if(fd == -2)
				{
					printf("Permission denied\n");
				}
				else if(fd == -3)
				{
					printf("File already present\n");					
				}
				else if(fd == -4)
				{
					printf("Unable to file Description\n");
				}
				else if(fd == -5)
				{
					printf("There is no inode to create new files\n");
				}
				else
				{
					printf("File create successfully\n");
				}
			}
			else if(strcmp(command[0],"open") == 0)
			{
				int fd = 0;
				fd = openFile(command[1],atoi(command[2]));
				
				if(fd == -1)
				{
					printf("Cannot create file\n");
				}
				else if(fd == -2)
				{
					printf("Permission denied\n");
				}
				else if(fd == -3)
				{
					printf("File not found\n");					
				}
				else if(fd == -4)
				{
					printf("Unable to allocacte file Description\n");					
				}
				else
				{
					printf("File open successfully with fd %d\n",fd);
				}
			}
			else if(strcmp(command[0],"read") == 0)
			{
				int ret = 0;
				char arr[FILESIZE]={'\0'};

				ret = readFile((command[1]),arr,atoi(command[2]));
				int i = 0;

				if(ret == -1)
				{
					printf("File not found\n");
				}
				else if(ret == -2)
				{
					printf("Permission denied\n");
				}
				else if(ret == -3)
				{
					printf("File is not regular\n");
				}
				else if (ret == -4)
				{
					printf("File max size reached\n");
				}
				else if (ret == -5)
				{
					printf("File is not open\n");
				}
				else
				{
					printf("%s\n",arr);
				}
			}
			else if (strcmp(command[0],"truncate") == 0)
			{
				int ret = 0;

				ret = truncateFile(command[1],atoi(command[2]));

				if(ret == -1)
				{
					printf("Enter valid file name\n");
				}
				else if (ret == -2)
				{
					printf("Enter valid length\n");
				}
				else if (ret == -3)
				{
					printf("File not found\n");
				}
				else
				{
					printf("Truncate successfully\n");
				}
			}
			else
			{
				printf("Command not found !! \n");
			}
		}
		else if(count == 4)
		{
			int ret = 0;
			ret = lseekFile(atoi(command[1]),atoi(command[2]),atoi(command[3]));

			if(ret == -1)
			{
				printf("ERROR\n");
			}
			else
			{
				printf("file seek\n");
			}
		}
		else
		{
			printf("Command not found !! \n");
		}
		 
	}
	return 0;
}
