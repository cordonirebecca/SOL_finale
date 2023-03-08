//
// Created by rebecca on 16/09/22.
//
#ifndef SOL_COLLECTOR_H
#define SOL_COLLECTOR_H

#define SOCKNAME     "./farm.sck"
#include <sys/un.h>
#include <sys/socket.h>
#define BUFSIZE 256
#include "list.h"

#define SYSCALL_EXIT(name, r, sc, str, ...)	\
    if ((r=sc) == -1) {				\
	perror(#name);				\
	int errno_copy = errno;			\
	exit(errno_copy);			\
    }


void bubbleSort(file_structure *start);

void cleanup();

void* socket_collector(void *arg);

#endif //SOL_COLLECTOR_H