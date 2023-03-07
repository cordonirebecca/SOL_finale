//
// Created by rebecca on 07/11/22.
//
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
#include<dirent.h>
#include "list.h"

#define UNIX_PATH_MAX 255

#define ec_meno1(s,m) \
    if((s)==-1) {perror(m); exit(EXIT_FAILURE); \
    }

#define ec_null(s,m) \
    if((s)==NULL) {perror(m); exit(EXIT_FAILURE); \
    }



//funzione ausiliaria che mi converte numero in char per la strcpy
void int_to_char(int integer,char **res){
    int temp=0,count=0,i,cnd=0;
    char ascii[10]={0};

    if(integer>>31){
        /*CONVERTING 2's complement value to normal value*/
        integer=~integer+1;
        for(temp=integer;temp!=0;temp/=10,count++);
        ascii[0]=0x2D;
        count++;
        cnd=1;
    }
    else
        for(temp=integer;temp!=0;temp/=10,count++);
    for(i=count-1,temp=integer;i>=cnd;i--){
        ascii[i]=(temp%10)+0x30;
        temp/=10;
    }
    *res=ascii;
}


