#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>

#include "../headers/Utilities.h"
#include "../headers/SplitterMerger.h"

int main(int argc,char *argv[])
{
  double time_l, time_r, searchers_time, sm_time, sm_time_l, sm_time_r, standard_sm;
  char *Dataset, *Pattern;
  int File_D, status, ret_val, fd_ls, fd_rs, fd_lsm, fd_rsm;
  int start, end, new_start, new_end, s_sum, num_records, split_flag, Height;
  char my_fifo_ls[25], my_fifo_rs[25], my_fifo_lsm[25], my_fifo_rsm[25];
  char start_l[12], end_l[12], start_r[12], end_r[12], Height_str[12];
  pid_t pid_l_s, pid_r_s, pid_l_sm, pid_r_sm;
  clock_t t, st_time;
  record_t rec;
  /* start clock for standard time taken by SplitterMerger */
  st_time = clock();
  /* 1st argument contains the name of binary file */
  Dataset = (char *) malloc((strlen(argv[1])+1) * sizeof(char));
  strcpy(Dataset,argv[1]);
  /* 2nd argument contains the string of Height */
  Height = atoi(argv[2]);
  /* 3rd argument contains the Pattern */
  Pattern = (char *) malloc((strlen(argv[3])+1) * sizeof(char));
  strcpy(Pattern,argv[3]);
  /* 4th argument contains start point */
  start = atoi(argv[4]);
  /* 5th argument contains end point */
  end = atoi(argv[5]);
  /* determine how to set start and end point (-s flag) */
  split_flag = atoi(argv[6]);
  /* get the arithmetic value of the sum from n = 1 up to 2^h of n */
  s_sum = atoi(argv[7]);
  /* get the arithmetic value of the sum from n = 1 up to 2^h of n */
  num_records = atoi(argv[8]);
  /* open File_D to communicate via my_fifo */
  File_D = open(argv[9],O_WRONLY);
  /* Calculate start and points for both left and right node */
  new_start = Start_Left(start);
  sprintf(start_l,"%d",new_start);
  new_end = End_Left(start,end,split_flag,s_sum,num_records,Height - 1);
  sprintf(end_l,"%d",new_end);
  new_start = Start_Right(new_end);
  sprintf(start_r,"%d",new_start);
  new_end = End_Right(end);
  sprintf(end_r,"%d",new_end);
  /* Convert height - 1 to string to pass it as argument */
  sprintf(Height_str,"%d",Height - 1);
  /* get standard time taken by SplitterMerger */
  st_time = clock() - st_time;
  standard_sm = (double) st_time / CLOCKS_PER_SEC;
  /* if height is 1 create two searcher nodes */
  if(Height == 1)
  {
    /* create pipe to communicate between Splitter/Merger and left Searcher */
    sprintf(my_fifo_ls,"LeftSearcher_ID_%d",getpid());
    ret_val = mkfifo(my_fifo_ls, 0666);
    if((ret_val == -1) && (errno != EEXIST))
    {
      perror("Creating Fifo Failed\n");
      exit(1);
    }

    pid_l_s = fork();
    if(pid_l_s < 0)
    {
      perror("fork() Execution Failed");
      exit(1);
    }
    /* Child proccess, so create the left searcher */
    else if(pid_l_s == 0)
    {
      execl("./Searcher", "Searcher", Dataset, Pattern, start_l, end_l, my_fifo_ls, argv[10], (char *) NULL);
      perror("execl() Execution Failed");
    }
    /* Parent proccess, so do the same to create now the right searching proccess */
    else
    {
      /* create pipe to communicate between Splitter/Merger and right Searcher */
      sprintf(my_fifo_rs,"RightSearcher_ID_%d",getpid());
      ret_val = mkfifo(my_fifo_rs,0666);
      if((ret_val == -1) && (errno != EEXIST))
      {
        perror("Creating Fifo Failed");
        exit(1);
      }

      pid_r_s = fork();
      if(pid_r_s < 0)
      {
        perror("fork() Execution Failed");
        exit(1);
      }
      /* Child proccess, so now create the right searcher */
      else if(pid_r_s == 0)
      {
        execl("./Searcher", "Searcher", Dataset, Pattern, start_r, end_r, my_fifo_rs, argv[10], (char *) NULL);
        perror("execl() Execution Failed");
      }
      /* Parent proccess of searchers, so we are in a Splitter/Merger node */
      else
      {
        /* start clock for Splitter/Merger */
        t = clock();
        /* open pipe that communicate with the left searcher */
        fd_ls = open(my_fifo_ls,O_RDONLY);

        read(fd_ls, &rec, sizeof(record_t));
        while((rec).custid != -1)
        {
          write(File_D ,&rec, sizeof(record_t));
          read(fd_ls, &rec, sizeof(record_t));
        }
        read(fd_ls,&time_l,sizeof(double));

        /* open pipe that communicate with the right searcher */
        fd_rs = open(my_fifo_rs,O_RDONLY);

        read(fd_rs, &rec, sizeof(record_t));
        while((rec).custid != -1)
        {
          write(File_D ,&rec, sizeof(record_t));
          read(fd_rs, &rec, sizeof(record_t));
        }
        read(fd_rs,&time_r,sizeof(double));
        rec.custid = -1;
        write(File_D, &rec, sizeof(record_t));

        /* write time taken both by left and right searchers */
        write(File_D, &time_l, sizeof(double));
        write(File_D, &time_r, sizeof(double));
        searchers_time = - 1.0;
        write(File_D, &searchers_time, sizeof(double));

        /* wait for both searchers to complete their job */
        wait(&status);
        wait(&status);
        /* get time from SplitterMerger */
        t = clock() - t;
        sm_time = (double) t / CLOCKS_PER_SEC;
        /* add standard time taken by each SplitterMerger and the time of two searchers to find total time of current SM */
        sm_time += standard_sm + time_l + time_r ;
        /* and write it to the named pipe */
        write(File_D, &sm_time, sizeof(double));

        sm_time = -1;
        write(File_D, &sm_time, sizeof(double));

        /* close file descriptor and delete the named pipe using by the left searcher */
        close(fd_ls);
        unlink(my_fifo_ls);

      }
      /* close file descriptor and delete the named pipe using by the right searcher */
      close(fd_rs);
      unlink(my_fifo_rs);
    }
  }
  /* in this case we create two Splitters/Mergers to continue the job */
  else
  {
    /* create pipe to communicate between Splitter/Merger and left SM */
    sprintf(my_fifo_lsm,"LeftSM_ID_%d",getpid());
    ret_val = mkfifo(my_fifo_lsm, 0666);
    if((ret_val == -1) && (errno != EEXIST))
    {
      perror("Creating Fifo Failed");
      exit(1);
    }

    pid_l_sm = fork();

    if(pid_l_sm < 0)
    {
      perror("fork() Execution Failed");
      exit(1);
    }
    /* Child proccess, so create the left Splitter/Merger */
    else if(pid_l_sm == 0)
    {
      execl("./SplitterMerger", "SplitterMerger", Dataset, Height_str, Pattern, start_l, end_l, argv[6], argv[7], argv[8], my_fifo_lsm, argv[10], (char *) NULL);
      perror("execl() Execution Failed");
    }
    /* Parent proccess, so do the same to create now the right Splitter/Merger proccess */
    else
    {
      /* create pipe to communicate between Splitter/Merger and right SM */
      sprintf(my_fifo_rsm,"RightSM_ID_%d",getpid());
      ret_val = mkfifo(my_fifo_rsm,0666);
      if((ret_val == -1) && (errno != EEXIST))
      {
        perror("Creating Fifo Failed");
        exit(1);
      }

      pid_r_sm = fork();
      if(pid_r_sm < 0)
      {
        perror("fork() Execution Failed");
        exit(1);
      }
      /* Child proccess, so now create the right SM */
      else if(pid_r_sm == 0)
      {
        execl("./SplitterMerger", "SplitterMerger", Dataset, Height_str, Pattern, start_r, end_r, argv[6], argv[7], argv[8], my_fifo_rsm, argv[10],(char *) NULL);
        perror("execl() Execution Failed");
      }
      /* Parent proccess of searchers, so we are in a Splitter/Merger node */
      else
      {
        /* start clock for Splitter/Merger */
        t = clock();
        /* open pipe that communicate with the left SM */
        fd_lsm = open(my_fifo_lsm,O_RDONLY);

        read(fd_lsm, &rec, sizeof(record_t));
        while((rec).custid != -1)
        {
          write(File_D ,&rec, sizeof(record_t));
          read(fd_lsm, &rec, sizeof(record_t));
        }

        /* open pipe that communicate with the right SM */
        fd_rsm = open(my_fifo_rsm,O_RDONLY);

        read(fd_rsm, &rec, sizeof(record_t));
        while((rec).custid != -1)
        {
          write(File_D ,&rec, sizeof(record_t));
          read(fd_rsm, &rec, sizeof(record_t));
        }

        rec.custid = -1;
        write(File_D, &rec, sizeof(record_t));

        /* write statistics about searchers time after custid equals -1 */
        read(fd_lsm,&time_l,sizeof(double));
        while(time_l != -1.0 )
        {
          write(File_D, &time_l, sizeof(double));
          read(fd_lsm,&time_l,sizeof(double));
        }

        read(fd_rsm,&time_r,sizeof(double));
        while(time_r != -1.0 )
        {
          write(File_D, &time_r, sizeof(double));
          read(fd_rsm,&time_r,sizeof(double));
        }

        searchers_time = - 1.0;
        write(File_D, &searchers_time, sizeof(double));

        /* wait until both SM's complete their job */
        wait(&status);
        wait(&status);
        /* when SplitterMerger complete its job get time taken */
        t = clock() - t;
        sm_time = (double) t / CLOCKS_PER_SEC;
        /* write statistics about SM's time after searchers_time equals -1 */
        read(fd_lsm,&sm_time_l,sizeof(double));
        read(fd_rsm,&sm_time_r,sizeof(double));
        sm_time += standard_sm + sm_time_l + sm_time_r;
        /* and write total time of current SM to the named pipe */
        write(File_D, &sm_time, sizeof(double));
        while((sm_time_l != -1) && (sm_time_r != -1))
        {
          /* add standard time taken by each SplitterMerger and times of both left and right SM */
          write(File_D, &sm_time_l, sizeof(double));
          read(fd_lsm,&sm_time_l,sizeof(double));

          write(File_D, &sm_time_r, sizeof(double));
          read(fd_rsm,&sm_time_r,sizeof(double));
        }

        sm_time = -1;
        write(File_D, &sm_time, sizeof(double));

        /* close file descriptor and delete the named pipe using by the left sm */
        close(fd_lsm);
        unlink(my_fifo_lsm);
      }
      /* close file descriptor and delete the named pipe using by the right sm */
      close(fd_rsm);
      unlink(my_fifo_rsm);
    }
  }

  close(File_D);
  free(Dataset);
  free(Pattern);

  return EXIT_SUCCESS;
}

int Start_Left(int start)
{
  return start;
}

int End_Left(int start,int end,int split_flag,int s_sum,int k,int h)
{
  /* in this case each searcher checks k/2^h recrods */
  if(!split_flag)
  {
    int diff = end - start;
    if(diff % 2 == 1)
      return (start + (diff/2 +1));
    else
      return (start + diff/2);
  }
  /* compute range of checking as the -s flag indicates */
  else
  {
    int sum = 0;
    int upper_bound = power(2,h);
    for(int i = 1; i <= upper_bound; i++)
      sum+= k * i;

    return start + sum/s_sum;
  }
}

int Start_Right(int left_end)
{
  return left_end;
}

int End_Right(int end)
{
  return end;
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
