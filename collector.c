#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "collector.h"
#include "list.h"

void cleanup() {
    unlink(SOCKNAME);
}

//funzione di scambio a e b
void swap(file_structure *a, file_structure *b){
    long temp = a->value; // inverto i long
    a->value = b->value;
    b->value = temp;

    char*aus = a->info;//inverto i char
    a->info= b->info;
    b->info = aus;
}

//bubble sort dei long
void bubbleSort(file_structure *head){
    int swapped;
    struct file_structure *cor;
    struct file_structure *prec= NULL;

    //controllo se lista == NULL
    if (head == NULL)
        return;

    do{
        swapped = 0;
        cor = head;

        while (cor->next != prec){
            if (cor->value > cor->next->value){
                swap(cor, cor->next);
                swapped = 1;
            }
            cor = cor->next;
        }
        prec = cor;
    }
    while (swapped);
}

void* socket_collector(void *arg){
    struct sockaddr_un *serv_addr=((threadArgs_t*)arg)->serv_addr;

    llist *lista_ricevente = NULL;
    file_structure *List_to_order= NULL;

    // cancello il socket file se esiste
    cleanup();
    // se qualcosa va storto ....
    atexit(cleanup);
    int listenfd;
    // creo il socket
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket","");
    int notused;
    char buffer[BUFSIZE];
    char buffer2[BUFSIZE];

    // assegno l'indirizzo al socket
    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*)serv_addr,sizeof(*serv_addr)), "bind", "");
    // setto il socket in modalita' passiva e definisco un n. massimo di connessioni pendenti
    SYSCALL_EXIT("listen", notused, listen(listenfd, SOMAXCONN), "listen","");

    int connfd;

    SYSCALL_EXIT("accept", connfd, accept(listenfd, (struct sockaddr*)NULL ,NULL), "accept","");

    for(int i=0; i<7;i++){
        read(connfd,buffer,BUFSIZE);
        printf("CLIENT rcvd: %s\n",buffer);
    }

    return NULL;
}