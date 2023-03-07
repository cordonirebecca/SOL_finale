//
// Created by rebecca on 02/01/23.
//

#ifndef SOL_LIST_H
#define SOL_LIST_H

#include <pthread.h>
#define NUM_STRING 10

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

Queue_t *initQueue();

void deleteQueue(Queue_t *q);

void StampaLista(Queue_t *q);

int push(Queue_t *q, void *data);

char* file_singolo_da_inserire(struct llist* head);

void delete_head_lista_piena(struct llist** head,char* data);

void listdir(const char *name, int indent,struct llist *l);

void Look_for_file(char* filename, char* directorydipartenza, int indent,llist*l);

void insert_list(struct llist** head, char * opzione);

void canc(llist** head);

void add_list(struct llist* head, char * opzione);

void print_list(struct llist* head);

char *dequeue(Queue_t *q);

unsigned long length(Queue_t *q);

void insert_integer(list_integer**l, long el);

void print_integer(list_integer*l);

void insert_signal(llist**l,char* opzione);

void add_list_flag(struct llist** head, char * opzione,char* var);

int listLength(llist *item);

void removeChar(char * str, char charToRemmove);

file_structure *split_file(llist* l,file_structure* head);

void bubbleSort(file_structure *start);

void list_int_to_char(list_integer* lI, llist* l);

void insert_file(file_structure** head,char* opzione, long num);

void print_file(file_structure* head);

void linked_list_destroy(llist *linked_list);

void integer_list_destroy(list_integer *head);

void file_list_destroy(file_structure *head);

#endif //SOL_LIST_H
