#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include <sys/wait.h>
#include<string.h>
#include<sys/types.h>
#include<time.h>
#define BUFFER_SIZE 1000



int CCE[10];//stores num of CCE in thread 1 -> 10 separatly
int ECE[10];//same but for ECE
int CMPS[10];//same but for CMPS
int flag[10];//stores a flag for each thread to tell that the work is completed for this thread
int totalCCE =0;
int totalECE=0;
int totalCMPS=0;
int fileSize;

//function that takes a file, a word, an integer indicating which
//thread is calling "i" ,and the total number of threads "N" as
//arguments and returns the occurences of the word in the part
//of the file specified for that thread
int countOccurences(FILE *fptr, const char *word, int i, int N){

        char str[BUFFER_SIZE];
        char *pos;
        int index;
        int count;
        count = 0;

        fseek(fptr,(i*(fileSize/N)),SEEK_SET);
        //fseek to start searching at indicated part of the file
        //related to the thread number i
        while (((fgets(str,BUFFER_SIZE,(fptr)))!=NULL)){
                //ftell returns our current position in the file
                //and we compare it to where we want the file to
                //end to know when we should stop
                if(ftell(fptr)<( (i+1)*(fileSize/N))){
                index = 0;

                while ((pos = strstr(str + index , word))!= NULL){

                        index = (pos-str) + 1;
                        count ++;
                }

        }}
return count;}

//we use this struct to pass 2 arguments (current thread number and
//total number of threads) at the same time to the
//function mapper while we create the thread
struct info{int index; int N;};

//mapper function will be called when the mapper threads get created
void *mapper(void *i){

int x =(((struct info *)i) -> index);//current thread number
int y = (((struct info *)i) -> N);//Total number of threads
FILE *file;

//printf("hello from thr %d \n",x);

file = fopen("input.txt","r");
fseek(file,0,SEEK_SET);
CCE[x]= countOccurences(file,"CCE",x,y);
fclose(file);

file = fopen("input.txt","r");
ECE[x]= countOccurences(file,"ECE",x,y);
fclose(file);

file = fopen("input.txt","r");
CMPS[x]= countOccurences(file,"CMPS",x,y);
fclose(file);

printf("CCE in thread %d : %d \n",x,CCE[x]);
printf("ECE in thread %d : %d \n",x,ECE[x]);
printf("CMPS in thread %d : %d \n",x,CMPS[x]);

flag[x] = 999;
printf("Flag in thread %d is : %d\n\n",x,flag[x]);

pthread_exit(NULL);
}






void *reducerCCE(void*N){
        int y = *((int*)N);
        printf("reducer CCE created \n");
        for (int i = 0; i<y ; i++)
        	totalCCE = totalCCE + CCE[i];
        printf("Total CCE is: %d\n",totalCCE);

}

void *reducerECE(void*N){
        int y = *((int*)N);
        printf("reducer ECE created\n");
          for (int i = 0; i<y ; i++)
          	totalECE = totalECE + ECE[i];
	  printf("Total ECE is: %d\n",totalECE);
}

void *reducerCMPS(void*N){
        int y = *((int*)N);
        printf("reducer CMPS created\n");
          for (int i = 0; i<y ; i++)
        {totalCMPS = totalCMPS + CMPS[i];}
        printf("Total CMPS is: %d\n",totalCMPS);
}





int main(int argc,char**argv){
	
	clock_t start = clock();
        int N = atoi(argv[1]);

        FILE*fp = fopen("input.txt","r");
        fseek (fp,0L,SEEK_END);//to get total file size in Bytes
        fileSize= ftell(fp);
        fseek(fp,0L,SEEK_SET);

        printf("File size is %d Bytes\n",fileSize);



        int thr;
        struct info INFO[N];//for each thread i its own info that we will pass as an argument
        pthread_t mthreads[N];//mapper threads
        pthread_t rthreads[N];//reducer threads


        //creating mappers
        for (int i = 0 ; i<N ;i++){
                INFO[i].index=i;//for each thread its own thread num
                INFO[i].N=N;
                //printf("creating mapper thread %d\n", i);
                thr = pthread_create(&mthreads[i],NULL,mapper,&INFO[i]);
                if (thr != 0) exit(-1);
        }

        //waiting for mappers to finish with flags
        for (int i = 0; i<N ; i++){
                while(flag[i]!=999){usleep(100);}
        }

        //creating reducers
        thr=pthread_create(&rthreads[1],NULL,reducerCCE,&N);
        thr=pthread_create(&rthreads[1],NULL,reducerECE,&N);
        thr=pthread_create(&rthreads[1],NULL,reducerCMPS,&N);

	//Finding the runtime and storing it in temp2.txt
	clock_t end = clock();
	double exectime = (double)(end - start)/CLOCKS_PER_SEC;
        printf("Time taken: %0.10f\n", exectime);
	char time[10];
        sprintf(time, "%f", exectime);
        FILE* file;
        file = fopen("temp2.txt", "a+");
	fputs(time, file);
	fputs(" ", file);
	fclose(file);	
	pthread_exit(NULL);
}
