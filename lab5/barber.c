#include "ipc.h"

int main(int argc,char *argv[]) {
    int rate;
    Msg_buf	msg_arg;

    //可在在命令行第一参数指定一个进程睡眠秒数，以调解进程执行速度
    if(argv[1] != NULL)	rate = atoi(argv[1]);
    else rate = 3;

    //联系一个请求消息队列
    wait_quest_flg = IPC_CREAT| 0644;
    wait_quest_key = 201;
    wait_quest_id = set_msq(wait_quest_key,wait_quest_flg);

    //联系一个响应消息队列
    wait_respond_flg = IPC_CREAT| 0644;
    wait_respond_key = 202;
    wait_respond_id = set_msq(wait_respond_key,wait_respond_flg);

    //联系一个请求消息队列
    sofa_quest_flg = IPC_CREAT| 0644;
    sofa_quest_key = 203;
    sofa_quest_id = set_msq(sofa_quest_key,sofa_quest_flg);

    //联系一个响应消息队列
    sofa_respond_flg = IPC_CREAT| 0644;
    sofa_respond_key = 204;
    sofa_respond_id = set_msq(sofa_respond_key,sofa_respond_flg);

    //semaphore
    customer_key = 301;
    account_key = 302;
    sem_flg = IPC_CREAT | 0644;
    
    sem_val = 0;
    customer_sem = set_sem(customer_key,sem_val,sem_flg);

    sem_val = 1;
    account_sem = set_sem(account_key,sem_val,sem_flg);

    pid_t pid1,pid2;
    pid1 = fork();
    if(pid1 == 0) {
        while(1) {
            printf("%d barber sleep\n",getpid());
            // wait_quest_flg = 0;
            // printf("before receive\n");
            if(msgrcv(sofa_quest_id,&msg_arg,sizeof(msg_arg),0,0) >= 0) {
                // read sofa quest queue
                // printf("in ddd\n");
                msgsnd(sofa_respond_id,&msg_arg,sizeof(msg_arg),0);
                printf("%d barber cut for %d customer\n",getpid(),msg_arg.mid);
                sleep(rate);
                down(account_sem);
                printf("%d barber get paid from %d customer\n",getpid(),msg_arg.mid);
                up(account_sem);
            }
        }
    }
    else {
        pid2 = fork();
        if(pid2 == 0) {
            while(1) {
                printf("%d barber sleep\n",getpid());
                // wait_quest_flg = 0;
                if(msgrcv(sofa_quest_id,&msg_arg,sizeof(msg_arg),0,0) >= 0) {
                    // read sofa quest queue
                    msgsnd(sofa_respond_id,&msg_arg,sizeof(msg_arg),0);
                    printf("%d barber cut for %d customer\n",getpid(),msg_arg.mid);
                    sleep(rate);
                    down(account_sem);
                    printf("%d barber get paid from %d customer\n",getpid(),msg_arg.mid);
                    up(account_sem);
                }
            }
        }
        else {
            while(1) {
                printf("%d barber sleep\n",getpid());
                // wait_quest_flg = 0;
                if(msgrcv(sofa_quest_id,&msg_arg,sizeof(msg_arg),0,0) >= 0) {
                    // read sofa quest queue
                    msgsnd(sofa_respond_id,&msg_arg,sizeof(msg_arg),0);
                    printf("%d barber cut for %d customer\n",getpid(),msg_arg.mid);
                    sleep(rate);
                    down(account_sem);
                    printf("%d barber get paid from %d customer\n",getpid(),msg_arg.mid);
                    up(account_sem);
                }
            }
        }
    }
    return EXIT_SUCCESS;
}