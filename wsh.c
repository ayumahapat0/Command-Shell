#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "wsh.h"

// signal handlers
void intsig_handler(){

}

void stopsig_handler(){

}

// jobs list
job *jobs_array[256] = {NULL};

// exit function
int exit_function(){
  for(int i = 0; i < MAXLEN; i++){
      if(jobs_array[i] != NULL){
        for(int j = 0; j < MAXLEN; j++){
          free(jobs_array[i]-> commands[j]);
        }
        free(jobs_array[i] -> commands);
        free(jobs_array[i]);
        jobs_array[i] = NULL;
      }
    }
  exit(0);
}

// change directory
int ch_dir(char *path){
  if(chdir(path) < 0){
      printf("Error: could not change to said directory\n");
      return -1;
  }
  return 0;
}

// remove jobs that have already been completed
int remove_old_jobs(){
  for(int i = 0; i < MAXLEN; i++){
    if(jobs_array[i] != NULL){
      pid_t ret = waitpid(-(jobs_array[i] -> pgid), NULL, WNOHANG);

      if(ret == jobs_array[i] -> pgid || ret == -1){
        for(int j = 0; j < MAXLEN; j++){
          free(jobs_array[i]-> commands[j]);
        }
        free(jobs_array[i] -> commands);
        free(jobs_array[i]);
        jobs_array[i] = NULL;
      }
  }
}
return 0;
}

// print all jobs in the background or suspended
int jobs(){
  for(int i = 0; i < MAXLEN; i++){
    if(jobs_array[i] != NULL){
      pid_t ret = waitpid(-(jobs_array[i] -> pgid), NULL, WNOHANG);

      if(ret == jobs_array[i] -> pgid || ret == -1){
        for(int j = 0; j < MAXLEN; j++){
          free(jobs_array[i]-> commands[j]);
        }
        free(jobs_array[i] -> commands);
        free(jobs_array[i]);
        jobs_array[i] = NULL;
      }
      else{
        if (jobs_array[i] -> bg || ret == 0){
          printf("%d: %s\n", jobs_array[i] -> jobid, jobs_array[i] -> command_line);
        }
      }
    }
  }

  return 0;
}

// put a process specified by id into the background
// give control back to the shell
int bg(char * arg){
  // kill all zombie processes
  int max_id = -1;
  if(arg == NULL){
    for(int i = 0; i < MAXLEN; i++){
      if(jobs_array[i] != NULL){
        pid_t ret = waitpid(-(jobs_array[i] -> pgid), NULL, WNOHANG);

        if(ret == jobs_array[i] ->pgid || ret == -1){
          for(int j = 0; j < MAXLEN; j++){
            free(jobs_array[i]-> commands[j]);
          }
          free(jobs_array[i] -> commands);
          free(jobs_array[i]);
          jobs_array[i] = NULL;
        }else{
          if (jobs_array[i] -> jobid > max_id && jobs_array[i] -> bg == 0){
            max_id = jobs_array[i] -> jobid;
          }
        }
      }
    }
    if(max_id){
      jobs_array[max_id-1] -> bg = 1;
    }


    
  }else{
    if(atoi(arg) == -1){
      printf("invallid id\n");
    }
    max_id = atoi(arg);
    jobs_array[max_id-1] -> bg = 1;
  }

  kill(jobs_array[max_id-1] -> pgid, SIGCONT);
  tcsetpgrp(0, getpgid(getpid()));
  return 0;
}

// put a process specific by id into the foreground
int fg(char * arg){
  
  // kill all zombie processes
  int max_id = -1;
  if(arg == NULL){
    for(int i = 0; i < MAXLEN; i++){
      if(jobs_array[i] != NULL){
        pid_t ret = waitpid(-(jobs_array[i] -> pgid), NULL, WNOHANG);

        if(ret == jobs_array[i] -> pgid || ret == -1){
          for(int j = 0; j < MAXLEN; j++){
            free(jobs_array[i]-> commands[j]);
          }
          free(jobs_array[i] -> commands);
          free(jobs_array[i]);
          jobs_array[i] = NULL;
        }else{
          if (jobs_array[i] -> jobid > max_id){
            max_id = jobs_array[i] -> jobid;
          }
        }
      }
    }
    
    if(max_id){
      jobs_array[max_id-1] -> bg = 0;
    }
    else{
      printf("invalid id\n");
      exit(0);
    }


    
  }else{
    if(atoi(arg) == -1){
      printf("invallid id\n");
    }
    max_id = atoi(arg);
    jobs_array[max_id-1] -> bg = 0;
  }

  tcsetpgrp(0, jobs_array[max_id-1] -> pgid);

  for(int n = 0; n < jobs_array[max_id-1] -> num_commands; n++){
    kill(-1 * jobs_array[max_id-1] -> pgid, SIGCONT);
    int status;
    waitpid(-1 * jobs_array[max_id-1] -> pgid, &status, WUNTRACED);
  }

  tcsetpgrp(0, getpgid(getpid()));

  return 0;
}


int
fork_pipe(int n, char ***cmd){
  int i;
  pid_t pid;
  int in, fd [2];

  // get input from standard in
  in = 0;


  // create a chain of pipes from the first child process to the last
  for (i = 0; i < n; i++){ 

    if (i < n-1){
      pipe (fd);
    }

    pid = fork();
    
    // Child process
    if (pid == 0){
        
      if (in != 0){
          dup2 (in, 0);
          close(in);
      }

      if(i < n - 1){
        dup2 (fd[1], 1);
        close(fd[1]);
      }

      return execvp (cmd[i][0], (char * const *)cmd[i]);
    
    }
    // else if(pid < 0){
    //   printf("Error in fork pipes\n");
    //   exit(0);
    // }
    // // make all child progress into one big group process
    // else{
    //   if(i == 0){
    //    jobs_array[job_index] -> pgid = pid;
    //    setpgid(pid, pid);
    //   }else{
    //     setpgid(pid, jobs_array[job_index] -> pgid);
    //   }
    // }

    // parent process
    // make all child processes into one big group
    else if (pid > 0){
    	if(i == 0){
       	jobs_array[job_index] -> pgid = pid;
       	setpgid(pid, pid);
      	}else{
        setpgid(pid, jobs_array[job_index] -> pgid);
      }

    }

    // error in forking
    else{
    	printf("Error in fork pipes\n");
    	exit(0);
    }

    // Redirect pipes
    if(i < n - 1){
      close (fd [1]);
      in = fd [0];

    }

  }

  // wait for all child processes to terminate    
  if(!jobs_array[job_index] -> bg){
    tcsetpgrp(0, jobs_array[job_index] -> pgid);
    for(int j = 0; j < n; j++){
      int status;
      waitpid(-1 * jobs_array[job_index]->pgid , &status, WUNTRACED);
    }
  }

  // give control back to the shell
  tcsetpgrp(0, getpgid(getpid()));
      
  return 0;
}

// check and execute built in commands
int built_in(char ***cmd){
  if(!strcmp(EXIT, cmd[0][0])){
    exit_function();
    return 1;
  }
  else if(!strcmp(CHDIR, cmd[0][0])){
    ch_dir(cmd[0][1]);
    return 1;
  }
  else if (!strcmp(JOBS, cmd[0][0])){
    jobs();
    return 1;
  }
  else if (!strcmp(FG, cmd[0][0])){
    fg(cmd[0][1]);
    return 1;
  }
  else if (!strcmp(BG, cmd[0][0])){
    bg(cmd[0][1]);
    return 1;
  }
  else{
    return 0;
  }
}

// parse through given shell input

int parse_command(char *line_input){

  // add and create job struct 
  struct jobs *job = malloc(sizeof(struct jobs));
  job -> commands = malloc(MAXLEN * sizeof(char**));
  for(int i = 0; i < 256; i++){
    job -> commands[i] = malloc(MAXLEN * sizeof(char*));
  }

  for(int i = 0; i < MAXLEN; i++){
    if(jobs_array[i] == NULL){
        job_index = i;
        break;
      }
    }

  jobs_array[job_index] = job;
  jobs_array[job_index] -> command_line = malloc((int)(strlen(line_input) * sizeof(char)));
  strcpy(jobs_array[job_index] -> command_line, line_input);

  char *ptr;
  char* token = strtok_r(line_input, "|", &ptr);
  int num_commands = 0;
  int i = 0;
  int j = 0;
  
  while(token){
    j = 0;
    int command_found = 0;
    jobs_array[job_index] -> bg = 0;
    char* cmd_ptr;
    char* command_token = strtok_r(token, " \t\n", &cmd_ptr);
    while(command_token){
            
      if (!command_found){
        jobs_array[job_index] -> commands[i][j] = command_token;
        command_found = 1;
        j++;

      } else if(strcmp(command_token, "&") == 0){
          jobs_array[job_index] -> bg = 1;
      }
            
      else{
        jobs_array[job_index] -> commands[i][j] = command_token;
        j++;
      }

        command_token = strtok_r(NULL, " \t\n", &cmd_ptr);
      }

      jobs_array[job_index] -> commands[i][j] = NULL;
      num_commands++;
      jobs_array[job_index] -> jobid = job_index + 1;
      i++;
      token = strtok_r(NULL, "|", &ptr);
    }

  jobs_array[job_index] -> num_commands = num_commands;

  // check if command give is built in or not
  if(!built_in(jobs_array[job_index] -> commands)){
    fork_pipe(num_commands, jobs_array[job_index] -> commands);
  }

  remove_old_jobs();

  return 0;
}

int main(int argc, char *argv[]){
  char* line;
  // signals
  signal(SIGINT, intsig_handler);
  signal(SIGTSTP, stopsig_handler);
  signal(SIGTTOU, SIG_IGN);
  setpgid(0,0);
  tcsetpgrp(0, getpgid(getpid()));

    
  // too many arguments
  if (argc > 2){
    printf("Error: Too many arguments\n");
      exit(1);
  }

  // interactive mode
  else if (argc == 1){
        
    while(1){
      printf("%s", PROMPT);
      size_t size; 
      if(getline(&line, &size, stdin)== -1){
        printf("Error reading line\n");
        exit(1);
      }
      line[(int)strlen(line) - 1] = '\0';
          
      if(strcmp(line,"") == 0){
        continue;
      }

      parse_command(line);
    }

        
  }
    // batch mode
  else{

    FILE *file_ptr = fopen(argv[1], "r");

    if (file_ptr == NULL){
        printf("Cannot open file \n");
        exit(1);
    }

    while(fgets(line, MAXLEN, file_ptr) !=NULL){
      line[(int)strlen(line) - 1] = '\0';
      parse_command(line);
    }
  
    fclose(file_ptr);

  }

   return 0;
}
