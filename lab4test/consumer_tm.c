#include "ipc.h"

int main(int argc,char *argv[]) {
    int rate;
    if(argv[1]!=NULL) rate=atoi(argv[1]);
    else rate=3;

    buff_key=101;
    buff_num=1;
    cget_key=103;
    cget_num=1;
    shm_flg=IPC_CREAT | 0644;
    buff_ptr=(char *)set_shm(buff_key,buff_num,shm_flg);
    cget_ptr=(int *)set_shm(cget_key,cget_num,shm_flg);

    prod_key=201;
    pmtx_key=202;

    tobacco_paper_key=301;
    tobacco_matches_key=302;
    paper_matches_key=303;

    sem_flg=IPC_CREAT | 0644;

    sem_val=buff_num;
    prod_sem=set_sem(prod_key,sem_val,sem_flg);

    sem_val=0;
    tobacco_paper_sem=set_sem(tobacco_paper_key,sem_val,sem_flg);
    tobacco_matches_sem=set_sem(tobacco_matches_key,sem_val,sem_flg);
    paper_matches_sem=set_sem(paper_matches_key,sem_val,sem_flg);

    sem_val=1;
    pmtx_sem=set_sem(pmtx_key,sem_val,sem_flg);

    while(1) {
        down(tobacco_matches_sem);

        sleep(rate);
        printf("%d smoker get tobacco and matches from Buffer[%d]\n",getpid(),*cget_ptr);
        *cget_ptr=(*cget_ptr+1)%buff_num;

        up(prod_sem);
    }
    return EXIT_SUCCESS;
}