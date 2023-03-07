//
// Created by rebecca on 07/11/22.
//

#ifndef SOL_AUXILIARYMW_H
#define SOL_AUXILIARYMW_H
#include "list.h"

// tipo di dato usato per passare gli argomenti al thread
typedef struct threadArgs {
    int      thid;
    Queue_t *q;
    llist *l;
    struct sockaddr_un *serv_addr;
    int lenght_tail_list;
} threadArgs_t;


char* getPathAssoluto(char* directoryName);

void int_to_char(int integer,char **res);

void long_to_char(long number, char** res);


#endif //SOL_AUXILIARYMW_H
