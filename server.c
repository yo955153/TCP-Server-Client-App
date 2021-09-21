// Yoseph Hassan, EEL4781 Project 1 client file

/* This is the server code */
#include "file-server.h"
#include <sys/fcntl.h>

#define QUEUE_SIZE 10

// Helper function which prints the percent of the file sent
void print_percentage(int bytes, char *filename, int counter)
{
  static int percent_i = 1;
  double percent;

  while(percent_i <= 10)
  {
    percent = (double) (percent_i) / 10;

    if((percent * bytes) <= counter)
    {
      if(DEBUG)
        printf("Sent %d%% of %s\n", percent_i * 10, filename);
    }
    
    else
    {
      break;
    }
    
    if(percent_i == 10)
    {
      percent_i = 1;
      if(DEBUG)
        printf("\n\n");
      break;
    }

    percent_i++;
  }
}

// Helper function which checks if the byte ranges specified are valid
int byte_range_checker(off_t input_size, int *start_byte, int *end_byte, int accept_sock)
{
  int error_num = 0;

  if(*start_byte > (input_size - 1))
  {
    printf("Cannot write to client: start byte out of range\nThe byte range is from 1 to %ld\n", input_size);
    error_num = 2;
    write(accept_sock, &error_num, sizeof(int));
    write(accept_sock, &input_size, sizeof(off_t));
    return error_num;  
  }

  if(*end_byte == -1)
  {
    *end_byte = (input_size - 1);
  }


  if(*end_byte > (input_size - 1))
  {
    printf("Cannot write to client: end byte out of range\nThe byte range is from 1 to %ld\n", input_size);
    error_num = 3;
    write(accept_sock, &error_num, sizeof(int));
    write(accept_sock, &input_size, sizeof(off_t));
    return error_num;  
  }


  if(*start_byte > *end_byte)
  {
    printf("Start byte cannot be greater than end byte\n");
    error_num = 5; // start byte > end byte error number
    write(accept_sock, &error_num, sizeof(int));
    return error_num;
  }

  return 6;
}

int main(int argc, char *argv[])
{	
  int s, b, l, fd, sa, bytes, on = 1, error_num = 0;
  off_t file_size;
  int write_flag, start_byte, end_byte, amount_bytes, amount_bytes_cpy, counter = 0, file_descriptor;
  char buf[BUF_SIZE], filename[BUF_SIZE], string_ip[INET_ADDRSTRLEN];		/* buffer for outgoing file */

  struct addrinfo channel;
  struct addrinfo *client_channel;
  struct sockaddr client_addr;
  socklen_t c_size = sizeof(client_addr);

  struct stat file_info;

  memset(&channel, 0, sizeof(channel));
  channel.ai_family = AF_INET;
  channel.ai_socktype - SOCK_STREAM;

  channel.ai_flags = AI_PASSIVE;
  getaddrinfo(NULL, "2345", &channel, &client_channel);

  
  

  /* Passive open. Wait for connection. */
  s = socket(client_channel->ai_family, client_channel->ai_socktype, client_channel->ai_protocol);    /* create socket */

  if (s < 0) 
    fatal("\nsocket failed\n");
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));

  b = bind(s, client_channel->ai_addr, client_channel->ai_addrlen);

  if (b < 0) 
    fatal("bind failed\n");

  l = listen(s, QUEUE_SIZE);		/* specify queue size */

  if (l < 0) 
    fatal("listen failed\n");

  /* Socket is now set up and bound. Wait for connection and process it. */
  while (1) 
  {
        sa = accept(s, &client_addr, &c_size);
        struct sockaddr_in *ip;
        ip = (struct sockaddr_in *) &client_addr;

        inet_ntop(client_addr.sa_family, &ip, string_ip, sizeof(string_ip));

        read(sa, &start_byte, sizeof(start_byte));
        read(sa, &end_byte, sizeof(end_byte));
        read(sa, &write_flag, sizeof(write_flag));

        bytes = read(sa, filename, BUF_SIZE);

        if(write_flag == 1)
        {
          file_descriptor = open(filename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
          

          if(file_descriptor <0)
          {
            printf("\nCould not open file named %s\n", filename);
            
            error_num = 1;
            write(sa, &error_num, sizeof(int));
          }

          error_num = 6;
          write(sa, &error_num, sizeof(int));

          amount_bytes = 1 + end_byte - start_byte;
          amount_bytes_cpy = 1+ end_byte -start_byte;

          while(amount_bytes_cpy != 0)
          {
            if(amount_bytes_cpy > BUF_SIZE)
              bytes = read(sa, buf, BUF_SIZE);
            
            else
              bytes = read(sa, buf, amount_bytes_cpy);
            
            write(file_descriptor, buf, bytes);

            amount_bytes_cpy -= bytes;
          }

        }

        else
        {
          file_descriptor = open(filename, O_RDONLY);


          if(file_descriptor < 0)
          {
            printf("\nCould not open file named %s\n", filename);
            error_num = 1;
            write(sa, &error_num, sizeof(int));
            exit(0);
          }

          fstat(file_descriptor, &file_info);

          file_size = file_info.st_size;

          error_num = byte_range_checker(file_size, &start_byte, &end_byte, sa);

          if(error_num < 6)
            continue;
          
          error_num = 7; // This means it is fine
          write(sa, &error_num, sizeof(error_num));
          write(sa, &file_size, sizeof(file_size));

          lseek(file_descriptor, start_byte, SEEK_SET);

          amount_bytes = 1 + end_byte - start_byte;
          amount_bytes_cpy = 1 + end_byte -start_byte;


          if(DEBUG)
            printf("Sending %s to %s\n", filename, string_ip);
          while(amount_bytes_cpy != 0)
          {
            if(amount_bytes_cpy > BUF_SIZE)
              bytes = read(file_descriptor, buf, BUF_SIZE);
            
            else
              bytes = read(file_descriptor, buf, amount_bytes_cpy);
            
            write(sa, buf, bytes);
            counter += bytes;
            print_percentage(amount_bytes, filename, counter);
            amount_bytes_cpy -= bytes;
           
              if(DEBUG && !amount_bytes_cpy)
                printf("Finished sending %s to %s\n", filename, string_ip);
          }
        }

        // close to avoid memory leak
        close(file_descriptor);
        close(sa);

  return 0;
  }
}
