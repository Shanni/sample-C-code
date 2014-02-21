/** @file server.c */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <queue.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "queue.h"
#include "libhttp.h"
#include "libdictionary.h"

const char *HTTP_404_CONTENT = "<html><head><title>404 Not Found</title></head><body><h1>404 Not Found</h1>The requested resource could not be found but may be available again in the future.<div style=\"color: #eeeeee; font-size: 8pt;\">Actually, it probably won't ever be available unless this is showing up because of a bug in your program. :(</div></html>";
const char *HTTP_501_CONTENT = "<html><head><title>501 Not Implemented</title></head><body><h1>501 Not Implemented</h1>The server either does not recognise the request method, or it lacks the ability to fulfill the request.</body></html>";

const char *HTTP_200_STRING = "OK";
const char *HTTP_404_STRING = "Not Found";
const char *HTTP_501_STRING = "Not Implemented";
pthread_t *threads = NULL;
int *client_fd = NULL;
struct addrinfo *result = NULL;
int count = 0;
int sock_fd;
queue_t q;
//http_t request;

/**
 * Processes the request line of the HTTP header.
 * 
 * @param request The request line of the HTTP header.  This should be
 *                the first line of an HTTP request header and must
 *                NOT include the HTTP line terminator ("\r\n").
 *
 * @return The filename of the requested document or NULL if the
 *         request is not supported by the server.  If a filename
 *         is returned, the string must be free'd by a call to free().
 */
char* process_http_header_request(const char *request)
{
	// Ensure our request type is correct...
	if (strncmp(request, "GET ", 4) != 0)
		return NULL;

	// Ensure the function was called properly...
	assert( strstr(request, "\r") == NULL );
	assert( strstr(request, "\n") == NULL );

	// Find the length, minus "GET "(4) and " HTTP/1.1"(9)...
	int len = strlen(request) - 4 - 9;

	// Copy the filename portion to our new string...
	char *filename = malloc(len + 1);
	strncpy(filename, request + 4, len);
	filename[len] = '\0';

	// Prevent a directory attack...
	//  (You don't want someone to go to http://server:1234/../server.c to view your source code.)
	if (strstr(filename, ".."))
	{
		free(filename);
		return NULL;
	}

	return filename;
}

void sig_handler(int signo)
{
	if (signo == SIGINT) {
		printf("Received SIGINT\n");
	
		int i;	
		for (i = 0; i < count; i++) {
			//shutdown(sock_fd, SHUT_RDWR);
			close(client_fd[i]);
		}

		for (i = 0; i < count; i++) {
			pthread_join(threads[i], NULL);
		}
	
		//shutdown(sock_fd, SHUT_RDWR);	
		close(sock_fd);
		freeaddrinfo(result);
		//http_free(&request);

		if (threads != NULL) free(threads);
		if (client_fd != NULL) free(client_fd);	

		exit(1);
	}


}

void * work(void *a)
{
	int socket = *((int *)a);	
	free(a);
	while (1) {

		//int socket = *((int *)a);
		http_t request;	
				

		//printf("Request received\n");
	
		/*Initial processing of HTTP request*/
		int n = http_read(&request, socket);

		if (n == -1) {
			printf("http_read failed\n");
 			break;
		}	

		//printf("Request received\n");

		char *file_name = process_http_header_request(http_get_status(&request));
	
		char *connection = (char *)(http_get_header(&request, "Connection"));

		if (connection == NULL || strcasecmp("Keep-Alive", connection) != 0) {
			connection = "close";
		} else if (strcasecmp("Keep-Alive", connection) == 0) {
			connection = "Keep-Alive";
		}



		int response_code = -1;	
		char *response_code_string = NULL;
		char *content = NULL;
		char *content_type = NULL;
		int content_length = -1;

		FILE *f = NULL;	

		/*501 Invalid request*/
		if (file_name == NULL) {
			response_code = 501;
			response_code_string = (char *)HTTP_501_STRING;
			content = (char *)HTTP_501_CONTENT;
			content_type = "text/html";
			content_length = strlen(content);

	
		} else if (file_name != NULL) {
			char *file_temp = file_name;
			
			char file2[1000];	
			
			if (strcmp(file_name, "/") == 0) {
				file_name = "/index.html";
			}
	
			sprintf(file2, "web%s", file_name);	
			
			//printf("file: %s\n", file2 );
			f = fopen(file2, "r");
		
			free(file_temp);

			/*404 File Not Found*/
			if (f == NULL) {
				response_code = 404;
				response_code_string = (char *)HTTP_404_STRING;
				content = (char *)HTTP_404_CONTENT;
				content_type = "text/html";
				content_length = strlen(content);


			/*200 Valid File*/
			} else if (f != NULL) {
				
				
				response_code = 200;	
				response_code_string = (char *)HTTP_200_STRING;
			
				int prev = ftell(f);
				fseek(f, 0L, SEEK_END);
				int size = ftell(f);
				fseek(f, prev, SEEK_SET);
				content_length = size;

				content = malloc(size);
				fread(content, 1, size, f);
				

				if (strstr(file2, ".html") != NULL) {
					content_type = "text/html";
				} else if ( strstr(file2, ".css") != NULL) {
					content_type = "text/css";
				} else if ( strstr(file2, ".jpg") != NULL) {
					content_type = "image/jpeg";
				} else if ( strstr(file2, ".png") != NULL) {	
					content_type = "image/png";
				} else {
					content_type = "text/plain";
				}

			}
		}

		
		

	

		char response[2000];			

		sprintf(response, "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %i\r\nConnection: %s\r\n\r\n", response_code, response_code_string, content_type, content_length, connection);

		send(socket, response, strlen(response), 0);
		send(socket, content, content_length, 0); 
	
		if (response_code == 200) {
			free(content);
			fclose(f);
		}
		http_free(&request);	

		if (strcmp("Keep-Alive", connection) != 0) {
			close(socket);	
 			break;		
		}

	}
	
	
	printf("Request sent\n");
	
	return NULL;

}

int main(int argc, char **argv)
{
	if (argc < 2) return 0;

	//queue_init(&q);

	int backlog = 10;	


	char *port_num = argv[1];

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	struct addrinfo hints;
	memset(&hints, 0x00, sizeof(struct addrinfo));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	int s = getaddrinfo(NULL, port_num, &hints, &result);

	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(1);
	}

	if ( bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0) {
		perror("bind()");
		exit(1);
	}

	if ( listen(sock_fd, backlog) != 0) {
		perror("listen()");
		exit(1);
	}

	
	int i = 0;
	
	while (1) {
		
		signal(SIGINT, sig_handler); 
	
		if (i == 0) {		
			client_fd = malloc( (sizeof(int))*(i+1));
			threads = malloc( (sizeof(pthread_t))*(i+1));
		} else if (i != 0) {
			client_fd = realloc(client_fd, (sizeof(int))*(i+1) );
			threads = realloc(threads, (sizeof(pthread_t))*(i+1) );
		}	

		client_fd[i] = accept(sock_fd, NULL, NULL);
		//printf("Got new request\n");i
		int* temp = malloc(sizeof(int));
		*temp = client_fd[i];
		printf("new thread\n");
		pthread_create(&threads[i], NULL, work, temp );
		count++;
		

		i++;
	}
	
	int j;

	for (j = 0; j < i; j++) {

		pthread_join(threads[j], NULL);

	}
	
	free(threads);
	free(client_fd);
	
	printf("You fucked up\n");
	return 0;
}
