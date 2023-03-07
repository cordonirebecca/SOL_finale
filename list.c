//
// Created by rebecca on 02/01/23.
//
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

static inline void freeNode(llist *node)           { free((void*)node); }
#define EXTRA_LEN_PRINT_ERROR   512

Queue_t *initQueue() {
    Queue_t *q = malloc(sizeof(Queue_t));
    if (!q) return NULL;
    q->head = malloc(sizeof(llist));
    if (!q->head) return NULL;
    q->head->opzione = NULL;
    q->head->next = NULL;
    q->tail = q->head;
    q->qlen = 0;
    if (pthread_mutex_init(&q->qlock, NULL) != 0) {
        perror("mutex init");
        return NULL;
    }
    if (pthread_cond_init(&q->qcond, NULL) != 0) {
        perror("mutex cond");
        if (&q->qlock) pthread_mutex_destroy(&q->qlock);
        return NULL;
    }
    return q;
}

void deleteQueue(Queue_t *q) {
    while(q->head != q->tail) {
        llist *p = (llist*)q->head;
        q->head = q->head->next;

        freeNode(p);
    }
    if (q->head) freeNode((void*)q->head);
    if (&q->qlock)  pthread_mutex_destroy(&q->qlock);
    if (&q->qcond)  pthread_cond_destroy(&q->qcond);
    free(q);
}


void StampaLista(Queue_t *q){
    llist *temp = q->head;
    printf("Lista della codona :");
    while (temp != NULL){
        printf(" %s -> ", temp->opzione);
        temp = temp->next;
    }
    printf("\n");
}

int push(Queue_t *q, void *data) {
    if ((q == NULL) || (data == NULL)) {
        errno= EINVAL;
        return -1;
    }
    llist *n = malloc(sizeof(llist));
    if (!n){
        return -1;
    }
    n->opzione = data;
    n->next = NULL;
    //lock queue
    if (pthread_mutex_lock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE lock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    q->tail->next = n;
    q->tail       = n;
    q->qlen+= 1;
    //mando segnale che c'è un valore in lista
    if (pthread_cond_signal(&q->qcond)!=0){
        fprintf(stderr, "ERRORE FATALE signal\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    //sblocco queue
    if (pthread_mutex_unlock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE unlock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    return 0;
}


void delete_head_lista_piena(struct llist** head,char* data){
    if(*head != NULL){
        if((*head)->opzione == data){
            //cancello primo elemento uguale a data
            llist * aus= *head;
            *head=(*head)->next;
            //free(aus->opzione);
            free(aus);
        }
        else{
            delete_head_lista_piena(&(*head)->next,data);
        }
    }
}

char* file_singolo_da_inserire(struct llist* head){
    if(head == NULL){
        return NULL;
    }
    return head->opzione;
}

//stampa la lista che passa al masterworker, che poi spezza  e inserisce nella coda grande
void print_list(struct llist* head){

    llist *aus=head;
    printf("Lista: \n");
    while (aus != NULL){
        printf(" %s ", aus->opzione);
        printf("\n");
        aus = aus->next;
    }
    printf("NULL");
}

void add_list(llist* head, char * opzione){
    struct llist *new= malloc(sizeof(struct llist));
    struct llist *nodoCorrente= malloc(sizeof(struct llist));

    int len= strlen(opzione)+1;
    new->opzione=malloc(len* sizeof(char));
    strncpy(new->opzione,opzione,strlen(opzione));
    new->next=NULL;
    if((head->opzione)==NULL) {
        (head->opzione) = (new->opzione);
    }else{
        nodoCorrente=head;
        while(nodoCorrente->next != NULL){
            nodoCorrente=nodoCorrente->next;
        }
    }
    nodoCorrente->next=new;
}

void add_list_flag(struct llist** head, char * opzione,char* var){
    struct llist *new= malloc(sizeof(struct llist));
    struct llist *nodoCorrente;

    new->opzione=malloc(80* sizeof(char));
    strcpy(new->opzione,opzione);
    strcat(new->opzione,var);
    new->next=NULL;
    if(*head == NULL) {
        ((*head)->opzione) = (new->opzione);
    }else{
        nodoCorrente=*head;
        while(nodoCorrente->next != NULL){
            nodoCorrente=nodoCorrente->next;
        }
    }
    nodoCorrente->next=new;
}

void insert_list(struct llist** head, char *opzione){
    struct llist *new= (llist *)malloc(sizeof(struct llist));
    new->next=NULL;
    int len= strlen(opzione)+1;
    new->opzione=malloc(len* sizeof(char));
    struct llist *nodoCorrente= *head;

    strncpy(new->opzione,opzione,len);
    if(*head == NULL) {
        *head = new;
        return;
    }else{
        while(nodoCorrente->next != NULL){
            nodoCorrente=nodoCorrente->next;
        }
    }
    nodoCorrente->next=new;
}


//funzione che apre tutte le cartelle e mi stampa i file in ognuna
void listdir(const char *name, int indent,struct llist *l){
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            //path è il percorso directory senza i file
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            //printf("PATH: %s\n\n",entry->d_name);
            //printf("%*s[%s]\n", indent, "", entry->d_name);
            listdir(path, indent + 2,l);
        } else {
            // sono uno o più file nella directory
            //printf("%*s- %s\n", indent, "", entry->d_name);
            //qui riempio la lista che poi passo al producer
            add_list_flag(&l,entry->d_name,"S");
        }
    }
    closedir(dir);
}


//ritorno il path di un determinato file inserendolo nella lista l
//aggiorno data che è il file.txt
void Look_for_file(char* filename, char* directorydipartenza, int indent,struct llist*l){
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir(directorydipartenza))){
        return;
    }
    while ((entry = readdir(dir)) != NULL) {

        if (entry->d_type == DT_DIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0){
                continue;
            }
            snprintf(path, sizeof(path), "%s/%s", directorydipartenza, entry->d_name);
            Look_for_file(filename,path, indent + 2,l);
        } else {
            if((strcmp(entry->d_name,filename))==0){
                if(directorydipartenza !=NULL){
                    //printf("directory: %s, strlen directory: %ld\n\n", directorydipartenza,strlen(directorydipartenza));
                    //inserisco nella lista i miei path risultato.
                    insert_list(&l,directorydipartenza);
                }
            }
        }
    }
    closedir(dir);
}

//va cancellato tutto tranne l'ultimo elemento
void canc(llist** head){
    llist *cor=*head;
    int n =0;
    while(*head != NULL){ //conto tutti gli elementi della lista
        n++;
        *head=(*head)->next;
    }
    int c=n-1; //elementi della lista da cancellare, tutti tranne l'ultimo !
    while(c>0){
        c=c-1;
        *head=cor->next;
        free(cor);
        cor=*head;
    }
}

//mi elimina il primo elemento della lista e me lo ritorna, così il worker se lo prende
char *dequeue(Queue_t *q) {
    if (q == NULL) {
        errno= EINVAL;
        return NULL;
    }
    //blocco coda
    if (pthread_mutex_lock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE lock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    //finchè è vuota
    while(q->head == q->tail) {
        //unlock queue and wait
        if (pthread_cond_wait(&q->qcond, &q->qlock) != 0) {
            fprintf(stderr, "ERRORE FATALE wait\n");
            pthread_exit((void *) EXIT_FAILURE);
        }
    }
    //qui è tutto bloccato
    assert(q->head->next);
    llist *n  = (llist *)q->head;
    void *data = (q->head->next)->opzione;
    q->head    = q->head->next;
    q->qlen   -= 1;
    assert(q->qlen>=0);
    //unlock queue
    if (pthread_mutex_unlock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE unlock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    //libero lo spazio
    freeNode(n);
    return data;
}

unsigned long length(Queue_t *q) {
    //lockqueue
    if (pthread_mutex_lock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE lock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    unsigned long len = q->qlen;
    // UnlockQueue
    if (pthread_mutex_unlock(&q->qlock)!=0){
        fprintf(stderr, "ERRORE FATALE unlock\n");
        pthread_exit((void*)EXIT_FAILURE);
    }
    return len;
}


void insert_integer(list_integer**l, long el){
    list_integer *newlist=(list_integer*)malloc(sizeof(list_integer));
    list_integer *ultimo;
    newlist->info=el;
    newlist->next=NULL;
    if(*l == NULL){
        *l= newlist;
    }
    else{
        ultimo=*l;
        while(ultimo->next != NULL){
            ultimo=ultimo->next;
        }
        ultimo->next=newlist;
    }
}

void print_integer(list_integer*l){
    while(l!=NULL){
        printf("%ld->",l->info);
        l=l->next;
    }
}


void insert_signal(llist**l,char* opzione){
    int i=0;
    if(*l != NULL){
        if(i%2 == 0){ // numero pari
            i++;
            llist *aus=malloc(sizeof(llist));
            int len= strlen(opzione)+1;
            aus->opzione=malloc(len* sizeof(char));
            strncpy(aus->opzione,opzione, strlen(opzione));
            aus->next=*l;
            *l=aus;
            insert_signal(&(*l)->next->next,opzione);
        }
        else{
            i++;
            insert_signal(&(*l)->next,opzione);
        }
    }
}

void removeChar(char * str, char charToRemmove){
    int i, j;
    int len = strlen(str);
    for(i=0; i<len; i++)
    {
        if(str[i] == charToRemmove)
        {
            for(j=i; j<len; j++)
            {
                str[j] = str[j+1];
            }
            len--;
            i--;
        }
    }
}


int listLength(llist *item){
    llist * cur = item;
    int size = 0;

    while (cur != NULL){
        ++size;
        cur = cur->next;
    }

    return size;
}

//inserisco nella lista di interi solo la parte numerica dei char
file_structure *split_file(llist* l,file_structure* head){
    while(l != NULL){
        long y = atol(l->opzione);
        //mi salvo in una variabile ret la parte char da riattaccare
        const char ch = ' ';
        char *ret;
        ret = strchr(l->opzione, ch);

        //printf("\nThe integer value of y is %ld", y);
        //printf("\nThe char value of y id %s\n",ret);
        l=l->next;
        //inserisco gli integer nella lista per fare il sort
        insert_file(&head,ret,y);
    }

    return head;
}

/* function to swap data of two nodes a and b*/
void swap(file_structure *a, file_structure *b){
    long temp = a->value;
    a->value = b->value;
    b->value = temp;

    char*aus = a->info;
    a->info= b->info;
    b->info = aus;
}

/* Bubble sort the given linked list */
void bubbleSort(file_structure *start){
    int swapped;
    struct file_structure *ptr1;
    struct file_structure *lptr= NULL;

    /* Checking for empty list */
    if (start == NULL)
        return;

    do{
        swapped = 0;
        ptr1 = start;

        while (ptr1->next != lptr){
            if (ptr1->value > ptr1->next->value){
                swap(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    }
    while (swapped);
}


void insert_file(file_structure** head,char* opzione, long num){
    struct file_structure *new= (file_structure *)malloc(sizeof(struct file_structure));
    new->value=-1;
    new->next=NULL;
    new->info=NULL;
    file_structure *nodoCorrente;

    int len=strlen(opzione)+1;
    new->info=malloc(len* sizeof(char));
    strcpy(new->info,opzione);
    new->value=num;
    new->next=NULL;
    if((*head)==NULL) {
        *head = new;
    }else{
        nodoCorrente=*head;
        while((nodoCorrente->next) != NULL){
            nodoCorrente=nodoCorrente->next;
        }
    }
    nodoCorrente->next=new;
}

void print_file(file_structure* head){

    while(head != NULL){
        printf("%ld %s\n",head->value,head->info);
        head=head->next;
    }
}


void linked_list_destroy(llist *head){
    while (head != NULL){
        struct llist* tmp = head;
        head = head->next;
        free(tmp->opzione);
        free(tmp);
    }
}

void integer_list_destroy(list_integer *head){
    struct list_integer * tmp;
    while (head != NULL){
        tmp = head;
        head = head->next;
        free(tmp);
    }
}

void file_list_destroy(file_structure *head){
    struct file_structure *tmp;
    while(head != NULL){
        tmp=head;
        head=head->next;
        free(tmp->info);
        free(tmp);
    }
}
