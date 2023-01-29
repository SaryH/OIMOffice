#include "local.h"

int test;

int citizens_no,tellers_no,
birth_tellers,id_tellers,travel_tellers,reunion_tellers,
max_unserved,max_unhappy,max_satisfied,metal_gates_no;
int current_hour = 5, current_minute = 0, time_unit = 15;
int num_not_arrived, arrive_per_time_unit, num_new_citizens;

int tellers[4];
int opengl;

int simulation=1;

key_t key_msg;

int msgid_m, msgid_f, msgid_t;
int *array_pids;
int pid_array_pointer=0;

void * shmptr;
int shmid;


void alarm_catcher(int the_sig);

//read user data from file
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
	printf("%d metal gates no\n",metal_gates_no);
	fflush(stdout);

}
//_______________________________________________________SHMEM_____________________________________
//create shared memory
int create_shmem(){
	int key,shmid;
	if((key=ftok(".",SEED_SHM))==-1){
		perror("Error in public key generation of shmem");
		exit(-1);
	}
	if((shmid=shmget(key,SHMEM_SIZE,IPC_CREAT | 0644))==-1){//ONLY OWNER CAN WRITE NOW
		perror("Error opening shared memory");
		exit(-1);
	}
	return shmid;
}

//attach the shared memory
void * shmem_attach(int shmid){
	void * shmptr;
	if((shmptr=shmat(shmid,0,0))==(int *)-1){
		perror("Error attaching shared memory to address");
		exit(-2);
	}
	return shmptr;
}




int * attach_shared_resources(int * shmptr){

	shmptr[HR_INDEX]=current_hour;
	shmptr[MN_INDEX]=current_minute;
	shmptr[GATE_INDEX]=4;
	shmptr[UNSERVED]=0;
	shmptr[SATISFIED]=0;
	shmptr[UNHAPPY]=0;
	
	shmptr[GAT_INDEX]=0;
	shmptr[MQU_INDEX]=0;
	shmptr[FQU_INDEX]=0;
	shmptr[MET_INDEX]=0;
	shmptr[TEL_INDEX]=0;
	

	return shmptr;

}


//________________________________________________SEMAPHORE____________________________________

int create_semaphore(union semun arg){

	int key,semid;

	if((key=ftok(".",SEED_SEM))==-1){
		perror("Error in semaphore key generation");
		exit(-1);
	}
	if((semid=semget(key,metal_gates_no,IPC_CREAT | 0666))==-1){
		perror("Error in semaphore creation in parent");
		exit(-2);
	}
	if(semctl(semid,0,SETALL,arg)==-1){
		perror("Error in semctl in parent");
		exit(-3);
	}

	return semid;

}


//_______________________________________________MESSAEGE QUEUE__________________________________

int create_msg_queue(char gender){
	int msgid;
	int queue_seed = SEED_MSQ_F;
	if (gender == 'M') queue_seed = SEED_MSQ_M;
	if(gender=='T') queue_seed=SEED_MSQ_T;

       // Define a message queue descriptor structure
    struct msqid_ds queue_descriptor;

    // Generate a key for the message queue using ftok()
    key_msg = ftok(".", queue_seed);
    if (key_msg == -1) {
        perror("ftok failed in delete msg queue");
        exit(EXIT_FAILURE);
    }

	msgid = msgget(key_msg, 0);
    if (msgctl(msgid, IPC_STAT, &queue_descriptor) != -1) {
          // Delete the message queue using msgctl() and the IPC_RMID command
    	if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl failed from delete");
    	}
    }
    printf("Message queue deleted\n");
    fflush(stdout);

    // Create the message queue using msgget()
    msgid = msgget(key_msg, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget failed in create");
        exit(EXIT_FAILURE);
    }


	//MAKE NONEBLOCKING
	int flags = fcntl(msgid, F_GETFL, 0);
    if (flags == -1) {
        // handle error
    }

    flags |= O_NONBLOCK;
    if (fcntl(msgid, F_SETFL, flags) == -1) {
        // handle error
    }

    printf("NEW QUEUE CREATED\n");
    fflush(stdout);



	return msgid;

}







void recieve_msg(int msgid){
/*
    // Generate a key for the message queue using ftok()
    key = ftok(".", 1);
    if (key == -1) {
        perror("ftok failed in recieve msg");
        exit(EXIT_FAILURE);
    }

    // Create the message queue using msgget()
    msgid = msgget(key, 0666);
    if (msgid == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }
    */

	Message_queue msg;
	    // Define a message queue descriptor structure
    struct msqid_ds queue_descriptor;

	 if (msgctl(msgid, IPC_STAT, &queue_descriptor) == -1) {
        perror("msgctl failed in descriptor ");
        exit(EXIT_FAILURE);
    }

    // Print the number of messages in the queue
    printf("Number of messages in queue: %d\n", queue_descriptor.msg_qnum);


 // Receive a message using msgrcv()
    if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
        perror("msgrcv failed in recieve");
        exit(EXIT_FAILURE);
    }

    // Print the message text
    printf("Received message: %s\n", msg.mtext);

     if (msgctl(msgid, IPC_STAT, &queue_descriptor) == -1) {
        perror("msgctl failed in recieve");
        exit(EXIT_FAILURE);
    }

    // Print the number of messages in the queue
    printf("Number of messages in queue: %d\n", queue_descriptor.msg_qnum);

    }



    void fork_tellers(){
	//forking tellers
	tellers[0]=fork();
	switch(tellers[0]){
		case -1://failed to fork teller
			exit(-1);
			break;

		case 0://currently in child
			execlp("./teller","teller","B",(char *)NULL);
			break;
		default:
			printf("\nPARENT FORKED B Teller");
			break;
	}

	tellers[1]=fork();
	switch(tellers[1]){
		case -1://failed to fork teller
			exit(-1);
			break;

		case 0://currently in child
			execlp("./teller","teller","T",(char *)NULL);
			break;
		default:
			printf("\nPARENT FORKED T Teller");
			break;
	}

	tellers[2]=fork();
	switch(tellers[2]){
		case -1://failed to fork teller
			exit(-1);
			break;

		case 0://currently in child
			execlp("./teller","teller","I",(char *)NULL);
			break;
		default:
			printf("\nPARENT FORKED I Teller");
			break;
	}

	tellers[3]=fork();
	switch(tellers[3]){
		case -1://failed to fork teller
			exit(-1);
			break;

		case 0://currently in child
			execlp("./teller","teller","R",(char *)NULL);
			break;
		default:
			printf("\nPARENT FORKED R Teller");
			break;
	}
	
		opengl = fork();
	switch(opengl){
		case -1://failed to fork teller
			exit(-1);
			break;

		case 0://currently in child
			execlp("./opengl","opengl",(char *)NULL);
			break;
		default:
			printf("\nPARENT FORKED OPENGL");
			break;
	}
}




void fork_citizen(){

    pid_t pid;
    pid = fork();
    if (pid == 0) {
        // This is the child process
        char *new_argv[] = {"./citizen", NULL};
        execv("./citizen", new_argv);

        // If execv() returns, it means an error occurred
        perror("execv failed");
        exit(EXIT_FAILURE);
    }else if(pid>0){
    //parent
    array_pids[pid_array_pointer] = pid;
    pid_array_pointer++;
    }else if(pid<0) {
        // fork() failed
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}




//_________________________________________MAIN_________________________________________-


int main(){

	read_file();
	int semid;
	union semun arg_sem;
	arg_sem.array=(unsigned short[]){1,1,metal_gates_no,1,1,1,1,1,1};//0 1 2 3

	msgid_m = create_msg_queue('M');
	msgid_f = create_msg_queue('F');
	msgid_t = create_msg_queue('T');

	shmid=create_shmem();
	shmptr=(int*)shmem_attach(shmid);
	shmptr=attach_shared_resources(shmptr);

	semid=create_semaphore(arg_sem);

	sleep(1);

	int gate_officer=fork();
	switch(gate_officer){
		case -1://failed to fork officer
			exit(-1);
			break;

		case 0://currently in child
			execlp("./officer",NULL);
			break;
		default:
			sleep(1);
			printf("PARENT FORKED OFFICER\n");
			break;
	}


	fork_tellers();
	printf("total citizen number: %d\n",citizens_no);
	fflush(stdout);


    array_pids = malloc(citizens_no * sizeof(int));
    if (array_pids == NULL) {
        printf("ARRAYPIDS null");
    }


	num_not_arrived = citizens_no;
	srand(time(NULL));
	arrive_per_time_unit = citizens_no/(8*(60/time_unit));
	printf("arrive_per_time_unit: %d\n", arrive_per_time_unit);


	  if ( sigset(SIGALRM, alarm_catcher) == SIG_ERR ) {
	    perror("Sigset can not set SIGALRM");
	    exit(SIGINT);
	  }


	fflush(stdout);
	alarm(1);

	while(1){
		pause();
	}
	    free(array_pids);

	return 0;
}







void alarm_catcher(int the_sig){

int * shmptr_temp=shmptr;

//UPDATE TIME
  current_minute += time_unit;
  if(current_minute >= 60) {
  	current_minute %= 60;
	current_hour++;
  }

    printf(BLUE"hour: %d, minute: %d\n"RESET,current_hour, current_minute);
    shmptr_temp[HR_INDEX] = current_hour;
    shmptr_temp[MN_INDEX] = current_minute;
    //kill(-1 , SIGCONT);


//NEW CITIZEN ARRIVAL
  if(num_not_arrived > 0 && current_hour >=5 && current_hour <=13){
  	printf("%d people left unserved so far!\n",shmptr_temp[UNSERVED]);
  	printf("%d people left unhappy so far!\n",shmptr_temp[UNHAPPY]);
  	printf("%d people left satisfied so far!\n",shmptr_temp[SATISFIED]);
  	
	  num_new_citizens=1;

	  if(arrive_per_time_unit > 0)
	  	num_new_citizens = rand()%arrive_per_time_unit + arrive_per_time_unit/2 + 1;

	  if(num_new_citizens > num_not_arrived || current_hour == 13)
	  	num_new_citizens = num_not_arrived;

	  num_not_arrived -= num_new_citizens;
	  printf("new: %d remaining: %d\n\n", num_new_citizens, num_not_arrived);
		fflush(stdout);
	for(int i=0; i< num_new_citizens;i++)
		fork_citizen();
	}
	if(shmptr_temp[UNSERVED]>=max_unserved){
  		printf(CYAN"MAXIMUM UNSERVED PEOPLE REACHED!\n");
  		fflush(stdout);
  		kill(opengl,SIGKILL);
  		kill(getpid(),SIGKILL);
  		simulation=0;
  	}else if(shmptr_temp[UNHAPPY]>=max_unhappy){
  		printf(CYAN"MAXIMUM UNHAPPY PEOPLE REACHED!\n");
  		fflush(stdout);
  		kill(opengl,SIGKILL);
  		kill(getpid(),SIGKILL);
  		simulation=0;
  	}else if(shmptr_temp[SATISFIED]>=max_satisfied){
  		printf(CYAN"MAXIMUM SATISFIED PEOPLE REACHED!\n");
  		fflush(stdout);
  		kill(opengl,SIGKILL);
  		kill(getpid(),SIGKILL);
  		simulation=0;
  	}



//SIGCONT
/*
if(current_hour == 6 && current_minute == 0){
    int result = kill(array_pids[0] , SIGCONT);
    if (result == 0) printf("___SIGCONT sent %d\n",array_pids[0]);

    result = kill(array_pids[1] , SIGCONT);
    if (result == 0) printf("___SIGCONT sent %d\n",array_pids[1]);
}

*/
/*
printf("male");
recieve_msg(msgid_m);
printf("female");
recieve_msg(msgid_f);
*/

	if(simulation)
		alarm(1);
}


