#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>

#define BUFSZ 256

int get_ipc_id(char *proc_file,key_t key);

char* set_shm(key_t shm_key,int shm_num,int shm_flag);
int set_msq(key_t msq_key,int msq_flag);
int set_sem(key_t sem_key,int sem_val,int sem_flag);

int down(int sem_id);
int up(int sem_id);

typedef union semuns {
    int val;
}Sem_uns;

typedef struct megbuf {
    long mtype;
    char mtext[1];
}Msg_buf;

key_t buff_key;
int buff_num;
char *buff_ptr;

key_t pput_key;
int pput_num;
int *pput_ptr;

key_t cget_key;
int cget_num;
int *cget_ptr;

key_t pmtx_key;
int pmtx_sem;
key_t prod_key;
int prod_sem;

int sem_val;
int sem_flg;
int shm_flg;

key_t tobacco_paper_key;
int tobacco_paper_sem;
key_t tobacco_matches_key;
int tobacco_matches_sem;
key_t paper_matches_key;
int paper_matches_sem;
