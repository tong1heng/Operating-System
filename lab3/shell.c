#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 100


void sigcat() {
    printf("%d Process continue\n", getpid());
}

int main() {
    int background=0;
    int redirect_input=0,redirect_output=0;
    char *inputFile,*outputFile;
    int create_pipe=0;

    pid_t pid;
    int status;
    char cmd[MAX_LINE]={0};
    ssize_t len;

    while(1) {
        signal(SIGINT,SIG_IGN);
        signal(SIGTSTP,SIG_DFL);    /* set other signal to "exit" */

        background=0;
        redirect_input=0,inputFile=NULL;
        redirect_output=0,outputFile=NULL;
        create_pipe=0;

        // printf("tyh@localhost:~$ ");
        // fflush(stdout);

        len=read(0,cmd,MAX_LINE);       /* read an instruction */
        if(len <= 0) {
            // printf("len=%ld\n",len);
            // printf("cmd=%s\n",cmd);
            // perror("read failed");
            exit(EXIT_FAILURE);
        }
        else { /* deal with LF */
            cmd[len-1]='\0';
            len--;
        }
        // printf("len=%ld\n",len);
        char *argv[100];
        for(int i=0;i<100;i++)
            argv[i]=NULL;
        argv[0]=cmd;
        int argc=1;
        char *p=cmd;
        
        /* analyse the command */

        if(cmd[len-1] == '&') { // background
            background=1;
            cmd[len-1]='\0';
            len--;
            if(cmd[len-1] == ' ') {
                cmd[len-1]='\0';
                len--;
            }
        }

        while(*p != '\0') {
            if(*p == ' ') {
                *p='\0';
                p++;

                if(*p == '<') { // redirect input
                    redirect_input=1;
                    p++;
                    if(*p == ' ') {*p='\0';p++;}
                    inputFile=p;
                    while(*p != ' ' && *p != '\0') {
                        p++;
                    }
                }
                else if(*p == '>') { // redirect output
                    redirect_output=1;
                    p++;
                    if(*p == ' ') {*p='\0';p++;}
                    outputFile=p;
                    while(*p != ' ' && *p != '\0') {
                        p++;
                    }
                }
                else if(*p == '|') { // create pipe
                    create_pipe=1;
                    p++;
                    if(*p == ' ') {*p='\0';p++;}
                    break;
                }
                else {
                    argv[argc]=p;
                    argc++;
                }
            }
            else {
                p++;
            }
        }

        pid = fork();
        if (pid < 0) { /* create child process failed */
            printf("Create child process failed!\n");
            exit(EXIT_FAILURE);
        }
        if(pid == 0) { /* child process*/
            signal(SIGINT,SIG_DFL);
            umask(0000); 
            if(redirect_output) {
                int fd=creat(outputFile,0777);
                close(1);
                dup(fd);
                close(fd);
            }
            if(redirect_input) {
                int fd=open(inputFile,O_RDONLY,0777);
                close(0);
                dup(fd);
                close(fd);
            }
            if(create_pipe == 1) {
                int pipe1[2];
                pipe(pipe1);

                pid_t pid1=fork();

                if(pid1 == 0) { // deal with the left of "|"
                    close(1);
                    dup(pipe1[1]);
                    close(pipe1[1]);
                    close(pipe1[0]);
                    status = execvp(argv[0],argv);
                    perror("error in pid1");
                }
                else {
                    waitpid(pid1, &status, 0);
                    close(0);
                    dup(pipe1[0]);
                    close(pipe1[0]);
                    close(pipe1[1]);

                    // char buffer[1000];
                    // read(0,buffer,sizeof(char)*1000);
                    // printf("DEBUG:%s\n",buffer);
                    // exit(0);

                    int redirect_input1=0,redirect_output1=0;
                    char *inputFile1=NULL,*outputFile1=NULL;

                    char *argv1[100];
                    for(int i=0;i<100;i++) {
                        argv1[i]=NULL;
                    }
                    argv1[0]=p;
                    int argc1=1;
                    while (*p != '\0') {
                        if (*p == ' ') {
                            *p = '\0';
                            p++;

                            if (*p == '<') { // redirect input
                                redirect_input1 = 1;
                                p++;
                                if (*p == ' ') {
                                    *p = '\0';
                                    p++;
                                }
                                inputFile1 = p;
                                while (*p != ' ' && *p != '\0') {
                                    p++;
                                }
                            }
                            else if (*p == '>') { // redirect output
                                redirect_output1 = 1;
                                p++;
                                if (*p == ' ') {
                                    *p = '\0';
                                    p++;
                                }
                                outputFile1 = p;
                                while (*p != ' ' && *p != '\0') {
                                    p++;
                                }
                                // printf("DEBUG:%d\n",redirect_output1);
                                // printf("DEBUG:%s\n",outputFile1);
                            }
                            // else if (*p == '|') { // create pipe
                            //     create_pipe = 1;
                            //     break;
                            // }
                            else {
                                argv1[argc1] = p;
                                argc1++;
                            }
                        }
                        else {
                            p++;
                        }
                    }
                    // printf("DEBUG--%s\n",argv1[0]);
                    // for(int i=0;i<100;i++)
                    //     printf("DEBUG--%s\n",argv1[i]);
                    if (redirect_output1)
                    {
                        // printf("Enter redirect output1\n");
                        int fd1 = creat(outputFile1, 0777);
                        // printf("finish create file\n");
                        close(1);
                        dup(fd1);
                        close(fd1);
                    }
                    if (redirect_input1)
                    {
                        int fd1 = open(inputFile1, O_RDONLY, 0777);
                        close(0);
                        dup(fd1);
                        close(fd1);
                    }

                    status = execvp(argv1[0],argv1);
                    perror("error in pipe");
                }
            }
            else {
                status = execvp(argv[0],argv);
                perror("error");
            }
        }
        else { /* parent process */
            if(background == 0) { /* wait for child process */
                waitpid(pid, &status, 0);
            }
            // printf("parent process finish\n");
        }
    }
}