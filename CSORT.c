/**
 * Author: Daniah Mohammed
 * St# 101145902
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <errno.h>
#include <time.h>
#include <wait.h>
#include <ctype.h>

#define SIZE_OF_ARRAY 7

struct shared_use_st {
	char arr[7];
};

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};


static int set_semvalue(void);
static void del_semvalue(void);
static int semaphore_p(void);
static int semaphore_v(void);
static int sem_id;


int main(){
	pid_t pid;

    //allocating shared memo in every single child or only once? 
    void *shared_memory = (void *)0;
	struct shared_use_st *shared_stuff; 
	int shmid;

    //attaching and init the shared memo
    srand((unsigned int)getpid());    
	
	shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666 | IPC_CREAT);
	
	if (shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	//make shared memo accessable 
	shared_memory = shmat(shmid, (void *)0, 0);
	if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

    //semaphore checking operations
    sem_id = semget((key_t)1234, 1, 0666 | IPC_CREAT);


    if (!set_semvalue())
    {
        fprintf(stderr, "Failed to initialize semaphore\n");
        exit(EXIT_FAILURE);
    }

    if (!semaphore_p())
        exit(EXIT_FAILURE);
    
    
    if (!semaphore_v())
        exit(EXIT_FAILURE);

	shared_stuff = (struct shared_use_st *)shared_memory;
    
    //initializing the matrix M by taking an input from the user
    char arrayOfChars;
    printf("Enter seven distinct letters with space between every letter: (Enter a Letter and press Enter after it)\n");
    char c;
    for (int index = 0; index < SIZE_OF_ARRAY; index++) {
        fflush(stdin);
        scanf(" %c", &c);
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
            fprintf(stderr, "incorrect input\n");
            exit(EXIT_FAILURE);
        }
        shared_stuff->arr[index] = tolower(c);
    }

    //parent should fork all 3 process
    int i=1;

    while (i<4){
        pid = fork();
        if(pid ==0){
            break;
        }
        i++;
    }

    switch (pid)
    {
    case -1: 
        perror("fork failed");
        exit(1);
        break;
    case 0:
        semaphore_p();
        printf("\n-------------------------------------------------------------------------------------\n");
        printf("                      Child %d  process started excuting  \n\n", i);
        int o = 0;
        /* The issue in my code should be found here, the while loop should check if the array is totally sotred but I ran out of time to get my code functioning properly*/
        while (o<SIZE_OF_ARRAY)
        {
           for(int c=i+2; c>0; c--){
                if (shared_stuff->arr[c + 1] < shared_stuff->arr[c]) {
                char temp = shared_stuff->arr[c];
                shared_stuff->arr[c] = shared_stuff->arr[c + 1];
                shared_stuff->arr[c + 1] = temp;
                }
            } 
            o++;   
        }
        printf("\n-------------------------------------------------------------------------------------\n");
        semaphore_v();
        break;
    
    default:
        //waiting for the 3 children to execute
        wait(NULL);
        wait(NULL);
        wait(NULL);
        printf("\n-------------------------------------------------------------------------------------\n");
        printf("                        Parent process started excuting  \n\n");
        printf("The ordered array is: \n");

        for (int i = 0; i < SIZE_OF_ARRAY; i++) {
            printf("%c\t", shared_stuff-> arr[i]);
        }
        printf("\n");
       
        // Deattaching and clearing the shared memo
        if (shmdt(shared_memory) == -1) {	
            fprintf(stderr, "shmdt failed\n");	
            exit(EXIT_FAILURE);
        }
        if (shmctl(shmid, IPC_RMID, 0) == -1){
            fprintf(stderr, "shmctl(IPC_RMID) failed\n");
            exit(EXIT_FAILURE);	
        }
        break;
    }
}

//semaphore funstions
static int set_semvalue(void){
    union semun sem_union;
    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
        return (0);
    return (1);
}

static void del_semvalue(void){
    union semun sem_union;
    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

static int semaphore_p(void){
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1)
    {
        fprintf(stderr, "semaphore_p failed\n");
        return (0);
    }
    return (1);
}

static int semaphore_v(void){
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1){
        fprintf(stderr, "semaphore_v failed\n");
        return (0);
    }
    return (1);
}