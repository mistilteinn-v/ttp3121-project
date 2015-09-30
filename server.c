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

void conn(int); 
void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]) /* Main function for receiving connection from socket clients */
{

     int num;
     int sin_size; 
    
     int sockfd, newsockfd, portno, pid;

     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     } 
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	error("ERROR on binding");
	

     listen(sockfd,5);
     printf("\nThe server is listening at port :%d\n",portno);
     clilen = sizeof(cli_addr);
     while (1) {newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //receiving multiple connection
         if (newsockfd < 0) 
             error("ERROR on accept");
         pid = fork();
         if (pid < 0)
             error("ERROR on fork");
         if (pid == 0)  {
             close(sockfd);
	printf("\nNew connection established from %s,with socket %d\n", inet_ntoa(cli_addr.sin_addr), sockfd);
             conn(newsockfd);
             exit(0);
         }
         else close(newsockfd);
     } 
     close(sockfd);
     return 0; 
}


void conn (int sock) //this function is responsible to handle all of the separated communication for each for the connection that is established from client.
{
   printf("A client has connected to the Server.\n");
   int count = 0;
   int buflen;
   while(count == 0){

	int n;
	char buffer[256];

	//receiving Client menu input
	bzero(buffer,256);
	n = read(sock, (char*)&buflen, sizeof(buflen));
	if (n < 0) error("ERROR reading from socket");
	buflen = htonl(buflen);
	n = read(sock,buffer,buflen);
	if (n < 0) error("ERROR reading from socket");

	printf("\nClient's input: %s\n",buffer);
	
	if(buffer != NULL){
	if((strcmp(buffer, "1\n")) == 0){	//Client side create file
		count = 0;
	}
	else if((strcmp(buffer, "2\n")) == 0){	//Client download file from Server
	
		printf("Sending file to Client.");
		char buff[256];
		int n;
		
		//Setting directory
		char dir[256] = "/home/";
		char hostname[256];
		gethostname(hostname, 255);
		char file[256] = "/Server/";
		strcat(dir, hostname);
		strcat(dir, file);
		printf("\nPath: %s", dir);
			
		//Retrieving files from directory
		char tempo[256] = "";
		printf("\nAvailable file:\n");
		DIR *directory;
		struct dirent *ent;

		if((directory = opendir("/home/ubuntu/Server/")) != NULL){
		  while((ent = readdir(directory)) != NULL){
		if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0 )
			  {
				strcat(tempo, ent->d_name);
				strcat(tempo, "\n");
				printf("%s\n", ent->d_name);

			  }
		  }
		  closedir(directory);
		}
		else{
		  perror("ERROR");
		  exit(0);
		}


		//Sending files those are available to Client 
		int datalen = strlen(tempo);
		int tmp = htonl(datalen);
		n = write(sock, (char*)&tmp, sizeof(tmp));
		if(n < 0) error("ERROR writing to socket");
		n = write(sock,tempo,datalen);
		if (n < 0) error("ERROR writing to socket");

		//Read the file name that Client wants to download
		char fileRev[256];
		bzero(fileRev,256);
		n = read(sock, (char*)&buflen, sizeof(buflen));
		if (n < 0) error("ERROR reading from socket");
		buflen = htonl(buflen);
		n = read(sock,fileRev,buflen);
		if (n < 0) error("ERROR reading from socket");

		char split[2] = "\n";
		strtok(fileRev, split);
		printf("\nSending file %s to Client... \n", fileRev);
		
		//Sends files to Client
		if(fileRev != NULL){
			strcat(dir, fileRev);
			FILE *fs = fopen(dir, "rb");
			if(fs == NULL){
			  printf("ERROR: File not found.\n");
			  perror("fopen");
			  break;
			}
			else{
			  //Writes file to Client
			  bzero(buff, 256);
			  int fs_block_sz;
			  while((fs_block_sz = fread(buff, sizeof(char), 256, fs)) > 0){
			    if(send(sock, buff, fs_block_sz, 0) < 0){
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
		count = 0;
	}
	else if((strcmp(buffer, "3\n")) == 0){ //Client sending file to Server

		printf("Receiving file from Client... ");
		char revBuff[256];
		
		//Setting directory
		char dir[256] = "/home/";
		char hostname[256];
		gethostname(hostname, 255);
		char file[256] = "/Server/";
		strcat(dir, hostname);
		strcat(dir, file);
		printf("\nPath: %s", dir);
		
		//Create directory if it does not exist
		struct stat st = {0};
		if(stat(dir, &st) == -1){
		  mkdir(dir, 0700);
		}
			
		//Getting file name that is send by Client
		char tempo[256];
		bzero(tempo,256);
		n = read(sock, (char*)&buflen, sizeof(buflen));
		if (n < 0) error("ERROR reading from socket");
		buflen = htonl(buflen);
		n = read(sock,tempo,buflen);
		if (n < 0) error("ERROR reading from socket");

		char split[2] = "\n";
		strtok(tempo, split);

		printf("\nFile saved as: %s\n", tempo);
		
		char filename[256];
		strcpy(filename, tempo);

		//Receives file from Client
		if(filename != NULL){
			strcat(dir, filename);
			printf("\nFile directory: %s", dir);
			FILE *fr = fopen(dir, "w+");
			if(fr == NULL){
			  printf("File cannot be opened");
			  perror("fopen");
			  break;
			}
			else{
			  //Reads file from Client
			  bzero(revBuff, 256);
			  int fr_block_sz = 0;
			  while((fr_block_sz = recv(sock, revBuff, 256, 0)) > 0){
			  	int write_sz = fwrite(revBuff, sizeof(char), fr_block_sz, fr);
				if(write_sz < fr_block_sz){
				  error("File write failed on server.\n");
				}
	 			bzero(revBuff, 256);
				if(fr_block_sz == 0 || fr_block_sz != 256){
				  break;			
				}
			  }
			  printf("\nReceived successfully\n");
			  fclose(fr);
			}
		}		
		else{
			printf("\nERROR: Filename cannot be NULL");		
			printf("\nERROR: Please try again later");
			break;
		}
		count = 0;
	}
	else if((strcmp(buffer, "4\n")) == 0){	//Client perform delete function on Client Side
		count = 0;
	}
	else if((strcmp(buffer, "5\n")) == 0){	//Client request to disconnect from Server 
		printf("Client is disconnecting from the Server... ");
		count = 1;
	}
	else{	//Invalid input 
		printf("\nClient inserted wrong input.\n\n");
		count = 0;
		break;
	}
	}

   }
}
