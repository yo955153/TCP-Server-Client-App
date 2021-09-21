// Yoseph Hassan, EEL4781 Project 1 client file

/* This page contains a client program that can request a file from the server program
 * on the next page. The server responds by sending the whole file.
 */

#include "file-server.h"


// Helper function to check if input is a number
int numcheck(char *data)
{
  int length = strlen(data), i;

  if(!data)
    return 0;

  for(i = 0; i < length; i++)
  {
    if(isdigit(data[i]) == 0)
    {
      return 0;
    }
  }

  return 1;
}

// Helper function which prints the format for commands 
void usg_details()
{
  printf("\n\n\n**********Command Format**********\n");
  printf("Default Specification: client server-name file-name");
  printf("\n\nAdditional Commands:\n");
  printf("-s, Specify the start bit of the file being sent (Start bit is 1 and end bit is inclusive)\n");
  printf("-e, Specify the end but of the file being sent\n");
  printf("Usage: client server-name -s START_BYTE -e END_BYTE file-name\n\n");

  printf("Uploading to Server:\n");
  printf("-w, Write file to server (Can't be used in conjunction with -s & -e arguments)\n");
  printf("Usage: client server-name -w file-name\n\n\n");

}


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
void byte_range_checker(off_t input_size, int *start_byte, int *end_byte)
{

  if(*start_byte > (input_size - 1))
  {
    printf("Cannot write to server: start byte out of range\nThe byte range is from 1 to %ld\n", input_size);
    exit(1);
  }

  if(*end_byte == -1)
  {
    *end_byte = (input_size - 1);
  }

  if(*start_byte > *end_byte)
  {
    printf("Start byte cannot be greater than end byte\n");
    exit(1);
  }

  if(*end_byte > (input_size - 1))
  {
    printf("Cannot write to server: end byte out of range\nThe byte range range from 1 to %ld\n", input_size);
    exit(1);
  }

}



int main(int argc, char **argv)
{
  int c, s, bytes, command, byte_range_flag = 0, start_byte = 0, end_byte = -1, write_flag = 0, amount_bytes, amount_bytes_cpy, file_descriptor;
  int counter = 0, error_num = 0;
  off_t file_size;
  char buf[BUF_SIZE];		/* buffer for incoming file */

  struct addrinfo channel; // holds IP address
  struct addrinfo *server_channel; // addrinfo pointer5
  struct stat file_info; // stat data type tells info about files being transferred

  // Checks if arguments passed are in valid format
  if (argc < 3) 
    fatal("Usage: client server-name [-w Write to Server] file-name [-s START_BYTE] [-e END_BYTE]");



  // loop for parsing through arguments passed
  for(;(command = getopt(argc, argv, "s:e:w")) != -1;)
  {
    switch(command)
    {
      case 'w':
        write_flag =1;

        if(byte_range_flag)
        {
          printf("\nByte Range Selection cannot be used in conjunction with write command.\n");
          usg_details();
          exit(1);
        }
        break;
      
      case 's':
        if(write_flag)
        {
          printf("\nWrite command cannot be used in conjunction with Byte Range Selection.\n");
          usg_details();
          exit(1);
        }
        else
        {
          if(numcheck(optarg) == 0 || atoi(optarg) == 0)
          {
            printf("\nNon positive integer input detected for start byte.\n");
            usg_details();
            exit(1);
          }
          else
          {
            start_byte = atoi(optarg) - 1;
            byte_range_flag = 1;
          }
          
        }
        break;
      
      case 'e':
        if(write_flag)
        {
          printf("\nWrite command cannot be used in conjunction with Byte Range Selection.\n");
          usg_details();
          exit(1);
        }

        else
        {
          if(numcheck(optarg) == 0 || atoi(optarg) == 0)
          {
            printf("\nNon positive integer input detected for end byte.\n");
            usg_details();
            exit(1);
          }
          else
          {
            end_byte = atoi(optarg) - 1;
            byte_range_flag = 1;

            if(start_byte && (start_byte > end_byte))
            {
              printf("\nStart byte cannot be greater than end byte.\n");
              usg_details();
              exit(1);
            }
          }
        }
        break;

        default:
          printf("\nUnidentifiable error\nExiting Application...\n\n");
          exit(1);
    }
  }

  // If write_flag is true open file and pass through byte_range_checker
  if(write_flag)
  {
    file_descriptor = open(argv[argc - 1], O_RDONLY);

    fstat(file_descriptor, &file_info);
    file_size = file_info.st_size;

    if(file_descriptor < 0)
    {
      printf("\nCould not open file named %s\n", argv[argc - 1]);
      exit(1);
    }

    byte_range_checker(file_size, &start_byte, &end_byte);


    amount_bytes = 1 + end_byte - start_byte;
    amount_bytes_cpy = amount_bytes;
  }


  else
    file_descriptor = open(argv[argc - 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH); // This is in order to have permissions to open file



  if(file_descriptor < 0)
  {
    printf("\nCould not open file named %s\n", argv[argc - 1]);
    exit(1);
  }

  // Setup defaults for the channel
  memset(&channel, 0, sizeof(channel));
  channel.ai_family = AF_INET;
  channel.ai_socktype = SOCK_STREAM;

  if((getaddrinfo(argv[argc - 2], "2345", &channel, &server_channel)) != 0)
  {
    printf("\nGetaddrinfo Function failed: Exiting Program...\n");
    exit(1);
  }

  // defining the ip address variables
  char string_ip[INET_ADDRSTRLEN];
  struct sockaddr_in *ip;
  void *address;

  ip = (struct sockaddr_in *) (*server_channel).ai_addr;

  address = &(ip->sin_addr);

  // Turn IP address into a string which is printable
  inet_ntop((*server_channel).ai_family, address, string_ip, sizeof string_ip);


  // initializing socket variable
  s = socket((*server_channel).ai_family, (*server_channel).ai_socktype,(*server_channel).ai_protocol);

  // general error checking
  if(s < 0)
  {
    printf("\nSocket error:\n");
    exit(1);
  }

  c = connect(s, (*server_channel).ai_addr, (*server_channel).ai_addrlen);

  if(c < 0)
  {
    printf("\nConnect Error:\n");
    exit(1);
  }


  // Writing the arguments to the server for processing on the other side
  write(s, &start_byte, sizeof(start_byte));
  write(s, &end_byte, sizeof(end_byte));
  write(s, &write_flag, sizeof(write_flag));
  write(s, argv[argc - 1], strlen(argv[argc -1])+1);



  // Checking whether writing to or reading from server
  if(write_flag)
  {
    // DEBUG dictates printing defined in file-server.h
    if(DEBUG)
      printf("Sending %s to %s\n", argv[argc - 1], string_ip);

    lseek(file_descriptor, start_byte, SEEK_SET);
 
    read(s, &error_num, sizeof(error_num));

    if(error_num == 1)
    {
      printf("Could not open file %s on server side\n", argv[argc - 1]);
      exit(1);
    }

    while (amount_bytes_cpy != 0)
    {
      if(amount_bytes_cpy > BUF_SIZE)
        bytes = read(file_descriptor, buf, BUF_SIZE);

      else
        bytes = read(file_descriptor, buf, amount_bytes_cpy);
      
      write(s, buf, bytes);
      
      counter = counter + bytes;

      print_percentage(amount_bytes, argv[argc -1], counter);
      
      amount_bytes_cpy = amount_bytes_cpy - bytes;
      if(amount_bytes_cpy == 0)
      {
        if(DEBUG)
          printf("Finished sending %s to %s\n", argv[argc - 1], string_ip);
      }
    }
    

  }

  else
  {
    read(s, &error_num, sizeof(error_num));
    read(s, &file_size, sizeof(file_size));

    // Checking different error types and printing respective message
    if(error_num == 2)
    {
      printf("Couldn't download file: start byte out of range\nThe byte range is frome 1 to %ld\n", file_size);
      exit(1);
    }

    if(error_num == 3)
    {
      printf("Couldn't download file: end byte out of range\nThe byte range is from 1 to %ld\n", file_size);
      exit(1);
    }

    if(error_num == 1)
    {
      printf("Couldn't open the file %s\n", argv[argc-1]);
      exit(1);
    }

  
    byte_range_checker(file_size, &start_byte, &end_byte);

    /*if(error_num == 5)
    {
      printf("Start byte %d is greater than end byte %d\n", start_byte, end_byte);
      exit(1);
    }*/

    amount_bytes_cpy = 1 + end_byte - start_byte;

    // writing file descriptor to server
    while(amount_bytes_cpy != 0)
    {
      if(amount_bytes_cpy > BUF_SIZE)
        bytes = read(s, buf, BUF_SIZE);

      else
        bytes = read(s, buf, amount_bytes_cpy);

      write(file_descriptor, buf, bytes);
      amount_bytes_cpy = amount_bytes_cpy - bytes;
    
    
  }
}
    // free in order to avoid memory leak
    freeaddrinfo(server_channel);
    return 0;
}
