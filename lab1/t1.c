#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void sigcat() {
    printf("%d Process continue\n",getpid());
}

int main(int argc, char *argv[]) {
    int i;
    pid_t pid;
    int status,status2;
    char *args1[] = {"/bin/ls","-l",NULL};
    char *args2[] = {"/bin/ps","-l",NULL};

    signal(SIGINT,(__sighandler_t)sigcat);
    perror("SIGINT");

    pid=fork();
    if(pid < 0) { //建立子进程失败
        printf("Create child process1 failed!\n");
        exit(EXIT_FAILURE);
    }
    if(pid == 0) { //子进程代码段
        printf("I am child process1 %d\nMy father is %d\n",getpid(),getppid());
        pause();

        printf("%d Child1 will running:\n",getpid());
        for(i=0;args1[i]!=NULL;i++)
            printf("%s ",args1[i]);
        printf("\n");
        status=execve(args1[0],args1,NULL);
    }
    else { // 父进程代码段
        printf("\nI am parent process %d\n",getpid());

        pid_t pid2;
        pid2=fork();
        if(pid2 < 0) {
            printf("Create child process2 failded!\n");
            exit(EXIT_FAILURE);
        }
        if(pid2 == 0) {
            printf("I am child process2 %d\nMy father is %d\n",getpid(),getppid());
            printf("%d Child2 will running:\n",getpid());
            for(i=0;args2[i]!=NULL;i++)
                printf("%s ",args2[i]);
            printf("\n");
            status2=execve(args2[0],args2,NULL);
        }
        else {
            printf("%d Waiting for child2 done.\n\n",getpid());
            waitpid(pid2,&status2,0);
            printf("\nMy child2 exit! status=%d\n\n",status2);

            // wake up child process1
            kill(pid,SIGINT);
            printf("%d Waiting for child1 done.\n\n",getpid());
            waitpid(pid,&status,0);
            printf("\nMy child1 exit! status=%d\n\n",status);
        }
        
    }

    return EXIT_SUCCESS;
}