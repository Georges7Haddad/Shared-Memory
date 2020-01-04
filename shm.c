#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#define BUFFER_SIZE 1000

int countOccurences(FILE *fptr, const char *word){

        char str[BUFFER_SIZE];
        char *pos;
        int index;
        int count;
        count = 0;

        while ((fgets(str,BUFFER_SIZE,fptr))!=NULL){

                index = 0;

                while ((pos = strstr(str + index , word))!= NULL){

                        index = (pos-str) + 1;
                        count ++;
                }

        }
return count;

}
int main(int argc, char** argv) {
	
	clock_t start = clock();
        int lines;
        int N = atoi(argv[1]);
        lines = 320000/N;
	
	//New child process to execute the split
        pid_t childpid = fork();
        if (childpid == 0){
                char Flines[5]; 
                sprintf(Flines, "%d", lines);              
                execlp("split", "split","-l", Flines,"-a", "1", "-d", "input.txt", "output", NULL);
        }
        else if(childpid > 0){

                int status;
                wait(&status);


                int shmID[N];
                pid_t worker[N];
                pid_t reducer[3];
		
		//Generating an array of key with 4*N entries because each worker(total of N) will have 4 segments of data
	        //Keys have to be different and different than 0
        	key_t key[4*N];
		for(int i =0; i<4*N; i++)
                	key[i] = i+1;


                int CCE, ECE, CMPS = 0;
                char path[8];
                char* Flag[4];
                FILE *file;
 
		//Create N workers
                for(int i=0; i<N; i++){
                        worker[i] = fork();
                        sprintf(path, "output%d", i);	//Each workeri gets the file:outputi
                        if (worker[i] == 0) {
				
					//Creating shared memory segments
					//Segments with key[0==>N-1] have the flag of worker 0 to N-1
					//Segments with key[N==>2N-1] have the CCE count of worker 0 to N-1
					//Segments with key[2N==>3N-1] have the eCE count of worker 0 to N-1
					//Segments with key[3N==>4N-1] have the CMPS count of worker 0 to N-1			
				for(int j = 0; j<4; j++){
					shmID[j] = shmget(key[i+j*N], (4*sizeof(long)), IPC_CREAT | S_IRUSR | S_IWUSR);

					//Attaching to the shared memory segments
					Flag[j] = (char*) shmat(shmID[j], NULL, 0);
				}
				printf("WORKER%d: \n", i);
				
				//Count CCE
				file = fopen (path, "r");
                                CCE = countOccurences(file, "CCE");
                                printf("\t'CCE' is found %d times in %s. \n", CCE, path);
                                fclose(file);
				
				//Count ECE
                                file = fopen (path, "r");
                                ECE = countOccurences(file, "ECE");
                                printf("\t'ECE' is found %d times in %s. \n",  ECE, path);
                                fclose(file);
                                
				//Count CMPS
				file = fopen (path, "r");
                                CMPS = countOccurences(file, "CMPS");
                                printf("\t'CMPS' is found %d times in %s. \n", CMPS, path);
                                fclose(file);
                                
				//Assigning the shared memory segments
				sprintf(Flag[0], "999");
                                sprintf(Flag[1], "%d", CCE);
				sprintf(Flag[2], "%d", ECE);
				sprintf(Flag[3], "%d", CMPS);				
				printf("\tFlag[0],Flag: %s Flag[1], CCE: %s Flag[2], ECE: %s Flag[3], CMPS: %s\n", Flag[0], Flag[1], Flag[2], Flag[3]);
				
				//Detaching from the shared memory segments
				shmdt(Flag[0]);
				shmdt(Flag[1]);
                                shmdt(Flag[2]);
				shmdt(Flag[3]);
				exit(0);
				
                        }
                        else if (worker[i] > 0) {
                                wait(&status);
                        }
                }
                for(int i =0; i<3; i++) {

			reducer[i] = fork(); //Reducer 0 counts CCE | Reducer 1 counts ECE | Reducer 2 counts CMPS
                        char* flag[4];
			int sum = 0;
                        if(reducer[i] == 0) {
				for(int j =0; j< N; j++) {	

					//Getting the shared segments ID
                                	shmID[0] = shmget(key[j], (4*sizeof(long)), S_IRUSR | S_IWUSR);
					shmID[1] = shmget(key[i*N+N+j], (4*sizeof(long)), S_IRUSR | S_IWUSR);	

					//Attaching to the shared memory segments
					flag[0] = (char*) shmat(shmID[0], NULL, 0);	//flag[0] has now the flag
					flag[1] = (char*) shmat(shmID[1], NULL, 0);	//flag[1] has now the count
					
					//Stop until Flag[0] = "999" ==> Worker terminated
					while(strcmp(flag[0],"999") == 1)
						sleep(1);
										
					//Calculating the sum
					sum = sum + atoi(flag[1]);

					//Detaching from the shared memory segments
                                	shmdt(flag[0]);
					shmdt(flag[1]);
				}

				//Printing the Count
				if(i == 0)
					printf("SUM CCE: %d\n\n", sum);
				else if(i == 1)
					printf("SUM ECE: %d\n\n", sum);
				else if(i == 2)
					printf("SUM CMPS: %d\n\n", sum);
				exit(10);

                        }
                        else if(reducer[i] > 0) {
                                wait(&status);
                        }
                }
        }
	//Finding the runtime and storing it temp.txt
	clock_t end = clock();
	double exectime = (double)(end - start)*1000/CLOCKS_PER_SEC;
	printf("Time taken: %0.10f\n", exectime);

	char time[10];
	sprintf(time, "%f", exectime);
	FILE* file;
	file = fopen("temp.txt", "a+");
	fputs(time, file);
	fputs(" ",file);
	fclose(file);
	return 0;
}
