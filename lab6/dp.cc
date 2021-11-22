#include "dp.h"

//分别是由东向西，由西向东的车辆数和单方向火车站同时最多能发的车辆数
int eastsum,westsum,d_maxcar;

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
	if((semop(sem_id,&buf,1)) < 0) {
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
	if((semop(sem_id,&buf,1)) < 0) {
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

//条件变量
Condition::Condition(char *st[] ,Sema *sm) {
	state = st;
	sema = sm;
}

void Condition::Wait(Lock *lock,int i) {    
	if((*state[i^1] == waiting)) {  //对面没发车
        *state[i] = running;        //发车
    }    
	else {
		lock->open_lock();
		sema->down();
		lock->close_lock();
	}
}

void Condition::Signal(int i) {
	if((*state[i^1] == waiting) && (*state[i] == waiting)) { //可发车
		sema->up();
		*state[i] = running;
	}
}

/*
    * get_ipc_id() 从/proc/sysvipc/文件系统中获取 IPC 的 id 号
    * pfile: 对应/proc/sysvipc/目录中的 IPC 文件分别为
    * msg-消息队列,sem-信号量,shm-共享内存
    * key: 对应要获取的 IPC 的 id 号的键值
*/
int dp::get_ipc_id(char *proc_file,key_t key)
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
int dp::set_sem(key_t sem_key, int sem_val, int sem_flg) {
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
char *dp::set_shm(key_t shm_key, int shm_num, int shm_flg) {

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

dp::dp(int r,int maxcur) {
	int ipc_flg = IPC_CREAT | 0644;
	int shm_key = 220;
	int shm_num = 1;
	int sem_key = 120;
	int sem_val = 0;
	int sem_id;

	Sema *sema;
	rate = r;

	//创建共享内存
	maxcars = (int *) set_shm(shm_key++, 1, ipc_flg);
	nowcars = (int *) set_shm(shm_key++, 1, ipc_flg);
	sumeast = (int *) set_shm(shm_key++, 1, ipc_flg);
	sumwest = (int *) set_shm(shm_key++, 1, ipc_flg);
	//初始化
	*maxcars = maxcur;
	*nowcars = 0;
	*sumeast = 0;
	*sumwest = 0;
	//创建管程中的锁和条件变量
	if((sem_id = set_sem(sem_key++,1,ipc_flg)) < 0) {
		perror("Semaphore create error");
		exit(EXIT_FAILURE);
	}
	sema = new Sema(sem_id);
	lock = new Lock(sema);
	for(int i=0; i<2; i++) {
		if((state[i] = (char *)set_shm(shm_key++,shm_num,ipc_flg)) == NULL) {
			perror("Share memory create error");
			exit(EXIT_FAILURE);
		}
		*state[i] = waiting;
		if((sem_id = set_sem(sem_key++,sem_val,ipc_flg)) < 0) {
			perror("Semaphore create error");
			exit(EXIT_FAILURE);
		}
		sema = new Sema(sem_id);
		self[i] = new Condition(state,sema);
	}
}

void dp::start(int i) {
	lock->close_lock();
	//测试能否发车
	self[i]->Wait(lock,i);

	if(i == 0) {
		*sumeast = *sumeast+1;
		cout << "火车站" << getpid() << "发送火车由东向西"<<*nowcars<<"\n";
	} 
	else if(i == 1) {
		*sumwest=*sumwest+1;
		cout << "火车站" << getpid() << "发送火车由西向东"<<*nowcars<<"\n";
	}
	
	*nowcars = *nowcars+1;
	cnt[i] = cnt[i]+1;
	//模拟发送火车的时间
	sleep(rate);
	lock->open_lock();
}

//火车正常通过，离开
void dp::leave(int i) {
	lock->close_lock();
	if(i == 0) {
		if((cnt[0] == d_maxcar || *sumeast==eastsum)) {
			if(*sumwest < westsum) {
				cnt[0] = 0;
				*state[0] = waiting;
				self[1]->Signal(1);
				if(*sumeast < eastsum) cout << "火车站"<< getpid() << "等待\n";
				else cout << "火车站"<< getpid() << "结束发车\n";
			}
			else if(*sumeast < eastsum) {
				cout << "火车站"<< getpid() << "继续发车\n";
			}
			else {
				cout << "火车站"<< getpid() << "结束发车\n";
			}
		}
		else {
			cout << "火车站"<< getpid() << "继续发车\n";
		}
	}
	else if(i == 1)
	{
		if((cnt[1] == d_maxcar || *sumwest == westsum)) {
			if(*sumeast < eastsum) {
				cnt[1] = 0;
				*state[1] =waiting;
				self[0]->Signal(0);
				if(*sumwest<westsum) cout << "火车站"<< getpid() << "等待\n";
				else cout << "火车站"<< getpid() << "结束发车\n";
			}
			else if(*sumwest < westsum) {
				cout << "火车站" << getpid() << "继续发车\n";
			}
			else {
				cout << "火车站" << getpid() << "结束发车\n";
			}
		}
		else {
			cout << "火车站" << getpid() << "继续发车\n";
		}

	}
	lock->open_lock();
}
dp::~dp() {}



int main(int argc,char *argv[]) {
	int maxcur;
	cout << "请输入火车的数量：";
	cin >> maxcur;
	cout << "请输入单方向同时最多发送的火车数";
	cin >> d_maxcar;

	dp *tdp;
	int pid[2];
	int rate ;
	rate = (argc > 1) ? atoi(argv[1]) : 2 ;
	tdp = new dp(rate,maxcur);

	for(int i=0;i<maxcur;i++) {
		if(rand()%2==0) eastsum++;
		else westsum++;
	}
	cout<<eastsum<<' '<<westsum<<endl;

	for(int i=0;i<=1;i++) {
		pid[i]=fork();
		if(pid[i]==0){
			while(1) {
				//cout<<*tdp->sumeast<<" "<<*tdp->sumwest<<endl;
				tdp->start(i);
				tdp->leave(i);
				if(i==0 && *tdp->sumeast==eastsum) break;
				else if(i==1 && *tdp->sumwest==westsum) break;
			}
			exit(0);
		}
	}
	for(int i=0;i<2;i++) {
		waitpid(pid[i],NULL,0);
	}
	return 0;
}