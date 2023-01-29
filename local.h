#ifndef __LOCAL_H_
#define __LOCAL_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <wait.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <sys/msg.h>
#include <fcntl.h>

#define SEED_MSQ_M 'm'
#define SEED_MSQ_F 'f'

#define SEED_MSQ_T 't'
/*#define SEED_MSQ_T 't'
#define SEED_MSQ_R 'r'
#define SEED_MSQ_I 'i'*/

#define SEED_SHM 'z'
#define SEED_SEM 'q'

#define SEEDsem 's'
#define SEEDsem2 'h'

#define MAX_BUF 50 // read file string
#define INPUT_LINES 10



#define SHMEM_SIZE 1024

#define CITIZEN_MAX_WAIT 7
#define CITIZEN_MIN_WAIT 10
#define TELLER_TIME 4





//_______________________________________________________________________________________

#define HR_INDEX       0
#define MN_INDEX       1

#define UNSERVED	7
#define UNHAPPY		8
#define SATISFIED	9
#define AQM_INDEX       11 //allow queue
#define AQF_INDEX       10 //allow queue

#define GATE_INDEX	2 // metal gate index
#define BT_INDEX	3
#define TT_INDEX	4
#define RT_INDEX	5
#define IT_INDEX	6

#define GAT_INDEX	12
#define MQU_INDEX	13
#define FQU_INDEX	14
#define MET_INDEX	15
#define TEL_INDEX	16

    // Define a message buffer
	typedef struct Message_queue {
        long mtype;
        char mtext[10];
         char ticket;
    }Message_queue;


    typedef struct Citizen {
    int pid;
    char gender;
    //bool valid; // passport check
    int max_wait; // impatient time
    int arrival_hour; // initalized in arraival
    int arrival_minute; // initalized in arraival
    char ticket;
	} Citizen;
//________________________________________________________________________________________



union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

struct sembuf acquire = {0, -1, SEM_UNDO},
        release = {0, 1, SEM_UNDO};


typedef struct shmem {
    char buffer[MAX_BUF][10];
    int head, tail;
} MEMORY;




#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"



#endif

