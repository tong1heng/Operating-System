#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE 100


void sigcat() {
    printf("%d Process continue\n", getpid());
}

int main() {
    int background=0;
    int redirect_input=0,inputFile=0;
    int redirect_output=0,outputFile=0;
    int create_pipe=0;

    pid_t pid;
    int status;
    char cmd[MAX_LINE]={0};
    ssize_t len;

    while(1) {
        signal(SIGINT,SIG_IGN);

        /* set other signal to "exit" */

        printf("tyh@localhost:~$ ");
        fflush(stdout);

        len=read(0,cmd,MAX_LINE);       /* read an instruction */
        if(len <= 0) {
            perror("read failed");
            exit(EXIT_FAILURE);
        }
        else { /* deal with LF */
            cmd[len-1]='\0';
            len--;
        }
        // printf("len=%ld\n",len);
        char* argv[100];
        for(int i=0;i<100;i++)
            argv[i]=NULL;
        argv[0]=cmd;
        int argc=1;
        char* p=cmd;
        /* analyse the command */
        while(*p != '\0') {
            if(*p == ' ') {
                *p='\0';
                p++;
                argv[argc]=p;
                argc++;
            }
            else {
                p++;
            }
        }

        if(cmd[len-1] == '&') {
            background=1;
        }
        for(int i=1;i<argc;i++) {
            if(argv[i] == "<") {
                redirect_input=1;
                inputFile=i+1;
            }
            if(argv[i] == ">") {
                redirect_output=1;
                outputFile=i+1;
            }
        }

        // printf("background=%d",background);
        // exit(0);

        pid = fork();
        if (pid < 0) { /* create child process failed */
            printf("Create child process failed!\n");
            exit(EXIT_FAILURE);
        }
        if(pid == 0) { /* child process*/
            signal(SIGINT,SIG_DFL);

            printf("%s\n",argv[0]);
            printf("%s\n",argv[1]);
            status = execvp(argv[0],argv);
            printf("%d\n",status);
            perror("error");
        }
        else { /* parent process */
            if(background == 0) { /* wait for child process */
                waitpid(pid, &status, 0);
            }
        }
    }
}