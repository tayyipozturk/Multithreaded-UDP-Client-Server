#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

struct Package* parseMessage(char* message, int* size); /*Split string to packages which holds 4 byte char data and other data*/
char* assembleMessage(struct Package* list);  /*Is not used. Implemented by hand inside receive functions on both server and client.*/
int check(struct Package data); /*Check if calculated checksum is equal to checksum var of Package*/
void resetList(struct Package* list, int size); /*Prepared to set all the variables inside a Package in an array 0.*/

struct Package{
    char data[4];   /*4 bytes of character array*/
    bool isAck; /*Set 1 while sending ACK back to the sender. So that the package comes from receiver can be distinguisged as it is a message or ACK.*/
    bool gotAck;    /*If the ACK is got or not.*/
    short id;   /*Priority of the package in the Package array, implying the place of characters in the message.*/
    short size; /*Number of packages*/
    int checksum; /*As it is stated.*/
};