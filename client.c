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

void* clientSend(void* arg);
void* clientReceive(void* arg);

pthread_t clientSendThread, clientReceiveThread;                                                /*Create threads.*/

int socketDesc;                                                                                 /*Create essentials of communication.*/
struct sockaddr_in serverAddress, clientAddress;
char serverMessage[256], clientMessage[256];

struct Package* list;       /*Array of packages to be sent*/
struct Package received[32];/*Array of packages to be received*/
int sent=0;					/*Counter of sent and ACK'ed packages.*/
int rec=0;					/*Counter of received and not corrupted packages.*/

int main(int argc, char** argv){   
    socketDesc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
    if(socketDesc < 0){
        /*printf("ERROR! Error while creating socket.\n");*/
        return -1;
    }
    /*printf("--Socket created successfully, READY!\n");*/
    
    serverAddress.sin_family = AF_INET;                                                         /*Server info.*/
    serverAddress.sin_port = htons(atoi(argv[2]));
    serverAddress.sin_addr.s_addr = INADDR_ANY;/*inet_addr("127.0.0.1");*/

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(atoi(argv[3]));
    clientAddress.sin_addr.s_addr = inet_addr(argv[1]);

    int bindStatus = bind(socketDesc, (struct sockaddr*)&clientAddress,sizeof(clientAddress));	/*Bind client to the given port*/
	if(bindStatus<0){
		/*printf("ERROR! Couldn't bind to the port.\n");*/
	}
    else{
        /*printf("--Binding done!\n");*/
    }

    if(pthread_create(&clientReceiveThread, NULL, clientReceive, NULL) != 0){                   /*Printf's states the aim of those functions below.*/
        /*printf("ERROR! Couldn't create receive thread.\n");*/
    }
    if(pthread_create(&clientSendThread, NULL, clientSend, NULL) != 0){
        /*printf("ERROR! Couldn't create send thread.\n");*/
    }
    if(pthread_join(clientReceiveThread, NULL) != 0){
        /*printf("ERROR! Couldn't join receive thread.\n");*/
    }
    if(pthread_join(clientSendThread, NULL) != 0){
        /*printf("ERROR! Couldn't join send thread.\n");*/
    }
    close(socketDesc);
    return 0;
}

void *clientSend(void* arg){            /*Sender, which gets inputs from user and sends messages*/
    while(1){                           /*as Packages which is a struct data type.*/
        int sendStatus;
        int size;
        int i,l;
        int w = 4;      /*Window size w and last index in the window px instantly.*/
        int px = w-1;
        int serverStructLength = sizeof(serverAddress);
        struct Package* data;
        struct Package temp;
        memset(clientMessage, '\0', sizeof(clientMessage));
        /*printf(">");*/
        fgets(clientMessage, 256, stdin);
        clientMessage[strcspn(clientMessage, "\n")] = '\0';

        if(!strcmp(clientMessage, "BYE")){
            printf("---ENDING COMMUNICATION!---\n");
            break;
        }

        list = parseMessage(clientMessage, &size);  /*Split message to packages.*/
        
        labelSend:
        for(i=0;i<size;i++){
            if(list[i].gotAck == 0){
                sent++;
                temp.id = list[i].id;
                temp.size = list[i].size;
                temp.checksum = list[i].checksum;
                temp.isAck = list[i].isAck;
                temp.gotAck = list[i].gotAck;
                temp.data[0] = list[i].data[0];
                temp.data[1] = list[i].data[1];
                temp.data[2] = list[i].data[2];
                temp.data[3] = list[i].data[3];
                
                sendStatus = sendto(socketDesc, &temp, sizeof(struct Package), 0,
                                (struct sockaddr*)&serverAddress, serverStructLength);
            }
        }
        if(sent==size){
            sent=0;
            resetList(list,size);
            continue;
        }
        goto labelSend;
    }
    if(pthread_cancel(clientReceiveThread)!=0){
        /*printf("ERROR! Thread cancel.\n");*/
    }
    if(pthread_cancel(clientSendThread)!=0){
        /*printf("ERROR! Thread cancel.\n");*/
    }
    return NULL;
}

void* clientReceive(void* arg){             /*Receiver; which takes messages and send their ACK responses*/
    while(1){                               /*besides get ACK responses to it's messages which are sent*/
        struct Package* data;
        int sendStatus, recvStatus;
        int index;
        int size;
        int i;
        int serverStructLength = sizeof(serverAddress);
        
        recvStatus = recvfrom(socketDesc, data, sizeof(struct Package), 0,
                            (struct sockaddr*)&serverAddress, &serverStructLength);
        if(recvStatus < 0){
            /*printf("ERROR! Couldn't receive.\n");*/
        }

        size = data->size;
        
        if(data->isAck == 0 && check(*data)){   /*If data is server's message.*/
            index = data->id;
            data->isAck = 1;
            received[index].size = data->size;
            received[index].id = data->id;
            received[index].checksum = data->checksum;
            received[index].isAck = 1;
            received[index].gotAck = 0;
            received[index].data[0] = data->data[0];
            received[index].data[1] = data->data[1];
            received[index].data[2] = data->data[2];
            received[index].data[3] = data->data[3];

            serverMessage[4*index] = received[index].data[0];
            serverMessage[4*index+1] = received[index].data[1];
            serverMessage[4*index+2] = received[index].data[2];
            serverMessage[4*index+3] = received[index].data[3];

            rec++;
            if(rec==size){                      /*Halting condition*/
                int j=0;
                rec=0;
                while(j<size){
                    serverMessage[4*j] = received[j].data[0];
                    serverMessage[4*j+1] = received[j].data[1];
                    serverMessage[4*j+2] = received[j].data[2];
                    serverMessage[4*j+3] = received[j].data[3];
                    j++;
                }
                printf("Server's message: \x1B[32m%s\x1B[0m\n", serverMessage);
                if(!strcmp(serverMessage, "BYE")){
                    printf("---ENDING COMMUNICATION!---\n");
                    break;
                }
                resetList(received, 32);        /*Reset the array of packages for next incoming message.*/
                memset(serverMessage, '\0', sizeof(serverMessage));
                continue;
            }

            sendStatus = sendto(socketDesc, data, sizeof(struct Package), 0,    /*If there are more packages to be come.*/
						(struct sockaddr*)&serverAddress, serverStructLength);
        }
        else if(data->isAck == 1 && check(*data)){  /*If incoming data is an ACK response to sent data.*/
            index = data->id;
            list[index].isAck = 1;
            list[index].gotAck = 1;
            sent++;                             /*Increment the count of ACK got from receiver.*/
        }        
    }
    if(pthread_cancel(clientSendThread)!=0){
        /*printf("ERROR! Thread cancel.\n");*/
    }
    if(pthread_cancel(clientReceiveThread)!=0){
        /*printf("ERROR! Thread cancel.\n");*/
    }
    return NULL;
}