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
#include "functions.h"

struct Package* parseMessage(char* message, int* size){                /*PARSE MESSAGE TO BE SEND TO 4 BYTES OF PACKAGES OF LIST*/
    int i = 0;
    int j = 0;
    int len;;
    int n;
    struct Package* list;
    
    len = strlen(message);
    if(len%4==0){
        n=len/4;
    }
    else{
        n=1+len/4;
    }
    *size = n;
    list = malloc(n*sizeof(struct Package));

    for(i=0;i<n;i++){
        list[i].data[0] = message[j++];
        list[i].data[1] = message[j++];
        list[i].data[2] = message[j++];
        list[i].data[3] = message[j++];
        /*printf("Package %d: %c%c%c%c\n",i, list[i].data[0],list[i].data[1],list[i].data[2],list[i].data[3]);*/

        list[i].id = i;
        list[i].size = n;
        list[i].isAck = 0;
        list[i].gotAck = 0;
        list[i].checksum = list[i].id + 2*list[i].size + 
                            7*list[i].isAck + 8*list[i].gotAck +
                            (int)list[i].data[0] + (int)list[i].data[1] +
                            (int)list[i].data[2] + (int)list[i].data[3];
    }
    //printf("f:%d",*size);   
    return list;
}

char* assembleMessage(struct Package* list){
    int i;
    int n = sizeof(list)/sizeof(list[0]);
    char* message = malloc(4*n*sizeof(char));
    char* p = message;
    for(i=0;i<n;i++){
        if((list[i].isAck) && (list[i].id == i)){
            strncpy(p, list[i].data, 4);
            p += 4;
        }
    }
    return message;
}

int check(struct Package data){
    int temp = data.id + 2*data.size + 7*data.isAck + 8*data.gotAck +
                (int)data.data[0] + (int)data.data[1] +
                (int)data.data[2] + (int)data.data[3];
    return (data.checksum == temp);
}

void resetList(struct Package* list, int size){
    int i;
    for(i=0;i<size;i++){
        memset(list[i].data, '\0', 4);
        list[i].id = 0;
        list[i].size = 0;
        list[i].isAck = 0;
        list[i].gotAck = 0;
        list[i].checksum = 0;
    }
}