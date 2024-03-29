//
// Created by rebecca on 02/01/23.
//

#ifndef SOL_LIST_H
#define SOL_LIST_H

#include <pthread.h>

typedef struct llist{
    char* opzione;
    struct llist *next;
} llist;

typedef struct Queue {
    llist       *head;
    llist       *tail;
    unsigned long  qlen;
    pthread_mutex_t qlock;
    pthread_cond_t  qcond;
} Queue_t;

typedef struct list_integer{
    long info;
    struct list_integer *next;
}list_integer;

typedef struct file_structure{
    long value;
    char* info;
    struct file_structure* next;
}file_structure;

typedef struct threadArgs {
    int      thid;
    Queue_t *q;
    llist *l;
    struct sockaddr_un *serv_addr;
    int lenght_tail_list;
    int tempo_di_invio;
} threadArgs_t;

//funzioni gestione coda q

Queue_t *initQueue();

void deleteQueue(Queue_t *q);

char *dequeue(Queue_t *q);

void StampaLista(Queue_t *q);

int push(Queue_t *q, void *data);



//funzioni gestione llist

void delete_head_lista_piena(struct llist** head,char* data);

void add_list_new(struct llist** head, char*opzione);

void listdir(const char *name,struct llist *l);

void Look_for_file(char* filename, char* directorydipartenza, llist*l);

void insert_list(struct llist** head, char * opzione);

void canc(llist** head);

void insert_signal(llist**l,char* opzione);

int listLength(llist *head);

void print_list(struct llist* head);

void linked_list_destroy(llist *linked_list);

char* primo_elemento (llist* l);

llist *clone(llist *l);



//funzioni gestione list_integer

void insert_integer(list_integer**l, long el);

void print_integer(list_integer*l);

void integer_list_destroy(list_integer *head);



//funzioni gestione file_structure

file_structure *split_file(llist* l,file_structure* head);

void insert_file(file_structure** head,char* opzione, long num);

void print_file(file_structure* head);

void file_list_destroy(file_structure *head);

void file_list_destroy_temp(file_structure *head);

void removeChar(char * str, char charToRemmove);

void print_file_2(file_structure* head);

#endif //SOL_LIST_H