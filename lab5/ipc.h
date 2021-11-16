#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>

#define BUFSZ	256
// #define MAXVAL	100
// #define STRSIZ	8
// #define WRITERQUEST		1	//写请求标识
// #define READERQUEST		2	//读请求标识
// #define FINISHED	    3	//读写完成标识

#define CHAIR  3
#define SOFA   4
#define ROOM   13

/* 信号量控制用的共同体 */
typedef union semuns {
    int val;
} Sem_uns;

/* 消息结构体 */
typedef struct msgbuf {
    long mtype;
    int	mid;
} Msg_buf;

key_t customer_key;
int customer_sem;
key_t account_key;
int account_sem;

int sem_val;
int sem_flg;

int wait_quest_flg;
key_t wait_quest_key;
int wait_quest_id;

int wait_respond_flg;
key_t wait_respond_key;
int wait_respond_id;

int sofa_quest_flg;
key_t sofa_quest_key;
int sofa_quest_id;

int sofa_respond_flg;
key_t sofa_respond_key;
int sofa_respond_id;


int get_ipc_id(char *proc_file,key_t key);

char *set_shm(key_t shm_key,int shm_num,int shm_flg);
int set_msq(key_t msq_key,int msq_flg);
int set_sem(key_t sem_key,int sem_val,int sem_flg);

int down(int sem_id);
int up(int sem_id);
