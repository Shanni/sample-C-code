#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>

#include "ds11_aux.h"

/* In this DS, we will build a simple client which performs a
   connection to a server. In particular, the server implements a
   service of reverse echo. Whatever is being sent by the client, gets
   repeated by the server in reverse order.

   Client Message type BIN definition:
     sizeof(int) bytes    str_len bytes
   [    str_len       ][      string      ]

   Cleint Message type ASCII definition:
   string\n

   After the connection is establisehd (ASCII communication):
   Server: "Welcome to the CS241 Reverse Echo Service\n"
   Server: "Please, choose your message format\n"
   Client: "ASCII\n"
   Client: "Some string\n"
   Server: "gnirts emoS\n"

   After the connection is establisehd:
   Server: "Welcome to the CS241 Reverse Echo Service\n"
   Server: "Please, choose your message format\n"
   Client: "BIN\n"
   Client: 11"Some string"
   Server: 11"gnirts emoS"

  */

#define WELCOME "Welcome to the CS241 Reverse Echo Service\n"
#define PROT_CHOOSE "Please, choose your message format\n"

#define PROT_ASCII 1
#define PROT_BIN 2

/**
 * @brief Perform a read operation on a socket until a newline
 * character ('\n') is detected. The message is stored into
 * buffer. The memory required to store the data is allocated if
 * needed. The variable bufsize is updated accordingly. NOTE: this
 * function never reads from the socket more data than the targeted
 * message, so it is safe to call it twice to retrieve two distinct
 * messages.
 *
 * @param fd socket on which the operation is being performed
 * 
 * @param buffer pointer to a buffer that will store the data. If a
 * realloc operation is performed, this variable is updated with the
 * new position of the buffer
 *
 * @param bufsize pointer to the size of the output buffer. If a
 * realloc operation is performed, this variable is updated with the
 * new size of the buffer
 *
 * @return the length of the retrieved message, excluding the trailing
 * newline character
 */
int read_to_nl(int fd, char ** buffer, ssize_t * bufsize) {
	int bread = 0;
	int size = (*bufsize > 1)?(*bufsize-1):(INIT_BUF);
	char * end = NULL;
	if(size +1 > *bufsize) 
		*buffer = realloc(*buffer, size + 1);
	do {
		/* Implementation goes HERE ! */
		int bytes = 0; /* Dummy definition */
		/* Implementation concludes HERE */
		bread += bytes;
	} while(!end);


out:
	*bufsize = size;
	return (end - *buffer);
}

int main(int argc, char ** args)
{

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	struct addrinfo * info = NULL;
	struct sockaddr * server = NULL;
	int retval = 0;
	
	char * buf = NULL;
	ssize_t buf_size = 0;
	int mesg_len;

	int choice;

	if(argc < 2) {
		fprintf(stderr, "Usage: %s <port>\n");
		return 1;
	}	
	
	/* START OF TODO BLOCK */
	
	/* Find the service with getaddrinfo */
	
	/* Perform a connect to the discovered service */

	/* Receive welcome message from server */

	/* Receive protocol prompt from server */
	
	/* END OF TODO BLOCK */

	/* Let the user choose */
	printf("1) ASCII Protocol\n2) BIN Protocol\nYour choice: ");
	scanf("%d", &choice);
	/* This is just to remove the trailing newline from the stdin */
	getline(&buf, &buf_size, stdin);

	switch (choice) {
	case PROT_ASCII :
		mesg_len = write_all(sock, "ASCII\n", 6);
		break;
	case PROT_BIN :
	default:
		mesg_len = write_all(sock, "BIN\n", 4);
	}
	
	if(mesg_len < 0) {
		retval = 1;
		goto out;
	}
	
	/* Final communication */
	while(1) {

		printf("Input string to reverse:\n");
		mesg_len = getline(&buf, &buf_size, stdin);
		--mesg_len;
		
		switch (choice) {
		case PROT_ASCII :
			/* START OF TODO BLOCK */
			/* Send the request */
			/* Read the response */
			/* Print it! */
			printf("%s\n\n", buf);
			/* END OF TODO BLOCK */
			break;
		case PROT_BIN :
		default:
			/* START OF TODO BLOCK */
			/* Send the message length */
			/* Send the message */
			/* Read the response length */
			/* Read the response */
			/* Print it! */
			printf("%.*s\n\n", mesg_len, buf);
			/* END OF TODO BLOCK */
		}
		
	}
	
	
out:
	close(sock);
	if(buf) free(buf);
	freeaddrinfo(info);	
	return retval;
}
