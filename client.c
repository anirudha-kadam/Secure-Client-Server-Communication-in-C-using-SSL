#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<resolv.h>
#include<openssl/ssl.h>
#include<openssl/err.h>
#include<netdb.h>

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		printf("\nINVALID NUMBER OF ARGUMENTS\n");
		exit(0);
	}
	char *hostName, *portNumber;
	hostName = argv[1];
	portNumber = argv[2];
	int clientSocket, choice, inputNum, num, readSize;
	char inputMessage[20], serverResponse[20];
    	struct sockaddr_in serverAddr;
	struct hostent *host;
     
	SSL_library_init();
	const SSL_METHOD *method;
	SSL_CTX *ctx;
	OpenSSL_add_all_algorithms(); //load & register cryptos
	SSL_load_error_strings(); //load all error messages 
	method = SSLv23_client_method(); //create client instance 
	ctx = SSL_CTX_new(method); //create context 

    	//create socket
    	if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	{
        	perror("\nERROR: COULD NOT CREATE SOCKET");
    	}
     
	if((host = gethostbyname((const char *)hostName)) == NULL)
	{
		perror("ERROR IN HOSTNAME");
		exit(0);
	}
	
    	serverAddr.sin_family = AF_INET;
    	serverAddr.sin_port = htons(atoi(portNumber));
	serverAddr.sin_addr.s_addr = *(long*)(host->h_addr);

	//connect to server
    	if (connect(clientSocket , (struct sockaddr *)&serverAddr , sizeof(serverAddr)) < 0)
    	{
        	perror("\nERROR: COULD NOT CONNET TO SERVER");
        	exit(0);
    	}
     
    	puts("\nconnected to server");

	SSL *ssl;
	ssl = SSL_new(ctx); //create new SSL connection state
	SSL_set_fd(ssl, clientSocket); //attach the socket descriptor
	SSL_connect(ssl); //perform the connection
	
	//send a request server
	while(1)
	{
		printf("\n********Menu********\n");
		printf("\n1. Input Number\n2. Exit\n");
		printf("\nEnter choice: ");
		scanf("%d",&choice);
		switch(choice)
		{
			case 1:
			printf("\n[NOTE: Enter Number input as 0 to shut both the client and server down]\n");
			printf("\nEnter Number: ");
			scanf("%d",&inputNum);
			snprintf(inputMessage,20,"%d",inputNum);

			//send number
        		if( SSL_write(ssl , inputMessage , strlen(inputMessage)) < 0)
        		{
            			perror("send error");
            			return 1;	
        		}
			
			//shut down both client and server
			if(inputNum == 0)
			{
				SSL_free(ssl);
				exit(0);
			}

			//Receive a reply from the server
        		if((readSize = SSL_read(ssl , serverResponse , 20)) < 0)
        		{
            			perror("recv error");
            			break;
        		}
        		serverResponse[readSize] = 0;
			num = atoi(serverResponse);
			printf("\nResponse from server: %d \n",num);
			break;

			case 2:
				SSL_free(ssl);
				exit(0);
			break;

			default:
			printf("\nENTER CORRECT CHOICE!!\n");
			break;
		}
	}
}
