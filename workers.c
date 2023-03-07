//
// Created by rebecca on 23/12/22.
//
#include "workers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/stat.h>
#include "auxiliaryMW.h"
#include <ctype.h>
#include <limits.h>
#include "list.h"
#include<dirent.h>
#include <assert.h>
#include <stdbool.h> // for bool
#include <math.h>
#include <ctype.h> // for isspace
#define UNIX_PATH_MAX 255

//salvo i numeri in un array e faccio sommatoria
long sommatoria(char* file_name){
    FILE *fd;
    long x=0;
    int res=0;
    long sum=0;

    list_integer *list_file=NULL;

    int i=0;

    /* apre il file */
    fd=fopen(file_name, "r+");
    if( fd==NULL ) {
        perror("Errore in apertura del file");
        exit(1);
    }


    /* ciclo di lettura */
    while(1) {
        /* legge un intero */
        res=fread(&x, sizeof(long), 1, fd);
        if( res!=1 )
            break;
        insert_integer(&list_file,x);//inserisco in lista i miei valori
    }


    // faccio sommatoria dei valori
    while(list_file != NULL){
        list_integer *tmp=list_file;
        sum=sum+(list_file->info)*i;
        list_file=list_file->next;
        free(tmp);
        i++;
    }



    /* chiude il file */
    fclose(fd);

    return sum;
}