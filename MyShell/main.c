/* main.c
 * ----------------------------------------------------------
 *  CS350
 *  Midterm Programming Assignment
 *
 *  Purpose:  - Use Linux programming environment.
 *            - Review process creation and management
 * ----------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

const int MAX_CMD = 1000;
const int MAX_TOK = 100;

char error_message[30] = "An error has occurred\n";

void exec(char **parsed, int numTok, int bg) {

    //exit
    if (strcmp("exit", parsed[0]) == 0) {
        if (numTok != 1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        exit(0);
    }

    //cd
    else if (strcmp("cd", parsed[0]) == 0) {
        if (numTok == 1) {
            chdir(getenv("HOME"));
        }
        else if (chdir(parsed[1]) != 0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
    }
    
    //wait
    else if (strcmp("wait", parsed[0]) == 0) {
        while (waitpid(-1, NULL, 0) > 0) {}
    }

    //pwd
    else if (strcmp("pwd", parsed[0]) == 0) {
        if (numTok != 1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else {
            char dir[MAX_CMD];
            printf("%s\n", getcwd(dir,MAX_CMD));
        }
    }

    //help
    else if (strcmp("help", parsed[0]) == 0) {
        if (numTok != 1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        else {
            printf("cd\n");
            printf("pwd\n");
            printf("wait\n");
            printf("exit\n");
            printf("help\n");
        }
    }
    
    //forked commands
    else {
        pid_t pid = fork();

        switch(pid) {

            case -1:
                write(STDERR_FILENO, error_message, strlen(error_message));

            case 0:
                execvp(parsed[0], parsed);
                write(STDERR_FILENO, error_message, strlen(error_message));
                _exit(0);

            default:
                if (!bg) {
                    waitpid(pid, NULL, 0);
                }
        }
    }
}

int read_raw(char *raw, size_t *buffer, FILE *file) {
    return (getline(&raw, buffer, file));
}

int parse_input(char *raw, char **parsed) {
    char *delim = " \n";
    int numTok = 0;
    if ((parsed[0] = strtok(raw, delim)) != NULL) {
        int i = 1;
        for (; i < MAX_TOK; i++) {
            if ((parsed[i] = strtok(NULL,  delim) ) == NULL) {
                break;
            }
        }
        numTok = i;
    }
    return(numTok);
}

int main( int argc, char ** argv )
{

  FILE *file = stdin;
  int batch = 0;

  if (argc > 1) {
      if ((file = fopen(argv[1], "r")) == NULL) {
          write(STDERR_FILENO, error_message, strlen(error_message));
      }
      else {
          batch = 1;
      }
  }

  char raw[MAX_CMD];
  char *parsed[MAX_TOK];
  size_t buffer = MAX_CMD;
    
  if (!batch) {
    printf("> ");
  }
  while(read_raw(raw, &buffer, file) != -1) {

      int bg = 0;
      int numTok = parse_input(raw, parsed);
  
      if (numTok < 1) {
          if (!batch) {
            printf("> ");
          }
          continue;
      }
      
      if (strcmp("&", parsed[numTok-1]) == 0) {
          bg = 1;
          --numTok;
          parsed[numTok] = NULL;
      }
      
      exec(parsed, numTok, bg);

      if (!batch) {
        printf("> ");
      }

      fflush(stdout);
  }
  
  fclose(file);
  
  return 0;
}
