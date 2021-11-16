#include "ipc.h"

int main(int argc,char *argv[]) {
    int rate;
    Msg_buf msg_arg;

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

    int sofa_count=0;
    int wait_count=0;
    int i=0;

    // printf("Wait quest \n");
    while(1) {
        sleep(rate);
        i++;
        msg_arg.mid=i;
        msg_arg.mtype=msg_arg.mid;
        if(sofa_count < SOFA) {
            if(wait_count != 0) {
                wait_count--;
                msgrcv(wait_quest_id,&msg_arg,sizeof(msg_arg),0,0);
                msgsnd(wait_respond_id,&msg_arg,sizeof(msg_arg),0);
                printf("%d customer move from wait room to sofa\n",msg_arg.mid);
            }
            else {
                printf("%d customer sit on sofa\n",i);
            }
            
            sofa_quest_flg=IPC_NOWAIT;
            
            // msgsnd(sofa_quest_id,&msg_arg,sizeof(msg_arg),sofa_quest_flg);
            // printf("sit on sofa\n");

            // printf("msg_arg.mid=%d\n",msg_arg.mid);
            // printf("msg_arg.mtype=%ld\n",msg_arg.mtype);

            if((msgsnd(sofa_quest_id,&msg_arg,sizeof(msg_arg),sofa_quest_flg)) == 0) 
                printf("successfully send to quest queue\n");
            else 
                perror("fail to send");
            sofa_count++;
            // printf("sofa count=%d\n",sofa_count);
        }
        else if(wait_count < ROOM) {
            printf("sofa is full, %d customer wait in the waiting room\n",i);
            wait_quest_flg=IPC_NOWAIT;
            msgsnd(wait_quest_id,&msg_arg,sizeof(msg_arg),wait_quest_flg);
            wait_count++;
        }
        else {
            printf("waiting room is full, %d customer leave\n",i);
            // msgrcv(sofa_respond_id,&msg_arg,sizeof(msg_arg),0,0);
            // sofa_count--;
            // i--;
        }

        sofa_quest_id=IPC_NOWAIT;
        if(msgrcv(sofa_respond_id,&msg_arg,sizeof(msg_arg),0,sofa_quest_flg) >= 0) {
            sofa_count--;
        }
        wait_quest_flg=IPC_NOWAIT;
        if(msgrcv(wait_respond_id,&msg_arg,sizeof(msg_arg),0,wait_quest_flg) >= 0) {
            wait_count--;
        }

        // printf("wait count = %d\n",wait_count);
    }
    return EXIT_SUCCESS;
}