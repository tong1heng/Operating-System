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

    // quest_flg = IPC_CREAT| 0644;
    // quest_key = 205;
    // quest_id = set_msq(quest_key,quest_flg);

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
        printf("---------------begin---------------\n\n");
        sleep(rate);
        i++;    // customer id

        if(sofa_count < SOFA) { // sofa is not full, certain customer can move from waiting room to sofa
            while(msgrcv(wait_quest_id,&msg_arg,sizeof(msg_arg),0,IPC_NOWAIT) >= 0) {
                msgsnd(wait_respond_id,&msg_arg,sizeof(msg_arg),0);
                printf("%d customer move from waiting room to sofa\n\n",msg_arg.mid);
                msgsnd(sofa_quest_id,&msg_arg,sizeof(msg_arg),0);
                sofa_count++;
                if(sofa_count == SOFA) { // break if sofa is full
                    break;
                }
            }
        }

        // wait_quest_flg=IPC_NOWAIT;
        while(msgrcv(wait_respond_id,&msg_arg,sizeof(msg_arg),0,IPC_NOWAIT) >= 0) {
            wait_count--;
        }

        printf("After waiting room ---> sofa move, sofa count = %d\n",sofa_count);
        printf("After waiting room ---> sofa move, wait count = %d\n\n",wait_count);

        msg_arg.mid=i;
        msg_arg.mtype=msg_arg.mid;

        if(sofa_count < SOFA) { // sofa is not full
            // if(wait_count > 0) {
            //     // wait_count--;
            //     msgrcv(wait_quest_id,&msg_arg,sizeof(msg_arg),0,0);
            //     msgsnd(wait_respond_id,&msg_arg,sizeof(msg_arg),0);
            //     printf("%d customer move from wait room to sofa\n",msg_arg.mid);
            // }

            // if(msgrcv(wait_quest_id,&msg_arg,sizeof(msg_arg),0,IPC_NOWAIT) >= 0) {
            //     msgsnd(wait_respond_id,&msg_arg,sizeof(msg_arg),0);
            //     printf("%d customer move from wait room to sofa\n",msg_arg.mid);
            // }
            // else {
            //     printf("%d customer sit on sofa\n",i);
            // }
            
            // sofa_quest_flg=IPC_NOWAIT;
            
            // msgsnd(sofa_quest_id,&msg_arg,sizeof(msg_arg),sofa_quest_flg);
            // printf("sit on sofa\n");

            // printf("msg_arg.mid=%d\n",msg_arg.mid);
            // printf("msg_arg.mtype=%ld\n",msg_arg.mtype);

            // if((msgsnd(quest_id,&msg_arg,sizeof(msg_arg),0)) == 0) 
            //     printf("successfully send to quest queue\n");
            // else 
            //     perror("fail to send");
            msgsnd(sofa_quest_id,&msg_arg,sizeof(msg_arg),0);
            printf("%d customer sits on sofa\n\n",i);
            sofa_count++;
            // printf("sofa count=%d\n",sofa_count);
        }
        else if(wait_count < ROOM) { // waiting room is not full
            // wait_quest_flg=IPC_NOWAIT;
            msgsnd(wait_quest_id,&msg_arg,sizeof(msg_arg),0);
            printf("Sofa is full, %d customer waits in the waiting room\n\n",i);
            wait_count++;
        }
        else { // waiting room is full
            printf("Waiting room is full, %d customer leaves\n\n",i);
            // msgrcv(sofa_respond_id,&msg_arg,sizeof(msg_arg),0,0);
            // sofa_count--;
            // i--;
        }
        
        sleep(1); // make sure barber picks first

        // sofa_quest_id=IPC_NOWAIT;
        while(msgrcv(sofa_respond_id,&msg_arg,sizeof(msg_arg),0,IPC_NOWAIT) >= 0) {
            sofa_count--;
            printf("%d customer is having a haircut\n\n",msg_arg.mid);
        }

        printf("After certain customer finishing, sofa count = %d\n",sofa_count);
        printf("After certain customer finishing, wait count = %d\n\n",wait_count);
        printf("---------------end---------------\n\n\n");
    }
    return EXIT_SUCCESS;
}