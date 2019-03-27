#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>


#include "../headers/Utilities.h"
#include "../headers/Searcher.h"

int main(int argc,char *argv[])
{
  FILE *fpb;
  char *Pattern;
  int i, j, start, end, fd;
  record_t rec;
  clock_t t;

  t = clock();
  /* Extract the pid of the root */
  pid_t pid = atoi(argv[6]);

  /* Extract Data from arguments */
  fpb = fopen (argv[1],"rb");
  if (fpb == NULL)
  {
    perror("Open Binary File Failed\n");
    exit(1);
  }

  Pattern = (char *) malloc((strlen(argv[2])+1) * sizeof(char));
  strcpy(Pattern,argv[2]);

  start = atoi(argv[3]);
  end = atoi(argv[4]);
  /* open pipe that communicate with parent SplitterMerger */
  fd = open(argv[5],O_WRONLY);

  /* skip records until reached the Correct range of checking */
  for(i = 0; i < start; i++)
    fread(&rec, sizeof(record_t), 1, fpb);

  for(j = i; j < end; j++)
  {
    fread(&rec, sizeof(record_t), 1, fpb);
    if(Check_Pattern(rec,Pattern))
      write(fd, &rec, sizeof(record_t));
  }
  rec.custid = -1;
  write(fd, &rec, sizeof(record_t));

  t = clock() - t;

  double time_taken = (double) t / CLOCKS_PER_SEC;

  write(fd, &time_taken, sizeof(double));

  close(fd);
  fclose(fpb);
  free(Pattern);
  /* send signal SIGUSR2 to the root */
  kill(pid,SIGUSR2);

  return EXIT_SUCCESS;
}

int Check_Pattern(record_t my_record,char *Pattern)
{
  char *res, cust_ID[25], House_ID[25], salary[25];

  res = strstr(my_record.LastName,Pattern);
  if(res != NULL)
    return 1;

  res = strstr(my_record.FirstName,Pattern);
  if(res != NULL)
    return 1;

  res = strstr(my_record.Street,Pattern);
  if(res != NULL)
    return 1;

  res = strstr(my_record.City,Pattern);
  if(res != NULL)
    return 1;

  res = strstr(my_record.postcode,Pattern);
  if(res != NULL)
    return 1;

  sprintf(cust_ID,"%ld",my_record.custid);
  res = strstr(cust_ID,Pattern);
  if(res != NULL)
    return 1;

  sprintf(House_ID,"%d",my_record.HouseID);
  res = strstr(House_ID,Pattern);
  if(res != NULL)
    return 1;

  sprintf(salary,"%9.2f",my_record.amount);
  res = strstr(salary,Pattern);
  if(res != NULL)
    return 1;

  /* no match found so return 0 */
  return 0;
}
