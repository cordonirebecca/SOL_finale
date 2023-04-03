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


    // cancello il socket file se esiste
    cleanup();
    // se qualcosa non va
    atexit(cleanup);

    llist *List_to_stamp = NULL;
    file_structure *List_to_order= NULL;

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

        if( strcmp(buffer,"STOP")==0){ // ho ricevuto Sigusr1 mi fermo e stampo solo quello che ho ricevuto
            //insert_list(&List_to_stamp,buffer);

            List_to_order=split_file(List_to_stamp,List_to_order);

            //ordino la lista di int
            bubbleSort(List_to_order);

            //stampo lista finale e inetrrompo il ciclo
            print_file(List_to_order);

            //printf("ricevuto il segnale nel collector, mi fermo \n\n");

        }
        else{ // non ho ricevuto sigusr1
            //printf("\n\ncollector got: %s\n\n",buffer);

            //inserisco gli elementi arriavti in list_to_stamp
            insert_list(&List_to_stamp,buffer);

            //write(connfd,"received",9);
            //close(listenfd);
            //printf("connection done [%d]\n", counter);
            counter++;
            help ++;
            if(help == lenght_tail_list){ // numero totale di file che ho in input
                //ordino gli elementi in modo crescente

                //mi prendo solo i numeri della lista e li inserisco in una nuova lista
                List_to_order=split_file(List_to_stamp,List_to_order);

                //ordino la lista di int
                bubbleSort(List_to_order);

                //stampo lista finale e inetrrompo il ciclo
                print_file(List_to_order);
                break;
            }
        }

        close(connfd);
    } while(1);


    linked_list_destroy(List_to_stamp);
    file_list_destroy(List_to_order);

    return NULL;
}