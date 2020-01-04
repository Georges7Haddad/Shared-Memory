#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/shm.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/wait.h>
#include<string.h>

int main()
{
    
    double execNo[4];
    double execOp[4];
    char arg[3];
    int values[4]={1,2,4,8};
    pid_t worker_pids[4];

    //Run 4 non-optimized splits and store their time in temp.txt
    for(int i=0;i<4;i++)
    {
        worker_pids[i]=fork();
        if(worker_pids[i]==0)
        {
		sprintf(arg, "%d", values[i]);
		execl("shm","shm", arg, NULL);
         }
    }
    
    int status;
    for(int i=0;i<4;i++)
    {
        waitpid(worker_pids[i] ,&status, 0);
    }

    

    //Run 4 optimized splits and store their time in temp2.txt
    for(int i=0;i<4;i++)
    {
        worker_pids[i]=fork();
        if(worker_pids[i]==0)
        {
		sprintf(arg, "%d", values[i]);
		execl("optimization","optimization", arg, NULL);
         }
    }
    
    for(int i=0;i<4;i++)
    {
        waitpid(worker_pids[i] ,&status, 0);
    }

    //Store temp.txt values in execNo
    FILE* file;
    file = fopen("temp.txt", "r"); 
    for(int i =0; i<4 ; i++)
    {
	    fscanf(file,"%lf", &execNo[i]);	
    }
    fclose(file);

    //Store temp2.txt values in execOp
    file = fopen("temp2.txt", "r"); 
    for(int i = 0; i<4; i++)
    {
	   fscanf(file,"%lf", &execOp[i]);
    }
    fclose(file);


    //Writing to bench.txt
    file = fopen("bench.txt", "w");
    fprintf(file,"%s %f %f\n", "1", execNo[0], execOp[0]);
    fprintf(file,"%s %f %f\n", "2", execNo[1], execOp[1]);
    fprintf(file,"%s %f %f\n", "4", execNo[2], execOp[2]);
    fprintf(file,"%s %f %f\n", "8", execNo[3], execOp[3]);
    fclose(file);
	
//  I tried to run gnuplot and ps2pdf but it's not working    
    worker_pids[0] = fork();
    if(worker_pids[0] == 0)
	    execl("gnuplot","gnuplot", "plot", NULL);
    else if(worker_pids[0] > 0){
	    wait(&status);
	    execl("ps2pdf", "ps2pdf", "bench.ps", NULL);
    }	
    return 0;
}
