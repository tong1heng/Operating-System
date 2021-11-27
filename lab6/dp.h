#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>

/*信号灯控制用的共同体*/
typedef union semuns{
    int val;
}Sem_uns;

//管程中使用的信号量
class Sema {
public:
    Sema(int id);
    ~Sema();
    int down();         //信号量加 1
    int up();           //信号量减 1
private:
    int sem_id;         //信号量标识符
};

//管程中使用的锁
class Lock {
public:
    Lock(Sema *lock);
    ~Lock();
    void close_lock();
    void open_lock();
private:
    Sema *sema;         //锁使用的信号量
};

class Condition {
public:
    Condition(Sema *s1, Sema *s2);
    ~Condition();
    void Wait(Lock *Lock, int dir, int *cnt, int *curflow);     //过路条件不足时阻塞
    void Signal(int dir, int *cnt, int *onrail);                //唤醒阻塞的列车
private:
    Sema* sema0;        // 一个方向的阻塞队列
    Sema* sema1;        // 另一方向的阻塞队列
    Lock* lock;         // 进入管程时获取的锁
};

class Monitor {
public:
    Monitor(int maxt, int maxf);
    ~Monitor();
    void Arrive(int dir);       //列车准备上铁路
    void Leave(int dir);        //列车通过了铁路

    int *cnt0,*cnt1;            //两个方向通过的列车数量
    int *waitcnt0,*waitcnt1;    //两个方向的阻塞队列长度
    int *curFlow;               //当前单次流量
    int *curDir;                //当前允许通行的方向
    int *onRail;                //当前正在通过的列车数
private:
    //建立或获取 ipc 信号量的一组函数的原型说明
    int get_ipc_id (char *proc_file, key_t key);
    int set_sem(key_t sem_key, int sem_val, int sem_flag);
    char *set_shm(key_t shm_key, int shm_num, int shm_flag);

    int *maxFlow;               //单次允许的最大流量
    Condition *condition;       //条件变量
    Lock *lock;                 //管程锁
};