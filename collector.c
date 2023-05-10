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
    int lenght_tail_list=((threadArgs_t*)arg)->lenght_tail_list;

    llist *lista_ricevente = NULL;
    file_structure *List_to_order= NULL;
    file_structure *List_stamp_before = NULL;

    // cancello il socket file se esiste
    cleanup();
    // se qualcosa va storto ....
    atexit(cleanup);
    int listenfd;
    // creo il socket
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket","");
    int notused;
    char buffer[BUFSIZE];

    // assegno l'indirizzo al socket
    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*)serv_addr,sizeof(*serv_addr)), "bind", "");
    // setto il socket in modalita' passiva e definisco un n. massimo di connessioni pendenti
    SYSCALL_EXIT("listen", notused, listen(listenfd, SOMAXCONN), "listen","");

    int connfd;

    SYSCALL_EXIT("accept", connfd, accept(listenfd, (struct sockaddr*)NULL ,NULL), "accept","");

    for(int i=0; i<lenght_tail_list;i++){
        read(connfd,buffer,BUFSIZE);
        //printf("CLIENT rcvd: %s\n",buffer);
        if(strchr(buffer,'F') != NULL){ // se ricevo il file con F = SIGUSR1
            removeChar(buffer,'F');
            List_stamp_before=split_file(lista_ricevente,List_stamp_before);
            bubbleSort(List_stamp_before);
            print_file_2(List_stamp_before);
            printf("\n");
        }
        if(strcmp(buffer,"STOP") != 0){ //finchè non ricevo STOP = SIGINT...
            insert_list(&lista_ricevente,buffer);
        }
    }

    List_to_order=split_file(lista_ricevente,List_to_order);
    //ordino la lista di int
    bubbleSort(List_to_order);
    print_file(List_to_order);

    linked_list_destroy(lista_ricevente);
    file_list_destroy(List_to_order);
    file_list_destroy(List_stamp_before);

    //printf("FINE COLLECTOR\n\n");

    return NULL;
}
