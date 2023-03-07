#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "collector.h"
#include "list.h"
#include "auxiliaryMW.h"

void cleanup() {
    unlink(SOCKNAME);
}

void* socket_collector(void *arg){ // il server non risponde al client
    struct sockaddr_un *serv_addr=((threadArgs_t*)arg)->serv_addr;
    int lenght_tail_list=((threadArgs_t*)arg)->lenght_tail_list;


    // cancello il socket file se esiste
    cleanup();
    // se qualcosa va storto ....
    atexit(cleanup);

    llist *List_to_stamp=malloc(sizeof(llist));
    List_to_stamp->next = NULL;
    List_to_stamp->opzione = NULL;

    file_structure *List_to_order= malloc(sizeof(file_structure));
    List_to_order->next=NULL;
    List_to_order->info=NULL;
    List_to_order->value=0;

    int listenfd=0;
    // creo il socket
    SYSCALL_EXIT("socket", listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket","");

    int notused=0;
    char buffer[BUFSIZE];
    // assegno l'indirizzo al socket
    SYSCALL_EXIT("bind", notused, bind(listenfd, (struct sockaddr*)serv_addr,sizeof(*serv_addr)), "bind", "");

    // setto il socket in modalita' passiva e definisco un n. massimo di connessioni pendenti
    SYSCALL_EXIT("listen", notused, listen(listenfd, SOMAXCONN), "listen","");

    int connfd = -1;
    int help = 0;
    int counter = 1;
    do {
        connfd=accept(listenfd, (struct sockaddr*)NULL ,NULL);
        read(connfd,buffer,BUFSIZE);
        //printf("\n\ncollector got: %s\n\n",buffer);
        insert_list(&List_to_stamp,buffer);

        //write(connfd,"received",9);
        //close(listenfd);
        //printf("connection done [%d]\n", counter);
        counter++;
        help ++;
        if(help == lenght_tail_list){ // numero di argomenti che ho
            //ordino gli elementi in modo crescente

            //mi prendo solo i numeri della lista e li inserisco in una nuova lista
            List_to_order=split_file(List_to_stamp->next,List_to_order);

            //printf("lista guardata:\n\n");
            //print_file(List_to_order);
            // printf("\n\n");

            //ordino la lista di int
            bubbleSort(List_to_order->next);

            print_file(List_to_order->next);
            // printf("\n\n");

            break;
        }

        if( strcmp(buffer,"STOP")==0){
            //printf("ricevuto il segnale nel collector, mi fermo \n\n");
            break;
        }

        close(connfd);
    } while(1);


    linked_list_destroy(List_to_stamp);
    file_list_destroy(List_to_order);

    //printf("fine collector\n\n");
    return NULL;
}