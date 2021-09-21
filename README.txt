Yoseph Hassan
EEL 4781, Fall 2020

Documentation for Programming Project

General Information:
    This project consists of two programs written in C languages which implement communication between a Server and Client via TCP sockets.
    
    The pre-requisites required in order to run these applications correctly are the following:
    Linux / WSL bash shell
    C compiler
    file-server.h file (includes further extensions)
    Makefile
    Seperate file environments for client and server applications

How to run/use:
    Open Linux bash terminal, navigate to directory of project, and run make command.
    Then move the client executable to a seperatedirectory with any files which are desired to be uploaded from the client to server. 
    Open another terminal for the server with two in total. 
    One terminal in the directory of the client's executable and the other terminal in the server's executable.
    
    In order to request a file from the server run the following command:

    ./client server-name file-name

    Request a certain byte-range from server: Run the following command
    ./client server-name file-name -s START_BYTE -e END_BYTE

    Note: if an end byte isn't specified the server will send the file starting for the start byte to the end
    Note: if a start byte isn't specified the server will send the file starting from the beginning to the end byte

    Sending a file to server: Run the folowing command
    ./client server-name -w file-name
    
    Note: Byte Range Specification cannot be used in conjuction with writing to the server

    In order to display messages of file transfer between both entities set the DEBUG variable to 1.
    DEBUG variable is found in file-server.h file and is defined to be 0 by defauly.
    Both the server and client rely on the DEBUG global variable in order to print transfer messages

    After reading/writing from the server the application should terminate on both ends.

Error Detection:
    If either application encounters an error it will be printed to the terminal with a message indicating the
    cause of the error. 
    For instance, if the start byte is greater than the end byte and error will be printed to the terminal saying 
    "Start byte x is greater than end byte y".

    If the user enters incorrect/fewer arguments an error message will be displayed along with a help message with
    samples of how to enter the arguments into the terminal.

    Note: If the program encounters an unknown error a corresponding message will be printed