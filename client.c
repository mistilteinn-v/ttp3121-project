#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <signal.h>
#include <ctype.h>          
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <dirent.h>

void error(const char *msg){
    perror(msg);
    exit(0);
}

void createFile(int sockfd){	//Create file on Client side
  	printf("Create a file.");
	
	//Setting directory
	char content[256];
	char dir[256] = "/home/";
	char hostname[256];
	gethostname(hostname, 255);
	char file[256] = "/Client/";
	strcat(dir, hostname);
	strcat(dir, file);
	printf("\nPath: %s", dir);
	
	//Create new directory if does not exist	
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}
	
	char filename[256];
	printf("\nPlease enter file name: ");
	fgets(filename, 256, stdin);
	
	if(filename != NULL){
		strcat(dir, filename);
		printf("File location: %s", dir);
		//Create file
		FILE *fp;
		fp = fopen(dir, "w+");
		if(fp == NULL){
		  printf("\nERROR: File cannot be created");
		  perror("fopen");	
		  exit(0);	
		}
		else{	//Content
		  printf("Please insert the content here: ");
		  fgets(content, 256, stdin);
		  printf("Content: %s", content);
		 
		  fprintf(fp, "%s", content);	//Write operation on the file
		  fclose(fp);
		  printf("\nFile created successfully!");
		}
	}
	else{
		printf("\nERROR: Filename cannot be NULL");		
		printf("\nERROR: Please try again later");
		exit(0);
	}
}

void downloadFile(int sockfd){	//Client download file from Server
	printf("Download file.");
	
	int n;
	int buflen;


	//Setting directory
	char revBuff[256];
	char dir[256] = "/home/";
	char hostname[256];
	gethostname(hostname, 255);
	char file[256] = "/Client/";
	strcat(dir, hostname);
	strcat(dir, file);
	printf("\nDownloading to path: %s", dir);
	
	//Create new directory if does not exist
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}

	//Getting files those are available from the Server directory then display them
	char tempo[256]="";
	tempo[0]= 0;
	char *s = tempo, *token;
	//bzero(tempo,256);
	n = read(sockfd, (char*)&buflen, sizeof(buflen));
	if (n < 0) error("ERROR reading from socket");
	buflen = htonl(buflen);
	n = read(sockfd,tempo,buflen);
	if (n < 0) error("ERROR reading from socket");
	printf("\nAvailable file: \n");
	printf("%s", tempo);

	if(strlen(tempo) == 0)
	{
	printf("There is no file on the server.\n");
	return;
	}	

	printf("Please enter the file name that you wanted to download: ");
	char selectFile[256];
	bzero(selectFile,256);
	fgets(selectFile,255,stdin);
    	char input[256];
	
	//Display a list of filename from the Server
	int datalen = strlen(selectFile);
	int tmp = htonl(datalen);
	n = write(sockfd, (char*)&tmp, sizeof(tmp));
	if(n < 0) error("ERROR writing to socket");
	n = write(sockfd,selectFile,datalen);
	if (n < 0) error("ERROR writing to socket");
	
	char filename[256];
	printf("Save file name as: ");
	fgets(filename, 256, stdin);

	if(filename[0] != '\n'){
		strcat(dir, filename);	//Concatenate directory and filename
		printf("Saving to file location: %s", dir);

		FILE *fr = fopen(dir, "ab");
		if(fr == NULL){
		  printf("File cannot be opened");
		  perror("fopen");
		  exit(0);
		}
		else{	
			
		int cout = 0;
		int isDledFile = 0;
		while ((token = strtok(s, "\n")) != NULL) {
	        s = NULL;
		cout = 1;
		strcat(token,"\n");
    
			if (strcmp(token,selectFile) ==0)
			{
			  bzero(revBuff, 256);
			  int fr_block_sz = 0;
			  isDledFile = 1;				
	
			  while((fr_block_sz = recv(sockfd, revBuff, 256, 0)) > 0){
			  	int write_sz = fwrite(revBuff, sizeof(char), fr_block_sz, fr);
					if(write_sz < fr_block_sz){
					  error("File write failed on server.\n");
					}
				bzero(revBuff, 256);
					if(fr_block_sz == 0 || fr_block_sz != 256){
					  break;			
					}
			  }
		  printf("\nFile downloaded successfully");
		  fclose(fr);
		  
			}
		}
			if (cout == 0) //if no files
			{	
			printf("\nERROR: File does not exist.\n");
			exit(0);
			}else if (isDledFile == 0) //incorrect name
			{
			printf("\nERROR: File does not exist.\n");
			exit(0);
			}

		}
	}
	else{
		printf("\nERROR: Filename cannot be NULL");		
		printf("\nERROR: Please try again later\n");
		exit(0);
	}
}

void sendFile(int sockfd){	//Client sends file to Server

	printf("Send file operation.");
	char buff[256];
	int n;
	
	//Setting directory
	char dir[256] = "/home/";
	char hostname[256];
	gethostname(hostname, 255);
	char file[256] = "/Client/";
	strcat(dir, hostname);
	strcat(dir, file);
	printf("\nPath: %s", dir);
	
	//Create new directory if does not exist
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}
	
	//Getting files those are available from the directory then display them
	printf("\nAvailable file: \n");
	DIR *directory;
	struct dirent *ent;
	if((directory = opendir(dir)) != NULL){
	  while((ent = readdir(directory)) != NULL){
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 )
		  {
			printf("%s", ent->d_name);

		  }
	  }
	  closedir(directory);
	}
	else{
	  perror("ERROR");
	  exit(0);
	}
	
	//Typing a filename to be sent to the server
	char tempo[256];
	printf("\nPlease enter the file name that you want to send: ");
	fgets(tempo, 256, stdin);
	char filename[256];
	strcpy(filename, tempo); 
	
	if(filename != NULL){

		//Proceeding with the file name to the Server
		int datalen = strlen(tempo);
		int tmp = htonl(datalen);
		n = write(sockfd, (char*)&tmp, sizeof(tmp));
		if(n < 0) error("ERROR writing to socket");
		n = write(sockfd,tempo,datalen);
		if (n < 0) error("ERROR writing to socket");
	
		char split[2] = "\n";
	 	strtok(tempo, split);

		strcat(dir, filename);
		printf("Sending %s to Server... ", tempo);
		printf("\nDir: %s", dir);
	
		FILE *fs = fopen(dir, "rb");	//Read file
		if(fs == NULL){
		  printf("\nERROR: File not found.\n");
		  perror("fopen");
		  exit(0);
		}
		else{	//Sending file to Server
		  bzero(buff, 256);
		  int fs_block_sz;
		  while((fs_block_sz = fread(buff, sizeof(char), 256, fs)) > 0){
		    if(send(sockfd, buff, fs_block_sz, 0) < 0){
			fprintf(stderr, "ERROR: Failed to send file. %d", errno);
			break;
		    }
		    bzero(buff, 256);
		  }
		  printf("\nFile sent successfully!\n");
		  fclose(fs);
		}
	}
	else{
		printf("\nERROR: Filename cannot be NULL");		
		printf("\nERROR: Please try again later");
		exit(0);
	}
		
}

void deleteFile(int sockfd){	//Deleting file on Client side
	printf("Delete file operation.");
	
	//Setting directory
	char content[256];
	char dir[256] = "/home/";
	char hostname[256];
	gethostname(hostname, 255);
	char file[256] = "/Client/";
	strcat(dir, hostname);
	strcat(dir, file);
	printf("\nPath: %s", dir);
	
	//Create new directory if does not exist	
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}

	//Getting files those are available from the directory then display them
	printf("\nAvailable file: \n");
	DIR *directory;
	struct dirent *ent;
	if((directory = opendir(dir)) != NULL){
	  while((ent = readdir(directory)) != NULL){

		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 ) //removing extra "." and ".."
			  {		
				printf("%s", ent->d_name);
			  }
	  }
	  closedir(directory);
	}
	else{
	  perror("ERROR");
	  exit(0);
	}

	//Input filename to delete
	char filename[256];
	printf("\nPlease enter the file name that you want to delete: ");
	fgets(filename, 256, stdin);

	
	if(filename != NULL){

		strcat(dir, filename);
		FILE *fp;
		
		//File available for deletion
		fp = fopen(dir, "r");
		if(fp == NULL){
		  printf("\nERROR: File cannot be deleted");
		  perror("fopen");	
		}
		else{	//Deleting file
		  int status = remove(dir);
		  if(status == 0){
			printf("\nFile deleted successfully!");
			fclose(fp);
		  }else{
			printf("\nERROR: Unable to delete the file");
			exit(0);
		  }
		}
	}
}

int main(int argc, char *argv[])	//Main function, starts with hostname and port argument
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;


    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("\n\nWelcome! You are now connected to the Server!");

	if(1)
{
    int count = 0;
    while(count == 0){	//Getting Client's choice
	int proceed = 0;
	printf("\nPlease proceed with selection:\n1.Create a new File. \n2.Download a file. \n3.Send file to server. \n4.Delete a file. \n5.Terminate connection.");
	printf("\nEnter: ");
	bzero(buffer,256);
	fgets(buffer,255,stdin);
    char input[256];
	strcpy(input, buffer);
	
	//Sending Client's choice to Server
	int datalen = strlen(buffer);
	int tmp = htonl(datalen);
	n = write(sockfd, (char*)&tmp, sizeof(tmp));
	if(n < 0) error("ERROR writing to socket");
	n = write(sockfd,buffer,datalen);
	if (n < 0) error("ERROR writing to socket");
	

	if((strcmp(input, "1\n")) == 0){	//Create file on Client side
	   createFile(sockfd);
	   count = 0;
	}
	else if((strcmp(input, "2\n")) == 0){	//Download file from Server
	   downloadFile(sockfd);
	   count = 0;
	}
	else if((strcmp(input, "3\n")) == 0){	//Client send file to Server
	   sendFile(sockfd);
	   count = 0;
	}
	else if((strcmp(input, "4\n")) == 0){	//Delete file on Client side
	   deleteFile(sockfd);
	   count = 0;
	}
	else if((strcmp(input, "5\n")) == 0){	//Client disconnects
	   count = 1;
	   proceed = 1;
	}
	else{
	   printf("\nWrong input, please try again.");	//Invalid input from Client
	   count = 0;
	}
    }
}
	
    close(sockfd);
    printf("\nYou have now disconnected from the Server.\n\n");
    return 0;
}
