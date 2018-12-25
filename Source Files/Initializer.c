#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "Utilities.h"
#include "Initializer.h"
#include "Handler.h"

/* global variable to count the number of SIGUSR2 sent */
volatile sig_atomic_t signal_received = 0;

int main(int argc,char *argv[])
{
  FILE *ofp;
  char *Dataset, *Pattern, *Height;
  char my_fifo[25], start[12], end[12], split[12], sum[12], no_records[12], str_root_id[12];
  int fd, status, ret_val, Height_num, first;
  int s = 0;
  double time_taken, time_sum_s, time_min_s, time_max_s, time_sum_sm, time_min_sm, time_max_sm, total;
  pid_t pid;
  record_t rec;
  /* struct defined on <signals.h> */
  struct sigaction sa;
  /* struct define in <sys/time.h> */
  struct timeval t_start, t_end;

  gettimeofday(&t_start, NULL);

  /* Checking arguments */
  if(argc < 7 || argc > 8 )
  {
    perror("Command Execution Failed");
    exit(1);
  }
  else
  {
    for(int i = 1; i < argc; i++)
    {
      if(!strcmp(argv[i],"-h"))
      {
        if(atoi(argv[i+1]) < 1 || atoi(argv[i+1]) > 5)
        {
          perror("Wrong Height Input");
          exit(1);
        }
        Height = (char *) malloc((strlen(argv[i+1])+1) * sizeof(char));
        strcpy(Height,argv[i+1]);
        Height_num = atoi(argv[i+1]);
      }
      else if(!strcmp(argv[i],"-d"))
      {
        Dataset = (char *) malloc((strlen(argv[i+1])+1) * sizeof(char));
        strcpy(Dataset,argv[i+1]);
      }
      else if(!strcmp(argv[i],"-p"))
      {
        Pattern = (char *) malloc((strlen(argv[i+1])+1) * sizeof(char));
        strcpy(Pattern,argv[i+1]);
      }
      else if(!strcmp(argv[i],"-s"))
        s = 1;
    }
  }
  /* Create a struct record_t array containing all the info about our records in the .bin file */
  const int num_records = Count_Records(Dataset);

  if( num_records == -1 )
  {
    perror("File Opening Failed");
    free(Dataset);
    free(Pattern);
    free(Height);
    exit(1);
  }

  sprintf(no_records,"%d",num_records);
  /*if everything ok create the strings to determine the range of searching */
  sprintf(start,"%d",0);
  sprintf(end,"%d",num_records);
  /* string to represent the -s flag */
  sprintf(split,"%d",s);
  /* sum is a string which contains the arithmetic spsum from n=1 up to 2^height to use it in the case of -s flag */
  int from = 1;
  int to = power(2,Height_num);
  int s_sum = Calculate_Sum(from,to);
  sprintf(sum,"%d",s_sum);

  /* Create fifo to communicate between root proccess and initial Splitter/Merger proccess */
  sprintf(my_fifo,"Root_ID_%d",getpid());
  ret_val = mkfifo(my_fifo,0666);
  if((ret_val == -1) && (errno != EEXIST))
  {
    perror("Creating Fifo Failed");
    exit(1);
  }

  /* Open output file */
  ofp = fopen ("output.txt" ,"w");
	if (ofp == NULL)
  {
    perror("Opening Output File Failed");
    exit(1);
  }

  /* get the root id to trigger SIGUSR2 via searcher */
  sprintf(str_root_id,"%d",getpid());

  /* specify the function to be executed when signal is caught */
  sa.sa_handler = sig_handler;

  sa.sa_flags = SA_RESTART;
  /* block every signal when running the sig_handler function */
  sigfillset(&sa.sa_mask);
  if (sigaction(SIGUSR2, &sa, NULL) == -1)
  {
    perror("Sigaction Execution Failed");
    exit(1);
  }
  /* Generate the proccess binary tree */
  pid = fork();

  if(pid < 0)
  {
    perror("fork() Execution Failed");
    exit(1);
  }
  /* Child proccess, so call Initial Splitter/Merger */
  else if(pid == 0)
  {
    execl("./SplitterMerger", "SplitterMerger", Dataset, Height, Pattern, start, end, split, sum, no_records, my_fifo, str_root_id, (char *) NULL);
    perror("execl() Execution Failed");
    exit(1);
  }
  /* Parent proccess, so collect all information through pipe */
  else
  {
    /* Pipe that communicates with Initial Merger*/
    fd = open(my_fifo, O_RDONLY );
    read(fd ,&rec, sizeof(record_t));
		while((rec).custid != -1)
		{
      fprintf(ofp,"%ld %s %s %s %d %s %s %-9.2f\n",rec.custid,rec.LastName,rec.FirstName,rec.Street,rec.HouseID,rec.City,rec.postcode,rec.amount);
      read(fd ,&rec, sizeof(record_t));
		}

    /* statistics time for searchers */
    read(fd,&time_taken,sizeof(double));
    time_sum_s = 0;
    first = 1;

    while(time_taken != -1.0 )
    {
      time_sum_s += time_taken;
      if(first)
      {
        time_min_s = time_taken;
        time_max_s = time_taken;
        first = 0;
      }
      else
      {
        if(time_taken < time_min_s)
          time_min_s = time_taken;

        if(time_taken > time_max_s)
          time_max_s = time_taken;
      }
      read(fd,&time_taken,sizeof(double));
    }

    /* statistics time for SM */
    read(fd,&time_taken,sizeof(double));
    time_sum_sm = 0;
    first = 1;

    while(time_taken != -1.0 )
    {
      time_sum_sm += time_taken;
      if(first)
      {
        time_min_sm = time_taken;
        time_max_sm = time_taken;
        first = 0;
      }
      else
      {
        if(time_taken < time_min_sm)
          time_min_sm = time_taken;

        if(time_taken > time_max_sm)
          time_max_sm = time_taken;
      }
      read(fd,&time_taken,sizeof(double));
    }

  }
  /* Wait communication between Initial Splitter/Merger and Root to finish */
  wait(&status);
  /* close file descriptor,output file and delete the named pipe */
  close(fd);
  fclose(ofp);
  unlink(my_fifo);
  /* create a new proccess to sort the output.txt and print it to the stdout */
  pid = fork();

  if(pid < 0)
  {
    perror("fork() Execution Failed");
    exit(1);
  }
  /* Child proccess, so call sort Unix Command */
  else if(pid == 0)
  {
    execlp("sort", "sort", "-nk1", "output.txt", (char *) NULL);
    perror("execl() Execution Failed");
    exit(1);
  }
  /* Parent proccess, so collect all information through pipe */
  else
  {
    /* wait sorting proccess to terminate */
    wait(&status);
    /* remove output.txt */
    remove("output.txt");
  }

  gettimeofday(&t_end, NULL);
  total = ((t_end.tv_sec * 1000000 + t_end.tv_usec) - (t_start.tv_sec * 1000000 + t_start.tv_usec)) / 1000000.0;
  /* print the number of SIGUSR2 received by searchers */
  printf("\n%d SIGUSR2 received by searchers\n",signal_received);
  /* print time statistics */
  printf("\nSearchers: Average time %f sec - Min time %f sec - Max time %f sec\n",time_sum_s/power(2,Height_num),time_min_s,time_max_s);
  printf("Splitters Mergers: Average time %f sec - Min time %f sec - Max time %f sec\n",time_sum_sm/power(2,Height_num - 1),time_min_sm,time_max_sm);
  printf("Turnaround time: %f sec\n",total);

  /* free all the memory allocated */
  free(Dataset);
  free(Pattern);
  free(Height);
  return EXIT_SUCCESS;
}

int Count_Records(char *FileName)
{
  /* open file using file descriptor */
  FILE *file_desc = fopen(FileName,"rb");
  /* Check if everything gone ok */
  if(file_desc == NULL)
    return -1;
  /* If do so, find the number of records */
  fseek (file_desc , 0 , SEEK_END);
  long file_size = ftell (file_desc);
  int records = file_size / sizeof(record_t);

  fclose(file_desc);
  return records;
}


int power(int x, unsigned int y)
{
    if (y == 0)
        return 1;
    else if (y % 2 == 0)
        return power(x, y/2)*power(x, y/2);
    else
        return x*power(x, y/2)*power(x, y/2);
}

int Calculate_Sum(int start,int end)
{
  int sum = 0;

  for(int i = start; i <= end; i++)
    sum += i;

  return sum;
}
