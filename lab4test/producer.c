#include "ipc.h"

int main(int argc,char *argv[]) {
    int rate;
    if(argv[1]!=NULL) rate=atoi(argv[1]);
    else rate=3;

    buff_key=101;
    buff_num=1;
    pput_key=102;
    pput_num=1;
    shm_flg=IPC_CREAT | 0644;
    buff_ptr=(char *)set_shm(buff_key,buff_num,shm_flg);
    pput_ptr=(int *)set_shm(pput_key,pput_num,shm_flg);

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

    srand(time(NULL));

    while(1) {
        int order=rand()%3;
        down(prod_sem);
        // down(pmtx_sem);

        // buff_ptr[*pput_ptr]='A'+order;
        // sleep(rate);

        if(order==0) {
            printf("%d producer put tobacco and paper to Buffer[%d]\n",getpid(),*pput_ptr);
        }
        else if(order==1) {
            printf("%d producer put tobacco and matches to Buffer[%d]\n",getpid(),*pput_ptr);
        }
        else {
            printf("%d producer put paper and matches to Buffer[%d]\n",getpid(),*pput_ptr);
        }

        // *pput_ptr=(*pput_ptr+1)%buff_num;

        // up(pmtx_sem);

        if(order==0) {
            up(tobacco_paper_sem);
        }
        else if(order==1) {
            up(tobacco_matches_sem);
        }
        else {
            up(paper_matches_sem);
        }
    }
    return EXIT_SUCCESS;
}