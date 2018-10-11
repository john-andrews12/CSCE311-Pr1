/************************************************************/
/* This is a stream socket client sample program for UNIX   */
/* domain sockets. This program creates a socket, connects  */
/* to a server, sends data, then receives and prints a      */
/* message from the server.                                 */
/************************************************************/

#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"
#define DATA "Hello from client"

using namespace std;

int main(void){

    int client_sock, rc, len;
    struct sockaddr_un server_sockaddr; 
    struct sockaddr_un client_sockaddr; 
    char buf[256];
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
     
    /**************************************/
    /* Create a UNIX domain stream socket */
    /**************************************/
    client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_sock == -1) {
        //printf("SOCKET ERROR = %d\n", sock_errno());
        cout << "SOCKET ERROR" << SO_ERROR << endl;
        exit(1);
    }

    /***************************************/
    /* Set up the UNIX sockaddr structure  */
    /* by using AF_UNIX for the family and */
    /* giving it a filepath to bind to.    */
    /*                                     */
    /* Unlink the file so the bind will    */
    /* succeed, then bind to that file.    */
    /***************************************/
    client_sockaddr.sun_family = AF_UNIX;   
    strcpy(client_sockaddr.sun_path, CLIENT_PATH); 
    len = sizeof(client_sockaddr);
    
    unlink(CLIENT_PATH);
    //unlink(SERVER_PATH);
    rc = ::bind(client_sock, (struct sockaddr *) &client_sockaddr, len);
    if (rc == -1){
        //printf("BIND ERROR: %d\n", sock_errno());
        cout << "BIND ERROR"<< SO_ERROR << endl;
        close(client_sock);
        exit(1);
    }
        
    /***************************************/
    /* Set up the UNIX sockaddr structure  */
    /* for the server socket and connect   */
    /* to it.                              */
    /***************************************/
    server_sockaddr.sun_family = AF_UNIX;
    strcpy(server_sockaddr.sun_path, SERVER_PATH);
    rc = connect(client_sock, (struct sockaddr *) &server_sockaddr, len);
    if(rc == -1){
        //printf("CONNECT ERROR = %d\n", sock_errno());
        cout << "CONNECT ERROR" << SO_ERROR << endl;
        close(client_sock);
        exit(1);
    }
    
    /************************************/
    /* Copy the data to the buffer and  */
    /* send it to the server socket.    */
    /************************************/
    strcpy(buf, DATA);                 
    printf("Sending data...\n");
    rc = send(client_sock, buf, strlen(buf), 0);
    if (rc == -1) {
        //printf("SEND ERROR = %d\n", sock_errno());
        cout << "SEND ERROR" << endl;
        close(client_sock);
        exit(1);
    }   
    else {
        printf("Data sent!\n");
    }

    /**************************************/
    /* Read the data sent from the server */
    /* and print it.                      */
    /**************************************/
    printf("Waiting to recieve data...\n");
    memset(buf, 0, sizeof(buf));
    rc = recv(client_sock, buf, sizeof(buf), 0);
    if (rc == -1) {
        //printf("RECV ERROR = %d\n", sock_errno());
        cout << "RECV ERROR" << endl;
        close(client_sock);
        exit(1);
    }   
    else {
        printf("DATA RECEIVED = %s\n", buf);
    }
    
    /******************************/
    /* Close the socket and exit. */
    /******************************/
    close(client_sock);
    
    return 0;
}






/************************************************************/
/* This is a stream socket server sample program for UNIX   */
/* domain sockets. This program listens for a connection    */
/* from a client program, accepts it, reads data from the   */
/* client, then sends data back to connected UNIX socket.   */
/************************************************************/
/*
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCK_PATH  "tpf_unix_sock.server"
#define DATA "Hello from server"

using namespace std;

int main(void){

    int server_sock, client_sock, len, rc;
    int bytes_rec = 0;
    struct sockaddr_un server_sockaddr;
    struct sockaddr_un client_sockaddr;     
    char buf[256];
    int backlog = 10;
    memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
    memset(buf, 0, 256);                
    
    /**************************************/
    /* Create a UNIX domain stream socket */
    /**************************************/
/*    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1){
        //printf("SOCKET ERROR: %d\n", sock_errno());
        cout << "SOCKET ERROR" << endl;
        exit(1);
    }
    
    /***************************************/
    /* Set up the UNIX sockaddr structure  */
    /* by using AF_UNIX for the family and */
    /* giving it a filepath to bind to.    */
    /*                                     */
    /* Unlink the file so the bind will    */
    /* succeed, then bind to that file.    */
    /***************************************/
/*    server_sockaddr.sun_family = AF_UNIX;   
    strcpy(server_sockaddr.sun_path, SOCK_PATH); 
    len = sizeof(server_sockaddr);
    
    unlink(SOCK_PATH);
    rc = ::bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
    if (rc == -1){
        //printf("BIND ERROR: %d\n", sock_errno());
        cout << "BIND ERROR" << endl;
        close(server_sock);
        exit(1);
    }
    
    /*********************************/
    /* Listen for any client sockets */
    /*********************************/
/*    rc = listen(server_sock, backlog);
    if (rc == -1){ 
        //printf("LISTEN ERROR: %d\n", sock_errno());
        cout << "LISTEN ERROR" << endl;
        close(server_sock);
        exit(1);
    }
    printf("socket listening...\n");
    
    /*********************************/
    /* Accept an incoming connection */
    /*********************************/
/*    client_sock = accept(server_sock, (struct sockaddr *) &client_sockaddr, (socklen_t *) &len);
    if (client_sock == -1){
        //printf("ACCEPT ERROR: %d\n", sock_errno());
        cout << "ACCEPT ERROR" << endl;
        close(server_sock);
        close(client_sock);
        exit(1);
    }
    
    /****************************************/
    /* Get the name of the connected socket */
    /****************************************/
/*    len = sizeof(client_sockaddr);
    rc = getpeername(client_sock, (struct sockaddr *) &client_sockaddr, (socklen_t *) &len);
    if (rc == -1){
        //printf("GETPEERNAME ERROR: %d\n", sock_errno());
        cout << "GETPEERNAME ERROR" << endl;
        close(server_sock);
        close(client_sock);
        exit(1);
    }
    else {
        printf("Client socket filepath: %s\n", client_sockaddr.sun_path);
    }
    
    /************************************/
    /* Read and print the data          */
    /* incoming on the connected socket */
    /************************************/
 /*   printf("waiting to read...\n");
    bytes_rec = recv(client_sock, buf, sizeof(buf), 0);
    if (bytes_rec == -1){
        //printf("RECV ERROR: %d\n", sock_errno());
        cout << "RECV ERROR" << endl;
        close(server_sock);
        close(client_sock);
        exit(1);
    }
    else {
        printf("DATA RECEIVED = %s\n", buf);
    }
    
    /******************************************/
    /* Send data back to the connected socket */
    /******************************************/
/*    memset(buf, 0, 256);
    strcpy(buf, DATA);      
    printf("Sending data...\n");
    rc = send(client_sock, buf, strlen(buf), 0);
    if (rc == -1) {
        //printf("SEND ERROR: %d", sock_errno());
        cout << "SEND ERROR" << endl;
        close(server_sock);
        close(client_sock);
        exit(1);
    }   
    else {
        printf("Data sent!\n");
    }
    
    /******************************/
    /* Close the sockets and exit */
    /******************************/
/*    close(server_sock);
    close(client_sock);
    
    return 0;
}



//#include "using_socket.h"

/*int main(void) {
	cout << "ENTERING" << endl;
	
	int server_sock, client_sock, len, rc;
	int bytes_rec = 0;
	struct sockaddr_un server_sockaddr;
	struct sockaddr_un client_sockaddr;
	char buf[256];
	int backlog = 10;
	memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
	memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
	memset(buf, 0, 256);
	
	//create the domain stream socket
	server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (server_sock == -1) {
		std::cout << "Socket Error" << std::endl;
		exit(1);
	}
	
	//initialize the sockaddr structure
	//use AF_UNIX for the family and 
	//bind things to a filepath 
	server_sockaddr.sun_family = AF_UNIX;
	strcpy(server_sockaddr.sun_path, SOCK_PATH);
	len = sizeof(server_sockaddr);
	
	unlink(SOCK_PATH);
	rc = ::bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
	if (rc == -1) {
		cout << "ERROR: Bind Error" << endl;
		close(server_sock);
		exit(1);
	}
	
	//now listen for any client sockets
	rc = listen(server_sock, backlog);
	if (rc == -1) {
		cout << "ERROR: Listen Error" << endl;
		close(server_sock);
		exit(1);
	}
	cout << "Socket listening..." << endl;
	
	//accept anything coming in
	client_sock = accept(server_sock, (struct sockaddr *) &client_sockaddr, (socklen_t *) &len);
	if (client_sock == -1) {
		cout << "ERROR: Accept Error" << endl;
		close(server_sock);
		close(client_sock);
		exit(1);
	}
	//cout << "Accepted" << endl;
	
	//get name of connected sock
	len = sizeof(client_sock);
	rc = getpeername(client_sock, (struct sockaddr *) &client_sockaddr, (socklen_t *) &len);
	if (rc == -1) {
		cout << "ERROR: getpeername Error" << endl;
		close(server_sock);
		close(client_sock);
		exit(1);
	}
	cout << "Client socket filepath: " << client_sockaddr.sun_path << endl;
	
	
	
	cout << "SUCCESSFUL EXIT" << endl;
}*/




//ORIGINAL EFFORT
/*#include "using_socket.h"

char const * socket_path = "\0";

int main(int argc, char *argv[]) {
	cout << "Entering" << endl;
	
	struct sockaddr_un addr;
	char buf[100];
	int fd,rc;
	
	if (argc > 1) {
		socket_path = argv[1];
	}
	
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		cout << "Socket Error" << endl;
		exit(-1);
	}
	
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	
	if (*socket_path == '\0') {
		*addr.sun_path = '\0';
		strncpy(addr.sun_path+1, socket_path+1, sizeof(addr.sun_path)-2);
	}
	else {
		strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);
	}
	
	if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		cout << "Connect Error" << endl;
		exit(-1);
	}
	
	while ((rc=read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
		cout << "hello" << endl;
		if (write(fd, buf, rc) != rc) {
			if (rc > 0) {
				cout << "Error: Partial Write";
			}
			else {
				cout << "Error: Write Error";
			}
			exit(-1);
		}
	}
	
	cout << "Exiting" << endl;
}*/