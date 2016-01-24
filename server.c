#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<resolv.h>
#include<openssl/ssl.h>
#include<openssl/err.h>

int main(int argc, char *argv[])
{ 
	if(argc != 4)
	{
		printf("\nINVALID NUMBER OF ARGUMENTS\n");
		exit(0);
	}
	char *hostName, *portNumber, *CertFile, *KeyFile;
	portNumber = argv[3];
	CertFile = argv[1];
	KeyFile = argv[2];
	int serverSocket, clientSocket, len, readSize;
	struct sockaddr_in serverAddr, clientAddr;
  	char clientMessage[20], serverResponse[20];

	SSL_library_init();
	const SSL_METHOD *method;
	SSL_CTX *ctx;
	OpenSSL_add_all_algorithms(); //load & register cryptos
	SSL_load_error_strings(); //load all error messages 
	method = SSLv23_server_method(); //create server instance 
	ctx = SSL_CTX_new(method); //create context 


	//set the local certificate from CertFile
	SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM);

	//set the private key from KeyFile
	SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM);

	//verify private key
	if( !SSL_CTX_check_private_key(ctx) )
		abort();

	//Create socket
    	serverSocket = socket(PF_INET , SOCK_STREAM , 0);
    	if (serverSocket == -1)
    	{
        	perror("COULD NOT CREATE SOCKET");
    	}
     	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
    	serverAddr.sin_addr.s_addr = INADDR_ANY;
    	serverAddr.sin_port = htons(atoi(portNumber));
	
	//bind socket
    	if(bind(serverSocket,(struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    	{
        	perror("BIND ERROR");
        	exit(0);
    	}

	//listen
    	listen(serverSocket , 10);

    	len = sizeof(struct sockaddr_in);
     
	

    	//serve client
	while(1)
	{
		puts("\n\nWaiting for connection...");
		
		//accept client connection
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, (socklen_t*)&len);
		if (clientSocket < 0)
    		{
        		puts("CONNECTION ACCEPT ERROR");
        		exit(0);
    		}
		puts("\nClient connected");
		
		//get new SSL state with context
		SSL *ssl = SSL_new(ctx); 

		//set connection to SSL state
		SSL_set_fd(ssl, clientSocket); 
		
		//start the handshaking
		SSL_accept(ssl); 			


		//receive a message from client
    		while( (readSize = SSL_read(ssl, clientMessage, sizeof(clientMessage))) > 0 )
    		{
			clientMessage[readSize] = 0;
			printf("\nclient entered: %s\n",clientMessage);
			//process client's request
			int num = atoi(clientMessage);
			if(num == 0)
			{
				puts("shutting down the server...");
				//close connection & clean up
				clientSocket = SSL_get_fd(ssl); //get the raw connection
				SSL_free(ssl); //release SSL state
				close(serverSocket); //close connection
				exit(0);
			}
			num = num - 1;
			snprintf(serverResponse, 20,"%d",num);

        		//respond to client
			SSL_write(ssl, serverResponse, strlen(serverResponse));
			printf("server responded: %s\n",serverResponse);
    		}
     
    		if(readSize == 0)
    		{
        		printf("\nClient disconnected");
        		fflush(stdout);
    		}
    		else if(readSize == -1)
    		{
        		perror("recv error");
    		}
    		
	}

}
