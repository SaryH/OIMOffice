#include "local.h"

key_t msg_key_m,msg_key_f;
int msgid_m;
int msgid_f;
int allow_entry_m = 1;
int allow_entry_f = 1;
int * shmptr_temp;
int semid;

//read file
int citizens_no,tellers_no,
birth_tellers,id_tellers,travel_tellers,reunion_tellers,
max_unserved,max_unhappy,max_satisfied,metal_gates_no;

void read_file(){
	char buffer[INPUT_LINES][MAX_BUF];

	FILE *data;
	char filename[MAX_BUF]="data.txt";
	data=fopen(filename,"r");
	if(data==NULL){
		perror("Error opening input file");
		exit(-1);
	}

	for(int i=0;i<INPUT_LINES;i++){
		while(fgets(buffer[i],sizeof(buffer[i]),data)!=NULL){
			i++;
		}
	}

	//parse values to variables:
	int stats[INPUT_LINES];
	for(int i=0;i<INPUT_LINES;i++){
		char *token;
		token=strtok(buffer[i]," ");
		token=strtok(NULL," ");
		stats[i]=atoi(token);
	}

	citizens_no=stats[0];
	tellers_no=stats[1];
	birth_tellers=stats[2];
	travel_tellers=stats[3];
	id_tellers=stats[4];
	reunion_tellers=stats[5];
	max_unserved=stats[6];
	max_unhappy=stats[7];
	max_satisfied=stats[8];
	metal_gates_no=stats[9];

}

//__________________________________________________SHMEM________________________________________
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

int open_semaphore(){

    int key,semid;

    if ((key = ftok(".", SEED_SEM)) == -1) {
        perror("error in sempahore key gen");
        exit(-1);
    }
    if ((semid = semget(key,metal_gates_no,0)) == -1) {
        perror("error in getting semaphore id");
        exit(-6);
    }

    return semid;
}

void receive_msg(){
	Message_queue msg_m;
	Message_queue msg_f;
	struct msqid_ds queue_desc_m;
	struct msqid_ds queue_desc_f;



	if(msgctl(msgid_m,IPC_STAT,&queue_desc_m)==-1){
		perror("Message ctl in officer failed");
		exit(-3);
	}
	if(msgctl(msgid_f,IPC_STAT,&queue_desc_f)==-1){
		perror("Message ctl in officer failed");
		exit(-3);
	}

	int males_in_q=queue_desc_m.msg_qnum;
	int females_in_q=queue_desc_f.msg_qnum;

	printf(RED"Officer sees %d males and %d females in queue\n"RESET,males_in_q, females_in_q);
	fflush(stdout);

	if(males_in_q==0 && females_in_q > 0){
		if(msgrcv(msgid_f,&msg_f,sizeof(msg_f.mtext),1,0)==-1){
			perror("Message rcv failed in officer");
			exit(-4);
		}
		printf(GREEN"Female taken from female queue with ID:%s\n"RESET,msg_f.mtext);
		fflush(stdout);
		kill(atoi(msg_f.mtext),SIGCONT);
		
	}else if(females_in_q==0 && males_in_q > 0){
		if(msgrcv(msgid_m,&msg_m,sizeof(msg_m.mtext),1,0)==-1){
			perror("Message rcv failed in officer");
			exit(-4);
		}
		printf(GREEN"Male taken from male queue with ID:%s\n"RESET,msg_m.mtext);
		fflush(stdout);
		kill(atoi(msg_m.mtext),SIGCONT);
		
	}else if(females_in_q==0 && males_in_q == 0){
	printf(GREEN"NO FEMALES OR MALES IN QUEUE:\n"RESET);
	fflush(stdout);
	release.sem_num=GATE_INDEX;//citizen releases the semaphore after completing its critical section
	if(semop(semid,&release,1) == -1) {
		perror("error in releasing sem in officer");
		exit(-12);
    	}
	
	}else{
		srand(time(0));
		int choice=rand()%2;
		if(choice==0){
			if(msgrcv(msgid_m,&msg_m,sizeof(msg_m.mtext),1,0)==-1){
				perror("Message rcv failed in officer");
				exit(-4);
			}
			printf("Male taken from male queue with ID:%s\n",msg_m.mtext);
			fflush(stdout);
			kill(atoi(msg_m.mtext),SIGCONT);
		}else{
			if(msgrcv(msgid_f,&msg_f,sizeof(msg_f.mtext),1,0)==-1){
			perror("Message rcv failed in officer");
			exit(-4);
			}
			printf("Female taken from female queue with ID:%s\n",msg_f.mtext);
			fflush(stdout);
			kill(atoi(msg_f.mtext),SIGCONT);
		}
	}





	if (males_in_q >= 30){
	shmptr_temp[AQM_INDEX] = 0;
	printf(GREEN"***_____Malefull\n"RESET);
	fflush(stdout);
	}
	if (females_in_q >= 30){
	shmptr_temp[AQF_INDEX] = 0;
	printf(GREEN"***_____FeMalefull\n"RESET);
	fflush(stdout);
	}



	if (males_in_q <= 15)
	shmptr_temp[AQM_INDEX] = 1;

	if (females_in_q <= 15)
	shmptr_temp[AQF_INDEX] = 1;

	/*if(msgrcv(msgid,&msg,sizeof(msg.mtext),1,0)==-1){
		perror("Message rcv failed in officer");
		exit(-4);
	}*/
}

int main(){
	read_file();

	if((msg_key_m=ftok(".",SEED_MSQ_M))==-1){
		perror("Message queue key generation failed in officer");
		exit(-1);
	}

	if((msgid_m=msgget(msg_key_m,0))==-1){
		perror("Cannot get the msg queue id in soldier");
		exit(-2);
	}

	if((msg_key_f=ftok(".",SEED_MSQ_F))==-1){
	perror("Message queue key generation failed in officer");
	exit(-1);
	}

	if((msgid_f=msgget(msg_key_f,0))==-1){
		perror("Cannot get the msg queue id in soldier");
		exit(-2);
	}

    int shmid=open_shmem();
	void * shmptr=(int*)shmem_attach(shmid);
	shmptr_temp=shmptr;
	shmptr_temp[AQM_INDEX] = 1;
	shmptr_temp[AQF_INDEX] = 1;

	
	semid=open_semaphore();


	while(1){
		printf(GREEN"***____FROM OFFICER%d\n"RESET,metal_gates_no);
		fflush(stdout);
		for(int i=0;i<metal_gates_no;i++){
			acquire.sem_num=GATE_INDEX;
			if(semop(semid,&acquire,1)==-1){//officer acquires one of the free semaphores (checks if gate is free)
				perror("Error in acquiring sem in officer");
				exit(-10);
				}
			receive_msg();//officer picks random citizen from male or female queue
		}
		usleep(800000);

	}
	return 0;

}

