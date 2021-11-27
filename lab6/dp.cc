#include "dp.h"
using namespace std;

Sema::Sema(int id) {
    sem_id = id;
}

Sema::~Sema() {}

/*
* 信号灯上的 down/up 操作
* semid:信号灯数组标识符
* semnum:信号灯数组下标
* buf:操作信号灯的结构
*/
int Sema::down() {
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("down error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

int Sema::up() {
    Sem_uns arg;
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("up error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}

Lock::Lock(Sema * s) {
    sema = s;
}
Lock::~Lock() {}

//上锁
void Lock::close_lock() {
    sema->down();
}
//开锁
void Lock::open_lock() {
    sema->up();
}

Condition::Condition(Sema *s1, Sema *s2) {
    sema0 = s1;
    sema1 = s2;
}

void Condition::Wait(Lock *lock, int dir, int *cnt, int *curflow) {
    if (dir == 0) {
        lock->open_lock();
        *cnt += 1;
        sema0->down();
        *curflow += 1;
        lock->close_lock();
    }
    else if (dir == 1) {
        lock->open_lock();
        *cnt += 1;
        sema1->down();
        *curflow += 1;
        lock->close_lock();
    }
}

void Condition::Signal(int dir, int *cnt, int *onrail) {
    if (dir == 0) {
        sema0->up();
        *cnt -= 1;
        *onrail += 1;
    }
    else if (dir == 1) {
        sema1->up();
        *cnt -= 1;
        *onrail += 1;
    }
}

/*
    * get_ipc_id() 从/proc/sysvipc/文件系统中获取 IPC 的 id 号
    * pfile: 对应/proc/sysvipc/目录中的 IPC 文件分别为
    * msg-消息队列,sem-信号量,shm-共享内存
    * key: 对应要获取的 IPC 的 id 号的键值
*/
int Monitor::get_ipc_id(char *proc_file,key_t key)
{
	#define BUFSZ 256

	FILE *pf;
	int i,j;
	char line[BUFSZ],colum[BUFSZ];
	if((pf = fopen(proc_file,"r")) == NULL) {
		perror("Proc file not open");
		exit(EXIT_FAILURE);
	}

	fgets(line, BUFSZ,pf);
	while(!feof(pf)) {
		i = j = 0;

		fgets(line, BUFSZ,pf);
		while(line[i] == ' ') i++;
		while(line[i] !=' ') colum[j++] = line[i++];
		colum[j] = '\0';

		if(atoi(colum) != key) 
            continue;

		j=0;
		while(line[i] == ' ') i++;
		while(line[i] != ' ') colum[j++] = line[i++];
		colum[j] = '\0';

		i = atoi(colum);
		fclose(pf);
		return i;
	}
	fclose(pf);
	return -1;
}

/* 
    * set_sem 函数建立一个具有 n 个信号灯的信号量 
    * 如果建立成功，返回 一个信号量的标识符 sem_id 
    * 输入参数： 
        * sem_key 信号量的键值 
        * sem_val 信号量中信号灯的个数 
        * sem_flag 信号量的存取权限 
*/ 
int Monitor::set_sem(key_t sem_key, int sem_val, int sem_flg) {
    int sem_id; Sem_uns sem_arg;

    //测试由 sem_key 标识的信号量是否已经建立
    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0 ) {
        //semget 新建一个信号灯,其标号返回到sem_id 
        if ((sem_id = semget(sem_key, 1, sem_flg)) < 0) {
            perror("semaphore create error");
            exit(EXIT_FAILURE);
        }
    }

    //设置信号量的初值
    sem_arg.val = sem_val; 
    if (semctl(sem_id, 0, SETVAL, sem_arg) < 0) {
        perror("semaphore set error");
        exit(EXIT_FAILURE);
    } 
    return sem_id;
}

/* 
	* set_shm 函数建立一个具有 n 个字节 的共享内存区 
	* 如果建立成功，返回 一个指向该内存区首地址的指针 shm_buf 
	* 输入参数： 
	* shm_key 共享内存的键值 
	* shm_val 共享内存字节的长度 
	* shm_flag 共享内存的存取权限 
*/
char *Monitor::set_shm(key_t shm_key, int shm_num, int shm_flg) {

    int i, shm_id; 
    char *shm_buf;

    //测试由 shm_key 标识的共享内存区是否已经建立
    if ((shm_id = get_ipc_id("/proc/sysvipc/shm", shm_key)) < 0) { 
        //shmget 新建 一个长度为 shm_num 字节的共享内存
        if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0) {
            perror("shareMemory set error"); 
            exit(EXIT_FAILURE);
        } 
        //shmat 将由 shm_id 标识的共享内存附加给指针 shm_buf
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
            perror("get shareMemory error"); 
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < shm_num; i++) 
            shm_buf[i] = 0; //初始为 0

    } 
    //共享内存区已经建立,将由 shm_id 标识的共享内存附加给指针 shm_buf
    if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
        perror("get shareMemory error"); 
        exit(EXIT_FAILURE);
    } 
    return shm_buf;
}

Monitor::Monitor(int maxt, int maxf) {
    int ipc_flg = IPC_CREAT | 0644;

    if ((maxFlow = (int *) set_shm(100, 4, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    if ((curFlow = (int *) set_shm(101, 4, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    if ((onRail = (int *) set_shm(102, 4, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    if ((curDir = (int *) set_shm(103, 4, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }

    if ((cnt0 = (int *) set_shm(200, 4, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    if ((cnt1 = (int *) set_shm(201, 4, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    if ((waitcnt0 = (int *) set_shm(202, 4, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    if ((waitcnt1 = (int *) set_shm(203, 4, ipc_flg)) == NULL) {
        perror("Share memory create error");
        exit(EXIT_FAILURE);
    }
    
    Sema *sema0,*sema1;
    Sema *semaLock;
    int sema0_id,sema1_id,semaLock_id;

    if ((sema0_id = set_sem(301, 0, ipc_flg)) < 0) {
        perror("Semaphore create error");
        exit(EXIT_FAILURE);
    }
    if ((sema1_id = set_sem(302, 0, ipc_flg)) < 0) {
        perror("Semaphore create error");
        exit(EXIT_FAILURE);
    }
    if ((semaLock_id = set_sem(303, 1, ipc_flg)) < 0) {
        perror("Semaphore create error");
        exit(EXIT_FAILURE);
    }
    
    //初始化所有变量
    *maxFlow = maxf;
    *curFlow = 0;
    *onRail = 0;
    *curDir = 2;
    *cnt0 = 0;
    *cnt1 = 0;
    *waitcnt0 = 0;
    *waitcnt1 = 0;

    sema0 = new Sema(sema0_id);
    sema1 = new Sema(sema1_id);
    semaLock = new Sema(semaLock_id);
    lock = new Lock(semaLock);
    condition = new Condition(sema0, sema1);
}

void Monitor::Arrive(int dir) {
    lock->close_lock();

    cout << "No." << getpid() << " Arrive -- 当前允许方向: " << *curDir << " "
        << "正在等待: " << int(*waitcnt0) << "(A->B) " << int(*waitcnt1) << "(B->A)" << " "
        << "正在通过: " << int(*onRail) << " "
        << "单次流量: " << int(*curFlow) << "\n";
    
    if (*curDir == 2) {
        *curDir = dir;
        *onRail += 1;
        *curFlow += 1;
        lock->open_lock();
    }
    else if(*curDir != dir) {
        if(dir == 0)    condition->Wait(lock, dir,waitcnt0,curFlow);
        if(dir == 1)    condition->Wait(lock, dir,waitcnt1,curFlow);
        lock->open_lock();
    }
    else if(*curDir == dir) {
        if((*curFlow) < (*maxFlow)) {
            *onRail += 1;
            *curFlow += 1;
            lock->open_lock();
        }
        else{
            if(dir == 0)    condition->Wait(lock, dir,waitcnt0,curFlow);
            if(dir == 1)    condition->Wait(lock, dir,waitcnt1,curFlow);
            lock->open_lock();
        }
    }
}

void Monitor::Leave(int dir) {
    lock->close_lock();
    
    *onRail -= 1;
    if (dir == 0) *cnt0 += 1;
    if (dir == 1) *cnt1 += 1;

    cout << "No." << getpid() << " Leave -- 方向: " << dir << " 离开" << "\n";

    if(*onRail == 0) {
        cout << "重设允许通过方向: ";
        *curFlow = 0;

        if (dir == 0) {
            if (*waitcnt1 > 0) {
                *curDir = 1;
                cout << *curDir << "\n";
                cout << "即将唤醒列车数量: " << min(*waitcnt1,*maxFlow) << "\n";
                for(int i=0; i < min(*waitcnt1,*maxFlow); i++)
                    condition->Signal(1,waitcnt1,onRail);
            }
            else if (*waitcnt0 > 0) {
                *curDir = 0;
                cout << *curDir << "\n";
                cout << "即将唤醒列车数量: " << min(*waitcnt0,*maxFlow) << "\n";
                for(int i=0; i < min(*waitcnt0,*maxFlow); i++)
                    condition->Signal(0,waitcnt0,onRail);
            }
            else {
                *curDir = 2;
                cout << *curDir << "\n";
            }
        }
        else if (dir == 1) {
            if (*waitcnt0 > 0) {
                *curDir = 0;
                cout << *curDir << "\n";
                cout << "即将唤醒列车数量: " << min(*waitcnt0,*maxFlow) << "\n";
                for(int i=0; i < min(*waitcnt0,*maxFlow); i++)
                    condition->Signal(0,waitcnt0,onRail);
            }
            else if (*waitcnt1 > 0) {
                *curDir = 1;
                cout << *curDir << "\n";
                cout << "即将唤醒列车数量: " << min(*waitcnt1,*maxFlow) << "\n";
                for(int i=0; i < min(*waitcnt1,*maxFlow); i++)
                    condition->Signal(1,waitcnt1,onRail);
            }
            else {
                *curDir = 2;
                cout << *curDir << "\n";
            }
        }
        // cout << "等待队列: waitcnt0 = " << *waitcnt0 << " " << "waitcnt1 = " << *waitcnt1 << "\n";
    }
    lock->open_lock();
}

Monitor::~Monitor() {}

int main(int argc, char **argv) {
    int MAX_TRAIN;
    int MAX_FLOW;
    cout << "请输入需要通过的列车总数:";
    cin >> MAX_TRAIN;
    cout << "请输入单方向允许同时通过的最大流量:";
    cin >> MAX_FLOW;
    Monitor *mnt = new Monitor(MAX_TRAIN, MAX_FLOW);

    int i;
    int pid[MAX_TRAIN];
    for (i = 0; i < MAX_TRAIN; i++) {
        sleep(1);
        pid[i] = fork();
        if (pid[i] < 0) {
            perror("process create error");
        }
        if (pid[i] == 0) {
            srand(time(NULL));
            int direct = rand() % 2;    //random direction
            cout << "No." << getpid() << " direction = " << direct << "\n";

            mnt->Arrive(direct);

            cout << "No." << getpid() << " Pass -- 正在通过: " << *mnt->onRail << " "
                << "单次流量: " << *mnt->curFlow << "\n";
            sleep(5);

            mnt->Leave(direct);
            exit(EXIT_SUCCESS);
            // pause();
        }
    }
    for (i = 0; i < MAX_TRAIN; i++) {
        waitpid(pid[i], NULL, 0);
    }

    cout << "调度结束.\n";
    cout << *(mnt->cnt0) << "辆列车从A到B," << *(mnt->cnt1) << "辆列车从B到A.\n";
    return EXIT_SUCCESS;
}