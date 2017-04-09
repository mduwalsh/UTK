/******
Drew's code for the filesystem
EVERYTHING SHOULD BE COPIED TO THE 'filesys.c' FILE!
******/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* This function starts the server to allow for Remote Procedure Calls, as defined in the
   assignment description */
void start_server(){
    int portnum;	/* Port number */
    int sock, fd; 	/* Ints to refer to sock and file descriptor, once initialized */
    int n;		/* Temporary variable */
    socklen_t len;	/* Client address length, as defined in the library */
    struct sockaddr_in server, client; /* Server and client addresses, using the library */

    printf("Starting server mode...\n");
    printf("Enter port number. Ports 5555 and 5500 are recommended.\n");
    
    scanf("%d", &portnum);
	printf("Review the manual for information about how to connect from client.\n");
    printf("To exit server mode, end this process and type 'End Server' in Client shell.\n");
    printf("Now redirecting input/output to Client.\n");
    
    /* Initialize sock, using method definted in Linux Howtos Guide */
    sock = socket(AF_INET, SOCK_STREAM, 0);
	printf("sock returned = %d\n", sock);
    if (sock < 0){
        perror("Error Starting Server");
        exit(1);
    }
    
    /* Clear out server address */
    bzero((char *) &server, sizeof(server));
    
    /* Set proper server struct attributes, according to methods outlined in Linux Howtos Guide */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portnum);

    /* Bind server to port and address, according to Linux Howtos Guide method */
    if(bind(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Error Binding Server");
        exit(1);
    }

    /* Start listening to the socket to wait for a connection */
    listen(sock, 5); /* Set to 5 since it is max size permitted */
    len = sizeof(client);

    /* Accept calling client and set file descriptor as fd */
    fd = accept(sock, (struct sockaddr *) &client, &len);
    if (fd < 0){
        perror("Error Accepting Client");
        exit(1);
    }
    setbuf(stdout, NULL); /* Keeps from having to call fflush() every time printf() is called */
    dup2(fd, 0); /* Read from Client, instead of Standard Input */
    dup2(fd, 1); /* Write to Client, instead of Standard Output */
    dup2(fd, 2); /* Write to Client, instead of Standard Error */	
    
    printf("Connected to Server. Type 'End Server' to exit.\n");    
return;
}

void end_server(){
    exit(1);
}


void execute(char *path){
    int id; 
    char *dest;
    char *filename;
    char *args = NULL;

/*
     Get filename
    filename = get_filename;
*/

    //export_file(path, dest); 

    if (fork() == 0){
        id = execv(filename, args);
        perror("Executing Process");
        exit(1);
    } else {
        wait(&id);
    }
return;
}
/*
void link(){
return 0;
}

void unlink(){
return 0;
}

void stat(){
return 0;
}

void cat(){
return 0;
}

void cp(){
return 0;
}

void tree(){
return 0;
}

void import(){
return 0;
}

void export(){
return 0;
}
*/

int main(){
    start_server();
    return 0;
}
