#include "local.h"
    key_t key_msg;
    int msgid;
    Citizen citizen;

//read file
int citizens_no,tellers_no,
birth_tellers,id_tellers,travel_tellers,reunion_tellers,
max_unserved,max_unhappy,max_satisfied,metal_gates_no;

int * shmptr_temp;
int semid;

//read file
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


//____________________________________________________MESSAGE QUEUE____________________________________
void get_msgid(char gender){
	int queue_seed = SEED_MSQ_F;
	if (gender == 'M') queue_seed = SEED_MSQ_M;
    if(gender=='T') queue_seed=SEED_MSQ_T;

    // Generate a key for the message queue using ftok()
    key_msg = ftok(".", queue_seed);
    if (key_msg == -1) {
        perror("ftok failed in citizen");
        exit(EXIT_FAILURE);
    }

        // Create the message queue using msgget()
    msgid = msgget(key_msg, 0666 );
    if (msgid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }
}



void enter_queue(){
Message_queue msg;

    // Set the message type
    msg.mtype = 1;

    // Set the message text
    sprintf(msg.mtext, "%d", citizen.pid);


    // Send the message using msgsnd()
    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }
    printf("Citizen %d entered queue\n",getpid());
	fflush(stdout);
}


void enter_ticket_queue(){
Message_queue msg;

    // Set the message type
    if(citizen.ticket=='B'){
    	msg.mtype = 0;
    }else if(citizen.ticket=='T'){
    	msg.mtype = 1;
    }else if(citizen.ticket=='R'){
    	msg.mtype = 2;
    }else if(citizen.ticket=='I'){
    	msg.mtype = 3;
    }
    

    // Set the message text
    sprintf(msg.mtext, "%d", citizen.pid);
    msg.ticket=citizen.ticket;


    // Send the message using msgsnd()
    if (msgsnd(msgid, &msg, sizeof(msg), 0) == -1) {//questionable? maybe
        perror("msgsnd failed");
        exit(EXIT_FAILURE);
    }
    printf("\nCitizen %d went to the inner waiting room (TICKET:%c)",getpid(),citizen.ticket);
	fflush(stdout);
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



void check_patience(int id){
	printf(YELLOW"\n%d SICK OF WAITING! BYE SHURUP"RESET,getpid());
	fflush(stdout);
	/*acquire.sem_num=UNSERVED;
	if(semop(semid,&acquire,1)==-1){//officer acquires one of the free semaphores (checks if gate is free)
		perror("Error in acquiring sem in officer");
		exit(-10);
	}*/
	shmptr_temp[UNSERVED]+=1;
	/*release.sem_num=UNSERVED;//citizen releases the semaphore after completing its critical section
	if(semop(semid,&release,1) == -1) {
		perror("error in releasing sem in officer");
		exit(-12);
    	}*/
	
	exit(0);
}

void set_alarm(){
	if (sigset(SIGALRM, check_patience) == -1) {
		perror("Cannot set SIGALARM");
		exit(SIGALRM);
	}
}


void wait_time_units(int num_time_units){
	sleep(num_time_units);
	//printf(GREEN"FINISHED MY %d PAUSE %d\n"RESET, num_time_units, getpid());
}


//_______________________________________________________MAIN___________________________________
int main(int argc, char *argv[])
{

	//GET ARRIVAL TIME
	int shmid=open_shmem();
	void * shmptr=(int*)shmem_attach(shmid);
	shmptr_temp=shmptr;

	shmptr_temp[GAT_INDEX]++;

	
    set_alarm();
    read_file();

	semid=open_semaphore();

	citizen.pid = getpid();
	srand(getpid());
	citizen.gender = rand()%2 == 0? 'M':'F';

	int request=rand()%4;
	if(request==0){
		citizen.ticket='B';
	}else if(request==1){
		citizen.ticket='T';
	}else if(request==2){
		citizen.ticket='R';
	}else{
		citizen.ticket='I';
	}

	//MAX WAIT FOR PASSENGER (PATIENCE)
	citizen.max_wait=rand()%CITIZEN_MAX_WAIT+CITIZEN_MIN_WAIT;


	citizen.arrival_hour = shmptr_temp[HR_INDEX];
	citizen.arrival_minute = shmptr_temp[MN_INDEX];
	
	printf("CITIZEN CREATED gender: %c  and %d, knows that time is %d:%d\n",citizen.gender, getpid(),  citizen.arrival_hour, citizen.arrival_minute);
	fflush(stdout);
	
	
	if(citizen.arrival_hour<8)
	wait_time_units((8-citizen.arrival_hour)*4);


 	//CHECK MESSAGE QUEUE AVAILABILITY
 	int index = AQF_INDEX;
 	if(citizen.gender == 'M') index = AQM_INDEX;
 	int allow_entry = shmptr_temp[index];


    // ENTER MESSAGE QUEUE
    while(allow_entry == 0){ 
    	//printf(GREEN"waiting for allow entry %d \n"RESET,getpid()); fflush(stdout);
    	wait_time_units(rand()%2+1);
    	allow_entry = shmptr_temp[index];
    }
    
	get_msgid(citizen.gender);
	enter_queue();
	shmptr_temp[GAT_INDEX]--;
	if(citizen.gender == 'M')
	shmptr_temp[MQU_INDEX]++;
	else	
	shmptr_temp[FQU_INDEX]++;



	raise(SIGSTOP);//wait for officer continue signal
	
	if(citizen.gender == 'M')
	shmptr_temp[MQU_INDEX]--;
	else	
	shmptr_temp[FQU_INDEX]--;
	
	shmptr_temp[MET_INDEX]++;
	

    int gate_time=rand()%4+1;
	printf(MAGENTA"I (%d) went into the metal detector! and I need %d seconds\n"RESET,getpid(),gate_time);
	fflush(stdout);
	
	//alarm(citizen.max_wait+gate_time);
	
	sleep(gate_time);
	
	alarm(citizen.max_wait);
	
	//shmptr_temp[GATE_INDEX]+=1;
	
	release.sem_num=GATE_INDEX;//citizen releases the semaphore after completing its critical section
	if(semop(semid,&release,1) == -1) {
		perror("error in releasing sem in officer");
		exit(-12);
    	}
    	
	shmptr_temp[MET_INDEX]--;
	shmptr_temp[TEL_INDEX]++;
	
	printf(MAGENTA"I (%d) EXITED METAL DETECTOR, my patience has a limit: %d\n"RESET,getpid(),gate_time,citizen.max_wait);
	fflush(stdout);

    	get_msgid('T');
    	enter_ticket_queue();
    	
    	while(1);

}

