/*
==============================================================================================================

Name             : proxy.c
Author           : Read L. Ballew Jr.
Course Section   : CSCE 3530.001
Date             : March 22nd, 2019
Description      : The purpse of this program is to receive a website address and either served the cached
				   cached page, or the page from the web server if this proxy server has not cached it yet,
				   then cache the 5 most frequently a

				   Compilation: gcc -o server proxy.c
    			   Execution  : ./proxy "Insert_Port_Of_Choice"

==============================================================================================================
*/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

bool cacheCheck(char*);
bool statusCodeConfirm();

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen, n;
   	struct sockaddr_in serv_addr, cli_addr;
    char buffer[256];
	bool checkerino = true;
	bool statusCheck = false;

	if(argc < 2)
	{
		printf("\nPort number is missing...\n");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		error(EXIT_FAILURE, 0, "ERROR opening socket");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		error(EXIT_FAILURE, 0, "ERROR binding");
	}

	printf("\nServer Started and listening to the port %d\n", portno);
    listen(sockfd, 5);

	while(1)
	{
		//Connecting with the client
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0)
		{
			error(EXIT_FAILURE, 0, "ERROR on accept");
		}
		else
		{
			checkerino = true;
		}
		printf("\nClient is connected...\n");

		while(checkerino)
		{
			//Receiving the message from the client
			bzero(buffer, 256);
			n = read(newsockfd, buffer, 255);
			if(n < 0)
			{
				error(EXIT_FAILURE, 0, "ERROR reading from socket");
			}
			else
			{
				char websiteName[256];
				char tempURL[256];
				char html[10000];
				printf("\nClient has requested: %s\n", buffer);
				strcpy(websiteName, buffer);

				strcpy(tempURL, buffer);
				tempURL[strlen(tempURL) - 1] = '/';
				
				//Get the webpage

				//write

				//read


				//Check the tempfile, update statusCheck:: checks header.txt for 200 OK code
				//statusCheck = statusCodeConfirm();

				//If 200 OK is found in the file
				if(statusCheck)
				{
					printf("\nStatus Code is 200 OK, web page will be cached.\n");

					//Page needs to be cached
					if(!cacheCheck(buffer))
					{
						//Add webpage to list
						FILE *fptr;
						char filename[9];
						strcpy(filename, "list.txt");
						fptr = fopen(filename, "a");
						fputs(websiteName, fptr);
						fclose(fptr);

						//Cache webpage ----------------------------------------------------------------------------------------------
						strcpy(websiteName, ".html");
						char *const cmd[] = {"curl", "-v", buffer, ">", websiteName, NULL};
						execvp(cmd[0], cmd);

						//Create package to send to client
						char *line;
						size_t length = 256;
						char fileChar;
						strcpy(filename, websiteName);
						fptr = fopen(filename, "r");
						fileChar = fgetc(fptr);
						while (fileChar != EOF)
						{
							getline(&line, &length, fptr);
							strcat(html, line);
						}
						fclose(fptr);
					}
					else
					{
						strcpy(websiteName, ".html");
						//Create package to send to client
						FILE *fptr;
						char filename[9];
						char *line;
						size_t length = 256;
						char fileChar;
						strcpy(filename, websiteName);
						fptr = fopen(filename, "r");
						fileChar = fgetc(fptr);
						while (fileChar != EOF)
						{
							getline(&line, &length, fptr);
							strcat(html, line);
						}
						fclose(fptr);
					}
				}
				else
				{
					//Status code was not 200 OK, page does not need to be cached
					//Webpage will be sent to the client
					//Create temp html file -------------------------------------------------------------------------------------
					strcpy(websiteName, ".html");
					char *const cmd[] = {"curl", "-v", buffer, ">", "temp.html", NULL};
					execvp(cmd[0], cmd);

					//Create package to send to client
					FILE *fptr;
					char filename[9];
					char *line;
					size_t length = 256;
					char fileChar;
					strcpy(filename, "temp.html");
					fptr = fopen(filename, "r");
					fileChar = fgetc(fptr);
					while (fileChar != EOF)
					{
						getline(&line, &length, fptr);
						strcat(html, line);
					}
					fclose(fptr);

					//Delete temp html file
					char *const cmd2[] = {"rm", "temp.html",  NULL};
					execvp(cmd2[0], cmd2);
				}

				//Serve the webpage to the client, or send a HTTP error message
				n = write(newsockfd, html, strlen(html));

				if(!checkerino)
				{
					break;
				}
			}
		}
	}

	return 0;
}

/*
======================================================================================================================================
Function : 
Parameters : 
Return : 
Description : 
======================================================================================================================================
*/

bool cacheCheck(char recvBuf[256])
{
	FILE *fptr;
	char filename[9];
	char *line;
	size_t length = 256;
	char fileChar;
	strcpy(filename, "list.txt");

	fptr = fopen(filename, "r");
    if (fptr == NULL)
    {
        printf("Cannot open file.\n");
        exit(0);
    }

	fileChar = fgetc(fptr);
    while (fileChar != EOF)
    {
		int checkerinos;
		//Counts number of characters read by getline
    	checkerinos = getline(&line, &length, fptr);
		//if 0, then there is nothing cached
		if(checkerinos == 0)
		{
			fclose(fptr);
			return true;
		}
		else if(strcmp(recvBuf, line) == 0)
		{
			fclose(fptr);
			return false;
		}

		fileChar = fgetc(fptr);
    }

	fclose(fptr);
	return true;
}

/*
======================================================================================================================================
Function : 
Parameters : 
Return : 
Description : 
======================================================================================================================================
*/

bool statusCodeConfirm()
{
	FILE *fptr;
	char *line;
	char filename[9];
	size_t length = 256;
	char fileChar;
	strcpy(filename, "header.txt");

	fptr = fopen(filename, "r");
    if (fptr == NULL)
    {
        printf("Cannot open file.\n");
        exit(0);
    }

	fileChar = fgetc(fptr);
    while (fileChar != EOF)
    {
    	getline(&line, &length, fptr);
		if(strcmp("HTTP/1.1 200 OK", line) == 0)
		{
			fclose(fptr);
			return true;
		}

		fclose(fptr);
		return false;

		fileChar = fgetc(fptr);
    }
}