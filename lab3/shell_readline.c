#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define CLOSE       "\033[0m"           // close all
#define HIGHLIGHT   "\033[1m"           // highlight
#define BEGIN(x,y)  "\033["#x";"#y"m"   // x: background，y: foreground

#define MAX_LEN  1000
#define MAX_ARGV 100

void sigcat() {
    printf("%d Process continue\n", getpid());
}

typedef struct CMD {
    // char cmd[MAX_LEN];
    char *argv[MAX_ARGV];
    ssize_t len;
    int argc;
    int background;
    int redirect_input,redirect_output;
    char *inputFile,*outputFile;
    int create_pipe;
} COMMAND;

void initializeCmd(COMMAND *c) {
    // for(int i=0;i<MAX_LEN;i++) c->cmd[i]=0;
    for(int i=0;i<MAX_ARGV;i++) c->argv[i]=NULL;
    c->len=0; c->argc=0;
    c->background=0;
    c->redirect_input=0; c->redirect_output=0;
    c->inputFile=NULL; c->outputFile=NULL;
    c->create_pipe=0;
}

/* read a command */
int readCmd(COMMAND *c) {
    char *str = readline(BEGIN(49, 34)"tongyiheng@localhost:~$ "CLOSE); //font color:blue
    add_history(str);
    write_history("./shell_history");

    if (str == NULL) {
        return -1;
    }
    else {
        char *p=str;
        while (*p) { /* compute cmd length */
            c->len++;
            p++;
        }
        // printf("len=%ld\n",c->len);
        c->argv[0] = str;
        c->argc = 1;
        return 0;
    }
}

/* set a command manually*/
void setCmd(COMMAND *c,char *p) {
    c->argv[0]=p;
    c->argc = 1;
    return ;
}

/* analyse the command */
char* analyseCmd(COMMAND* c) {    
    char *p = c->argv[0];

    if (c->argv[0][c->len - 1] == '&') { // background
        c->background = 1;
        c->argv[0][c->len - 1] = '\0';
        c->len--;
        if (c->argv[0][c->len - 1] == ' ') {
            c->argv[0][c->len - 1] = '\0';
            c->len--;
        }
    }

    while (*p != '\0') {
        if (*p == ' ') {
            *p = '\0';
            p++;

            if (*p == '<') { // redirect input
                c->redirect_input = 1;
                p++;
                if (*p == ' ') {
                    *p = '\0';
                    p++;
                }
                c->inputFile = p;
                while (*p != ' ' && *p != '\0') p++;
            }
            else if (*p == '>') { // redirect output
                c->redirect_output = 1;
                p++;
                if (*p == ' ') {
                    *p = '\0';
                    p++;
                }
                c->outputFile = p;
                while (*p != ' ' && *p != '\0') p++;
            }
            else if (*p == '|') { // create pipe
                c->create_pipe = 1;
                p++;
                if (*p == ' ') {
                    *p = '\0';
                    p++;
                }
                break;
            }
            else {
                c->argv[c->argc] = p;
                c->argc++;
            }
        }
        else {
            p++;
        }
    }
    return p;
}

void executeCmd(COMMAND* c,char *p) {
    int status;
    pid_t pid = fork();

    if (pid < 0) { /* create child process failed */
        printf("Create child process failed!\n");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) { /* child process*/
        signal(SIGINT, SIG_DFL);
        umask(0000);
        if (c->redirect_output) {
            int fd = creat(c->outputFile, 0777);
            close(1);
            dup(fd);
            close(fd);
        }
        if (c->redirect_input) {
            int fd = open(c->inputFile, O_RDONLY, 0777);
            close(0);
            dup(fd);
            close(fd);
        }
        if (c->create_pipe == 1) {
            int pipe1[2];
            pipe(pipe1);

            pid_t pid1 = fork();

            if (pid1 == 0) { // deal with the left cmd of "|"
                close(1);
                dup(pipe1[1]);
                close(pipe1[1]);
                close(pipe1[0]);
                status = execvp(c->argv[0], c->argv);
                perror("error in pid1");
                exit(EXIT_FAILURE);
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

                COMMAND rightCmd;
                initializeCmd(&rightCmd);
                setCmd(&rightCmd,p);
                // readCmd(&rightCmd);
                char *rightPtr=analyseCmd(&rightCmd);
                // printf("DEBUG:%d\n",rightCmd.redirect_output);
                // printf("DEBUG:%s\n",rightCmd.outputFile);
                // exit(0);
                executeCmd(&rightCmd,rightPtr);
                // printf("execute finish\n");
                exit(EXIT_SUCCESS);
            }
        }
        else {
            status = execvp(c->argv[0], c->argv);
            perror("error");
            exit(EXIT_FAILURE);
        }
    }
    else { /* parent process */
        if (c->background == 0) { /* wait for child process */
            // printf("parent wait for child process\n");
            waitpid(pid, &status, 0);
            // printf("finish wait\n");
        }
        // printf("parent process finish\n");
    }
}

int main() {
    read_history("./shell_history");
    while(1) {
        signal(SIGINT,SIG_IGN);
        signal(SIGTSTP,SIG_DFL);    /* set other signal to "exit" */
        
        COMMAND newCmd;
        initializeCmd(&newCmd);
        if(readCmd(&newCmd) == -1) {
            // printf("c->len=%ld\n",len);
            // printf("c->cmd=%s\n",cmd);
            // perror("read failed");
            // exit(EXIT_FAILURE);
            continue;
        }
        else {
            char *ptr=analyseCmd(&newCmd);
            executeCmd(&newCmd,ptr);
            // printf("return to main\n");
        }
        free(newCmd.argv[0]);
    }
}