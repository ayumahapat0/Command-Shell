#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#define PROMPT "wsh> "
#define EXIT "exit"
#define CHDIR "cd"
#define JOBS "jobs"
#define EXIT "exit"
#define FG "fg"
#define BG "bg"
#define MAXLEN 256

// struct to keep track of each job put in by the user
typedef struct jobs{
  char ***commands;
  int num_commands;
  int jobid;
  pid_t pgid;
  int bg;
  char * command_line;
}job;

// keep track of the job I am looking at
int job_index = -1;