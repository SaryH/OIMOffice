#include "local.h"

int msgid_t;
key_t msg_key_t;
char ticket_type;
int ticket;
int shmid;
void *shmptr;
int *shmptr_temp;

int open_shmem(){

	int key,shmid;

	if((key=ftok(".",SEED_SHM))==-1){
		perror("Error in shmem key");
		exit(-1);
	}

	if((shmid=shmget(key,0,0))==-1){
		perror("Error opening shared memory from citizen");
		exit(-2);
	}
	return shmid;
}


void * shmem_attach(int shmid){

	void * shmptr;
	if((shmptr=shmat(shmid,0,0))==(void*)-1){
		perror("Error attaching shmem to address");
		exit(-3);
	}
	return shmptr;

}

void receive_msg(){
	Message_queue msg_t;

	struct msqid_ds queue_desc_t;

	
	if((msg_key_t=ftok(".",SEED_MSQ_T))==-1){
		perror("Message queue key generation failed in teller");
		exit(-1);
	}
	
	if((msgid_t=msgget(msg_key_t,0))==-1){
		perror("Cannot get the msg queue id in teller");
		exit(-2);
	}
	
	
	if(msgctl(msgid_t,IPC_STAT,&queue_desc_t)==-1){
		perror("Message ctl in teller failed");
		exit(-3);
	}
	
	int people_in_q=queue_desc_t.msg_qnum;
	
	printf("\nTeller %c see %d people in queue",ticket_type,people_in_q);
	fflush(stdout);
	
	
	if(msgrcv(msgid_t,&msg_t,sizeof(msg_t),ticket,0)==-1){
		perror("Message rcv failed in teller");
		exit(-4);
	}
	srand(getpid());
	int passed=rand()%100;
	if(passed>70){
		printf(GREEN"\nTeller %c received ticket of type:%c and granted it!"RESET,ticket_type,msg_t.ticket);
		shmptr_temp[SATISFIED]+=1;
		
	}else{
		printf(RED"\nTeller %c received ticket of type:%c and DENIED it!"RESET,ticket_type,msg_t.ticket);
		shmptr_temp[UNHAPPY]+=1;
		
	}
	fflush(stdout);
	
	
}

int main(int argc, char **argv){

	shmid=open_shmem();
	shmptr=shmem_attach(shmid);
	shmptr_temp=shmptr;

	ticket_type=argv[1][0];
	if(ticket_type=='B'){
    		ticket = 0;
    	}else if(ticket_type=='T'){
    		ticket = 1;
    	}else if(ticket_type=='R'){
    		ticket = 2;
    	}else if(ticket_type=='I'){
    		ticket = 3;
    	}
	
	printf("\nHELLO I AM %c TELLER",ticket_type);
	fflush(stdout);
	
	while(1){
	
		receive_msg();
		sleep(TELLER_TIME);
		if(shmptr_temp[TEL_INDEX]>0)shmptr_temp[TEL_INDEX]--;
	
	}
	return 0;
	
}
