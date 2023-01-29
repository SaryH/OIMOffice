#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <GL/glut.h>
#include "local.h"

//struct message msg;
//static char buffer[B_SIZ];
int WINDOW_HEIGHT = 1000 ;
int WINDOW_WIDTH = 1000 ;
int updateTime = 500; //microseconds


int * shmptr_temp;
//int n,myfifo;
//static char buffer[B_SIZ];
char parentId[20];
//int parentFifo;

typedef struct point {
	int x;
	int y;
} point;

point points[5];	    //A0 - A5
point displacement[2][5]; //xp yp team 1 and 2

int order[2];

int moves[2];

int change_x[2];// (x_end - x_start)/speed;
int change_y[2]; //(y_end - y_start)/speed;

// start & end = point[]
int x_start = 500;
int y_start = 500;
int x_end = 650;
int y_end = 200;


ushort visitors[10][6]; // ( id | x_start | y_start | x_end | y_end | moving? | color )


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


void setupScene(int clearColor[]) {
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
    //glClearColor(250, 250, 250, 1.0);  //  Set the cleared screen colour to black.
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);  // This sets up the viewport so that the coordinates (0, 0) are at the top left of the window.
    
    // Set up the orthographic projection so that coordinates (0, 0) are in the top left.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -10, 10);
    
    // Back to the modelview so we can draw stuff.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the screen and depth buffer.
}

    
void initVariables(){
	//creating fifo for children to contact through
	/*
	sprintf(msg.fifo_name, "%d", getpid());
	if (mknod(msg.fifo_name, S_IFIFO | 0666, 0) < 0){
        perror("Private fifo creation");
        exit(-8);
    	}
    	myfifo=open(msg.fifo_name, O_RDONLY | O_NONBLOCK);
    	*/
    	
    //OPEN SHMEM
	int shmid=open_shmem();
	void * shmptr=(int*)shmem_attach(shmid);
	shmptr_temp=shmptr;

	
	points[0].x = 200;
	points[0].y = 500; 
	points[1].x = 500;
	points[1].y = 200; 
	points[2].x = 800;
	points[2].y = 500; 
	points[3].x = 700;
	points[3].y = 800; 
	points[4].x = 300;
	points[4].y = 800; 
    
    	for(int i=0; i<5; i++){
    		displacement[0][i].y = displacement[1][i].y =0;
    		displacement[0][i].x = displacement[1][i].x =0;
    	}
    	
        glPointSize(20);
}

void drawRaceGround(){

    glBegin(GL_QUADS);
    glColor3ub(180, 180, 180);
    glVertex2d(150, 500);
    glVertex2d(150, 750);
    glVertex2d(400, 750);
    glVertex2d(400, 500);
    glEnd();
    
    glBegin(GL_QUADS);
    glColor3ub(100, 80, 80);
    glVertex2d(400, 500);
    glVertex2d(400, 600);
    glVertex2d(600, 600);
    glVertex2d(600, 500);
    glEnd();
    
    glBegin(GL_QUADS);
    glColor3ub(100, 80, 80);
    glVertex2d(400, 650);
    glVertex2d(400, 750);
    glVertex2d(600, 750);
    glVertex2d(600, 650);
    glEnd();
    
    glBegin(GL_QUADS);
    glColor3ub(80, 140, 80);
    glVertex2d(600, 450);
    glVertex2d(600, 800);
    glVertex2d(800, 800);
    glVertex2d(800, 450);
    glEnd();
    
    glBegin(GL_QUADS);
    glColor3ub(80, 80, 100);
    glVertex2d(500, 150);
    glVertex2d(500, 450);
    glVertex2d(900, 450);
    glVertex2d(900, 150);
    glEnd();
    
    glBegin(GL_POINTS);
    glColor3ub(200, 0, 0);
    glVertex2d(750, 625);
    glEnd();
    
    
    int x_axis;
    int y_axis;
    srand(time(0));
    
    
    //citizen.arrival_hour = shmptr_temp[HR_INDEX];
	//citizen.arrival_minute = shmptr_temp[MN_INDEX];
    
    //Random points in Gate area
    for(int i=0; i<shmptr_temp[GAT_INDEX]; i++){
    x_axis = rand()%230 + 160; 
    y_axis = rand()%230 + 510; 
 
    glBegin(GL_POINTS);
    glColor3ub(0, 0, 255);
    glVertex2d(x_axis, y_axis);
    glEnd();
    }
    
    
    //Random points in Male Queue 
    for(int i=0; i<shmptr_temp[MQU_INDEX]; i++){
    x_axis = rand()%180 + 410; 
    y_axis = rand()%80 + 510; 
 
    glBegin(GL_POINTS);
    glColor3ub(0, 0, 255);
    glVertex2d(x_axis, y_axis);
    glEnd();
    }
    
    //Random points in Female Queue
    for(int i=0; i<shmptr_temp[FQU_INDEX];i++){
    x_axis = rand()%180 + 410; 
    y_axis = rand()%80 + 660; 
 
    glBegin(GL_POINTS);
    glColor3ub(0, 0, 255);
    glVertex2d(x_axis, y_axis);
    glEnd();
    }
    
        
    //Random points in Metal Detector
    for(int i=0; i<shmptr_temp[MET_INDEX]; i++){
    x_axis = rand()%180 + 610; 
    y_axis = rand()%50 + 450; 
 
    glBegin(GL_POINTS);
    glColor3ub(0, 0, 255);
    glVertex2d(x_axis, y_axis);
    glEnd();
    }
    
        
    //Random points at Teller
    for(int i=0; i<shmptr_temp[TEL_INDEX]; i++){
    x_axis = rand()%380 + 510; 
    y_axis = rand()%280 + 160; 
 
    glBegin(GL_POINTS);
    glColor3ub(0, 0, 255);
    glVertex2d(x_axis, y_axis);
    glEnd();
    }
    
    
    /*
    
glBegin(GL_QUADS);
    glColor3ub(80, 80, 80);
    glVertex2d();
    glVertex2d();
    glVertex2d();
    glVertex2d();
    glEnd();


    for(int i=0; i<5; i++){
    glBegin(GL_POINTS);
    glColor3ub(0, 0, 255);
    glVertex2d(points[i].x, points[i].y);
    glEnd();
    }
*/
}

void drawScene() {
    setupScene((int[]){250, 250, 250, 1});    
    drawRaceGround();  
/*
    //move team1
    for(int i=0; i<5; i++){
	    glPushMatrix();
	    glTranslatef(displacement[0][i].x, displacement[0][i].y, 0);
	    glBegin(GL_POINTS);
	    glColor3ub(0, 255, 0);
	    glVertex2d(points[i].x, points[i].y);
	    glEnd();
	    glPopMatrix();
     }
     
      //move team2
    for(int i=0; i<5; i++){
	    glPushMatrix();
	    glTranslatef(displacement[1][i].x, displacement[1][i].y, 0);
	    glBegin(GL_POINTS);
	    glColor3ub(255, 0, 0);
	    glVertex2d(points[i].x, points[i].y);
	    glEnd();
	    glPopMatrix();
     }
     
*/	


    glutSwapBuffers();  // Send the scene to the screen.
    glFlush();
}

void update(int value) {
	
	//kill(getpid(),SIGUSR1);
	//reading from fifo for updates
	/*
	if(moves[0] == 0 || moves[1] == 0)
	if((n=read(myfifo,&buffer,B_SIZ))>0){

		//printf("\n\n\t\tOPENGL GOT THE DATA: %s",buffer);
		//fflush(stdout);

		char delim[] = " ";
		char* token = strtok(buffer, delim);
		int currentOrder = atoi(token);
		int team = (currentOrder > 4) ? 1 : 0;
		//printf("____________%d",team);
		currentOrder = currentOrder%5;
		token = strtok(0, delim);
		int timeNeeded =atoi(token);
		
		//printf("\ncurrentOrder = %d, timeNeeded = %d",currentOrder, timeNeeded);
		//fflush(stdout);
		
		x_start = points[currentOrder].x;	  // set start cordinates 
		y_start = points[currentOrder].y;
		x_end = points[((currentOrder) + 1)%5].x;  // set end coordinates at next point
		y_end = points[((currentOrder) + 1)%5].y;
		
		
		displacement[team][currentOrder].x = 0;
		displacement[team][currentOrder].y = 0;

		
		order[team] = currentOrder;
		moves[team] = timeNeeded*1000/updateTime;


		change_x[team] = (x_end - x_start)/moves[team];
		change_y[team] = (y_end - y_start)/moves[team];

	}
	
	
		//printf("_____disp of team %d, is %d",1, displacement[1][order[1]].x);
		
	for(int ofTeam=0; ofTeam<2; ofTeam++){
		if(moves[ofTeam] > 0){  
			moves[ofTeam]--;
			displacement[ofTeam][order[ofTeam]].x += change_x[ofTeam];
			displacement[ofTeam][order[ofTeam]].y += change_y[ofTeam];
			//printf("_____disp of team %d, is %d",ofTeam, displacement[ofTeam][order[ofTeam]].x);
			if(moves[ofTeam] == 0 && order[ofTeam] == 4){
			    	for(int i=0; i<5; i++){
		    			displacement[0][i].y = displacement[1][i].y =0;
		    			displacement[0][i].x = displacement[1][i].x =0;
		    		}
				 for(int i=0; i<2; i++){
				    	moves[i] = 0;
					change_x[i] = 0;
					change_y[i] = 0;
					order[i] = 0; 
				}
			}
		}
	}
	*/

    
    glutPostRedisplay();  // Tell GLUT that the display has changed.
    glutTimerFunc(updateTime, update, 0);  // Tell GLUT to call update again in 25 milliseconds.
}

int main(int argc, char * argv[]){

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); 
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	initVariables();
	//initRendering();
	glutDisplayFunc(drawScene);
	//glutReshapeFunc(resizeWindow);
	//glutKeyboardFunc(myKeyboardFunc);
	//glutMouseFunc(myMouseFunc);
	glutTimerFunc(updateTime, update, 0);
	glutMainLoop();
	//close(myfifo);

	return 0;					// This line is never reached
}

