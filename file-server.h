/* This page contains a client program that can request a file from the server program
 * on the next page. The server responds by sending the whole file.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>

#define SERVER_PORT 2345		/* arbitrary, but client & server must agree */
#define BUF_SIZE 4096		/* block transfer size */
#define DEBUG 1 // DEBUG variable 



void fatal(char *string)
{
  printf("%s\n", string);
  exit(1);
}