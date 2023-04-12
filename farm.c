// 17/09/2022
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <assert.h>
#include <signal.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/stat.h>
#include "workers.h"
#define UNIX_PATH_MAX 256
#include "list.h"
#include "collector.h"

#define ec_null(s,m) \
    if((s)== 0) {perror(m); exit(EXIT_FAILURE); \
    }

// Global variables
int numberThreads=4;
int lenghtTail =8;
char* directoryName = NULL;
char* file_name;
int tempo=0;
pthread_t maskProducer;
int i=0;
llist *List_to_insert;
int termina=0;
int termina_prima= 0;


typedef struct{
    llist *lista;
}DATA;

DATA risultato_da_inviare;

// funzione eseguita dal signal handler thread
static void *sigHandler_func(void *arg) {
    // printf("\nnel sigHandler_func\n\n");
    sigset_t *set = (sigset_t*)arg;

    while(1) {
        int sig;
        int r = sigwait(set, &sig);
        if (r != 0) {
            errno = r;
            perror("FATAL ERROR 'sigwait'");
            return NULL;
        }

        switch(sig) {
            case SIGHUP:
            case SIGINT:
            case SIGTERM:
            case SIGQUIT:
                termina = 1;
                //printf("segnale modificato: %d\n",termina);

                return NULL;
            case SIGUSR1:
                termina_prima = 1;
                //printf("segnale modificato: %d\n",termina_prima);
                return NULL;
            default:
                return NULL;
        }
    }
    return NULL;
}


// funzione eseguita dal thread produttore
void *Producer(void *arg) {
    Queue_t *q  = ((threadArgs_t*)arg)->q;
    int t = ((threadArgs_t*)arg)->tempo_di_invio;
    // int   myid  = ((threadArgs_t*)arg)->thid;
    llist *l=((threadArgs_t*)arg)->l;
    int lenght_tail_list=((threadArgs_t*)arg)->lenght_tail_list;
    char *data;

    //l contiene tutti i file da inserire, è il list_to_insert del main

    for(int i=0;i<lenght_tail_list; ++i) {
        if(termina == 0){ // inserisco tutto tranquillamente
            //estraggo una alla volta i data dalla lista e li inserisco in modo concorrente in q
            data=l->opzione;
           // printf("DATA [%d]: %s\n\n",i,data);
            sleep(t/1000);
            if (push(q, data) == -1) {
                fprintf(stderr, "Errore: push\n");
                free(data);
                pthread_exit(NULL);
            }
            //elimino primo elemento dalla lista
            delete_head_lista_piena(&l,data); //libero la lista piano piano
        }
        else{ //è arrivato un segnale come sigint, smetto di inserire i dati e faccio finire quelli che erano in coda
            sleep(1);
            push(q,"STOP");
           // free(data);
        }
        //printf("Producer %d pushed <%d>\n", myid, i);
    }
    //printf("Producer%d exits\n", myid);
    return NULL;
}

// funzione eseguita dal thread consumatore
void *Consumer(void *arg) {
    Queue_t *q  = ((threadArgs_t*)arg)->q;
    //  int   myid  = ((threadArgs_t*)arg)->thid;

    char *path_socket=malloc(sizeof(char)*(UNIX_PATH_MAX));
    char *aus=malloc(sizeof(char )*(UNIX_PATH_MAX));
    struct llist *l= malloc(sizeof(llist));
    l->opzione=NULL;
    l->next=NULL;

    long sommatoria_risultato=0;
    size_t consumed=0;
    int unavolta=0;

    do {
        char* data= NULL;
        //la funzione dequeue mi restituisce il primo elemento della lista con i file
        data = dequeue(q);
        //printf("DATA IN WORKER: %s\n\n",data);
        assert(data);
        if (strcmp(data,"fine")== 0) {
            // free(data);
            //printf("entro nel break\n");
            break;
        }
        if(strcmp(data,"STOP")== 0 ){ //stampo quello che ho ricevuto fino a quel momento
            if(unavolta == 0){
                strncpy(path_socket,"STOP",5);
                //printf("path socket : %s\n\n",path_socket);
                unavolta= 1;
            }
            else{
                break;
            }
        }
        else{
            //se c'è il carattere S allora attacco path
            if(strchr(data, 'S') != NULL){
                //pulisco data togliendo S
                removeChar(data,'S');
                //uso la look_for_file per riportare nella lista l il percorso file
                Look_for_file(data,directoryName,l);
                //pulisco tutta la lista lasciando solo l'ultimo elemento.
                canc(&l); // mi rimane l'ultimo elemento da fare la free
                strncpy(aus,l->opzione, strlen(l->opzione)+1);
                strcat(aus,"/");
                strcat(aus,data);
                // printf("NEL WORKER: %s\n\n",aus);
            }
            else{ // caso in cui il file è senza path
                strncpy(aus,data,strlen(data)+1);
            }

            sommatoria_risultato= sommatoria(aus);
            //printf("RISULTATO SOMMATORIA: %ld, %s\n",sommatoria_risultato,aus);

            //mi preparo il messaggio da inviare col socket
            //mi converto long in char* con sprintf
            sprintf(path_socket,"%ld",sommatoria_risultato);
            strcat(path_socket, " ");
            strcat(path_socket,aus);
        }

        ++consumed;
        //printf("Consumer %d: estratto <%s>\n", myid, data);

        insert_list(&risultato_da_inviare.lista,path_socket);
    }while(1);

    //close(sockfd);
    free(path_socket);
    free(aus);
    linked_list_destroy(l);

    return NULL;
}

//comandi del parser
void *parser(int argc, char*argv[],llist **List_to_insert){
    int c;
    opterr=0;

    while((c = getopt(argc, argv, "n:q:d:t:")) != -1){
        switch(c){
            case 'n':
                ec_null(c,"main");
                numberThreads = atoi(optarg);
                // printf("numberThreads: %d\n", numberThreads);
                break;
            case 'q':
                ec_null(c,"main");
                lenghtTail = atoi(optarg);
                //  printf("lenghtail: %d\n", lenghtTail);
                break;
            case 'd':
                ec_null(c,"main");
                directoryName= optarg;
                //getPathAssoluto(directoryName);
                break;
            case 't':
                ec_null(c,"main");
                tempo= atoi(optarg);
                break;
        }
    }
    for(; optind < argc; optind++){
        //printf("extra arguments: %s\n", argv[optind]);
        insert_list(List_to_insert,argv[optind]);
    }
    return NULL;
}



int main(int argc, char* argv []){

    llist *List_to_insert=NULL;
    llist *List_to_send=NULL;
    // gestione parser
    parser(argc, argv, &List_to_insert);  //list_to_insert contiene i file che erano nella riga di comando

    risultato_da_inviare.lista=NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    sigset_t     mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask,SIGHUP);
    sigaddset(&mask,SIGUSR1);
    sigaddset(&mask, SIGPIPE);

    if (pthread_sigmask(SIG_BLOCK, &mask,NULL) != 0) {
        fprintf(stderr, "FATAL ERROR\n");
        abort();
    }

    // ignoro SIGPIPE per evitare di essere terminato da una scrittura su un socket
    struct sigaction s;
    memset(&s,0,sizeof(s));
    s.sa_handler=SIG_IGN;
    if ( (sigaction(SIGPIPE,&s,NULL) ) == -1 ) {
        perror("sigaction");
        abort();
    }

    pthread_t sighandler_thread;
    if (pthread_create(&sighandler_thread, NULL, sigHandler_func, &mask) != 0) {
        printf("errore nella creazione del signal handler thread\n");
        abort();
    }


///////////////////////////////////////////////////////////////////////////////////
    int p=1,c=numberThreads; //il produttore è uno solo, il masterWorker
    pthread_t    *th;
    threadArgs_t *thARGS;
    pthread_t t1;
    struct sockaddr_un serv_addr;
    int sockfd;
    memset(&serv_addr, '0', sizeof(serv_addr));
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME)+1);
    serv_addr.sun_family = AF_UNIX;

    //Listdir apre tutte le cartelle e inserisce i file nella lista
    listdir(directoryName,List_to_insert);//aggiunge i file nelle directories


    //conto il numero degli argomenti
    int lenght_of_list= listLength(List_to_insert);

    th     = malloc((p+c)*sizeof(pthread_t));
    thARGS = malloc((p+c)*sizeof(threadArgs_t));
    if (!th || !thARGS) {
        fprintf(stderr, "malloc fallita\n");
        exit(EXIT_FAILURE);
    }

    Queue_t *q = initQueue();
    if (!q) {
        fprintf(stderr, "initQueue fallita\n");
        exit(errno);
    }


    //argomenti per produtttore
    for(int i=0;i<p; ++i) {
        thARGS[i].thid = i;
        thARGS[i].q    = q;
        thARGS[i].l = List_to_insert;
        thARGS[i].lenght_tail_list=lenght_of_list;
        thARGS[i].serv_addr = &serv_addr;
        thARGS[i].tempo_di_invio=tempo;
    }

    //argomenti per consumatore
    for(int i=p;i<(p+c); ++i) {
        thARGS[i].thid = i-p;
        thARGS[i].q    = q;
        thARGS[i].lenght_tail_list=lenght_of_list;
        thARGS[i].serv_addr = &serv_addr;
        thARGS[i].l = List_to_send;
    }

    pid_t process_id = fork(); //creo collector, processo figlio del masterWorker
    if(process_id == -1){
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (process_id == 0){ //processo figlio = collector

        //ignoro tutti i segnali gestiti dal MasterWorker
        struct sigaction s;
        memset(&s, 0, sizeof(s)); //azzero s
        s.sa_handler = SIG_IGN; //ignoro tutti i isegnali
        s.sa_flags = SA_RESTART; // per non interrompere le sistem call

        sigaction(SIGINT,&s,NULL);
        sigaction(SIGQUIT,&s,NULL);
        sigaction(SIGTERM,&s,NULL);
        sigaction(SIGHUP,&s,NULL);
        sigaction(SIGUSR1,&s,NULL);
        sigaction(SIGPIPE,&s,NULL);


        //socket del collector
        for(int i=0;i<p; ++i){
            if(pthread_create(&t1,NULL,socket_collector,&thARGS[i])!= 0){
                fprintf(stderr, "pthread_create failed (Consumer)\n");
                exit(EXIT_FAILURE);
            }
        }

      //  printf("fine collector nel main \n\n");

        pthread_join(t1,NULL);//collector

        //libero memoria usata
        linked_list_destroy(List_to_insert);
        deleteQueue(q);
        free(th);
        free(thARGS);
    }
    else{ //processo padre = masterWorker

        for(int i=0;i<c; ++i)
            if (pthread_create(&th[p+i], NULL, Consumer, &thARGS[p+i]) != 0) {
                fprintf(stderr, "pthread_create failed (Consumer)\n");
                exit(EXIT_FAILURE);
            }

        for(int i=0;i<p; ++i)
            if (pthread_create(&th[i], NULL, Producer, &thARGS[i]) != 0) {
                fprintf(stderr, "pthread_create failed (Producer)\n");
                exit(EXIT_FAILURE);
            }

        // aspetto prima tutti i produttori
        for(int i=0;i<p; ++i)
            pthread_join(th[i], NULL);

        // quindi termino tutti i consumatori/workers
        for(int i=0;i<c; ++i) {
            push(q, "fine");
        }

        // aspetto la terminazione di tutti i consumatori
        for(int i=0;i<c; ++i){
            pthread_join(th[p+i], NULL);
        }

        //print_list(risultato_da_inviare.lista);

        SYSCALL_EXIT("socket", sockfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket","");
        int notused;
        SYSCALL_EXIT("connect", notused, connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)), "connect","");

       //invio il numero di file che arriveranno al collector
        int lungh_lista=listLength(risultato_da_inviare.lista);

        //trasformiamo int in char
        char* charValue= malloc(sizeof (char));
        sprintf(charValue,"%d",lungh_lista);

        //inviamo la lunghezza della lista al collector
        write(sockfd,charValue, strlen(charValue)+1);
        sleep(1);


        //invio tutti i file al collector
        for(int indice=0; indice<lungh_lista;indice++){
            write(sockfd,risultato_da_inviare.lista->opzione, strlen(risultato_da_inviare.lista->opzione)+1);
           // printf("FILE SEND: %s\n",risultato_da_inviare.lista->opzione);
            sleep(1);
            risultato_da_inviare.lista=risultato_da_inviare.lista->next;
        }

        close(sockfd);

        //libero memoria usata
        free(charValue);
        deleteQueue(q);
        free(th);
        free(thARGS);
    }


    pthread_kill(sighandler_thread,SIGTERM);
    pthread_join(sighandler_thread,NULL);

    linked_list_destroy(risultato_da_inviare.lista);

   // printf("fine main\n\n");
    return 0;
}