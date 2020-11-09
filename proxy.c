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
#include <netdb.h>
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

int main(int argc, char *argv[])
{
    //Client structure
    int sockfd, newsockfd, portno, clilen, n;
   	struct sockaddr_in serv_addr, cli_addr;
    
    // //Proxy server acting as client to origin server structure
    // int websockfd, portno2 = 80;
    // struct sockaddr_in serv_addr2;
    // struct hostent *host_entry;

    bool checkerino = true;
	bool cached = false;

    char htmlFilename[256];


	if(argc < 2)
	{
		printf("\nPort number is missing...\n");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
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

    // websockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if(websockfd < 0)
    // {
    //     error(EXIT_FAILURE, 0, "ERROR opening web socket");
    // }

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

        //while client is connected
        while(checkerino)
		{
            //Proxy server acting as client to origin server structure
            int websockfd, portno2 = 80;
            struct sockaddr_in serv_addr2;
            struct hostent *host_entry;
            char websiteName[256];
            char html[1000000];
            char url[256];
            char buffer[256];
            char line[1000000];

            websockfd = socket(AF_INET, SOCK_STREAM, 0);
            if(websockfd < 0)
            {
                error(EXIT_FAILURE, 0, "ERROR opening web socket");
            }

			//Receiving the message from the client
			bzero(buffer, 256);
			n = read(newsockfd, buffer, 255);
			if(n < 0)
			{
				error(EXIT_FAILURE, 0, "ERROR reading from socket");
			}
			else
			{
				printf("\nClient has requested: %s\n", buffer);
				strcpy(websiteName, buffer);
            }

            //websiteName[strlen(websiteName) - 1] = '/';
            //200iq: THIS IS HOW YOU GET RID OF THE LAST CHARACTER (newline character in this case) IN A STRING IN C.
            strncpy(url, websiteName, (strlen(websiteName) - 1));

            //printf("1->%s<-", url);

            host_entry = gethostbyname(url);
            if(host_entry == NULL)
            {
                printf("\nERROR, no such host... closing program\n");
                exit(0);
            }

            //Connecting with the origin server
            bzero((char *) &serv_addr2, sizeof(serv_addr2));
            serv_addr2.sin_family = AF_INET;
            serv_addr2.sin_port = htons(portno2);
            memcpy(&serv_addr2.sin_addr, host_entry->h_addr, host_entry->h_length);
            int errNo = connect(websockfd, (struct sockaddr*)&serv_addr2,sizeof(serv_addr2));
            if(errNo < 0)
            {
                printf("\nError Number: %d \nERROR connecting the web server...\n", errNo);
                //error(EXIT_FAILURE, 0, "ERROR connecting the web server...");
            }

            //printf("2->%s<-", url);

            //Create get request
            snprintf(line, 1000000, "GET / HTTP/1.0\r\nHost:%s\r\n\r\n", url);

            //write
            if (write(websockfd, line, strlen(line)) >= 0) 
            {
                bzero(html, 1000000);
                // Read the response
                while ((n = read(websockfd, html, 1000000)) > 0) 
                {
                    html[n] = '\0';

                    //search through html for 200 OK
                    //if 200 OK: write to custom .html, add to list.txt
                    if(strstr(html, "HTTP/1.0 200 OK") != NULL)
                    {
                        //write to html file
                        FILE *fptr;
                        char listLine[256];
                        size_t length = 256;

                        //check list.txt
                        //if list.txt contains URL, do nothing
                        fptr = fopen("list.txt", "r");
                        if (fptr == NULL)
                        {
                            printf("List file does not exist... creating...\n");
                            fptr = fopen("list.txt", "w");
                            if (fptr == NULL)
                            {
                                printf("Failed to create file, exiting...\n");
                                exit(0);
                            }

                            fclose(fptr);
                        }
                        else
                        {
                            fclose(fptr);
                        }

                        printf("3->%s<-", url);

                        fptr = fopen("list.txt", "r");
                        while(fgets(listLine, 256, fptr))
                        {
                            if(strcmp(url, listLine) == 0)
                            {
                                cached = true;
                                break;
                            }
                        }

                        fclose(fptr);

                        //printf("3.5->%s<-", url);

                        if(!cached)
                        {
                            //webpage not cached, so cache it
                            strcat(htmlFilename, url);
                            strcat(htmlFilename, ".html");

                            fptr = fopen(htmlFilename, "w");
                            if (fptr == NULL)
                            {
                                printf("Cannot open file 1.\n");
                                exit(0);
                            }

                            fputs(html, fptr);
                            //printf("Website has been cached.\n");

                            fclose(fptr);

                            //save URL to list.txt, because it was not cached
                            fptr = fopen("list.txt", "a");
                            if (fptr == NULL)
                            {
                                printf("Cannot open file 2.\n");
                                exit(0);
                            }

                            fputs(url, fptr);
                            fputs("\n", fptr);
                            //printf("Website has been listed.\n");

                            fclose(fptr);
                        }
                        else
                        {
                            char htmlPage[1000000];
                            strcat(htmlFilename, url);
                            strcat(htmlFilename, ".html");

                            fptr = fopen(htmlFilename, "r");
                            if (fptr == NULL)
                            {
                                printf("Cannot open file 3.\n");
                                exit(0);
                            }

                            while(fgets(htmlPage, 1000000, fptr))
                            {
                                strcat(html, htmlPage);
                            }

                            fclose(fptr);
                            printf("4->%s<-", url);
                        }

                        //printf("5->%s<-", url);
                    }

                    //printf("6->%s<-", url);
                    
                    //forward to client
                    write(newsockfd, html, strlen(html));
                    bzero(url, 256);                    
                    bzero(htmlFilename, 256);               
                }          
            }

            close(websockfd);
        }
    }

    close(sockfd);
    return 0;
}
