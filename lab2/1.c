#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct VARIABLE {
    int x,y;
}VAR;

void task1(int *);  // compute f(x)
void task2(int *);  // compute f(y)
void task3(VAR *);  // compute f(x,y)=f(x)+f(y)
int pipe1[2],pipe2[2];
pthread_t thrd1,thrd2,thrd3;

int main(int argc,char *argv[]) {
    int ret;
    int x,y;
    VAR v;
    printf("Please input x and y:\n");
    printf("x=");
    scanf("%d",&x);
    printf("y=");
    scanf("%d",&y);
    v.x=x; v.y=y;

    if(pipe(pipe1) < 0) {
        perror("pipe1 not create");
        exit(EXIT_FAILURE);
    }
    if(pipe(pipe2) < 0) {
        perror("pipe2 not create");
        exit(EXIT_FAILURE);
    }

    ret = pthread_create(&thrd1,NULL,(void *)task1,(void *)&x);
    if(ret) {
        perror("pthread_create:task1");
        exit(EXIT_FAILURE);
    }

    ret = pthread_create(&thrd2,NULL,(void *)task2,(void *)&y);
    if(ret) {
        perror("pthread_create:task2");
        exit(EXIT_FAILURE);
    }

    ret = pthread_create(&thrd3,NULL,(void *)task3,(void *)&v);
    if(ret) {
        perror("pthread_create:task3");
        exit(EXIT_FAILURE);
    }

    pthread_join(thrd1,NULL);
    pthread_join(thrd2,NULL);
    pthread_join(thrd3,NULL);
    exit(EXIT_SUCCESS);
}

void task1(int *x) {
    int f[1000];
    f[1]=1;
    for(int i=2;i<=*x;i++) {
        f[i]=f[i-1]*i;
    }
    printf("f(x)=%d\n",f[*x]);
    write(pipe1[1],&f[*x],sizeof(int));
    close(pipe1[1]);
}

void task2(int *y) {
    int f[1000];
    f[1]=f[2]=1;
    for(int i=3;i<=*y;i++) {
        f[i]=f[i-1]+f[i-2];
    }
    printf("f(y)=%d\n",f[*y]);
    write(pipe2[1],&f[*y],sizeof(int));
    close(pipe2[1]);
}

void task3(VAR *v) {
    int fx,fy;
    read(pipe1[0],&fx,sizeof(int));
    read(pipe2[0],&fy,sizeof(int));
    printf("f(x,y)=%d\n",fx+fy);
}