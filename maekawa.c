#include "types.h"
#include "stat.h"
#include "user.h"

#define MSGSIZE 4
#define MAX_INT 2147483646

float fabsm(float a){
  if(a<0) return -1*a;
  return a;
}

// returns floor of square root of x 
int floorSqrt(int x) { 
    if (x == 0 || x == 1) 
    return x; 
    int i = 1, result = 1; 
    while (result <= x) { 
      i++; 
      result = i * i; 
    } 
    return i - 1; 
} 

enum enquiry{
	REQUEST = 0,
	LOCKED = 1,
	INQUIRE = 2,
	RELEASE = 3,
	RELINQUISH = 4,
	UNLOCKED = 5,
	FAILED = 6
	};

struct msg{	
	int id;
	int value;
};
  
struct myLockInfo{	
	int lockedByID;
	int value;
};
  
void printMessage(struct msg m){
	printf(1, "ID: %d | Type: %d | PID: %d\n", m.id, m.value, getpid());
}
int main(int argc, char *argv[])
{
	// -----
	int P = 16;
	int P1 = 8;
	int P2 = 5;
	int P3 = 3;
	// -----
	printf(1, "[#] Vars: P->%d P1->%d P2->%d P3->%d\n", P, P1, P2, P3);
	
	int i,j;

	int Proot = floorSqrt(P);
	printf(1, "[*] P-root: %d\n", Proot);

	struct msg waitQueue[2*Proot*Proot];
	int sizeOfQueue=0;


	// fill the process types
	int Processes[Proot*Proot];
	int tmpCount = 0;
	for(i=0; i<Proot; i++){
		for(j=0; j<Proot; j++){
			tmpCount++;
			if(tmpCount<=P1){
				Processes[i*Proot + j] = 1;
			}else if(tmpCount<=P2+P1){
				Processes[i*Proot + j] = 2;
			}else{
				Processes[i*Proot + j] = 3;
			}
		}
	}

	int pipes[Proot*Proot][2];
	for(i=0; i<Proot; i++){
		for(j=0; j<Proot; j++){
			if(pipe(pipes[i*Proot+j]) < 0){
				printf(1, "Pipe couldn't be created!\n");
				exit();
			}
		}
	}

	for(i=0; i<Proot; i++){
		for(j=0; j<Proot; j++){
			printf(1, "%d ", Processes[i*Proot+j]);
			// printf(1, "", Processes[i*Proot+j]);
		}
		printf(1, "\n" );
	}

	int pIndex=0;
	struct myLockInfo lock;
	lock.value = UNLOCKED;
	lock.lockedByID = 0;
	for(i=1; i<Proot*Proot; i++){
		int cid=fork();
		if(cid==0){
			pIndex = i;
			break;
		}
	}

	// printf(1, "%d %d\n", pIndex, getpid());

	// let's send request for every subset for P2+P3 processes
	if(Processes[pIndex] == 2 || Processes[pIndex] == 3){
		i = (int)(pIndex/4);
		j = pIndex % 4;
		// printf(1, "%d %d %d\n", i, j, pIndex);
		int count=0;
		// in its rows
		int k;
		for(k=0; k<Proot; k++){
			if(k!=j){
				struct msg m;
				m.value = REQUEST;
				m.id = pIndex;
				write(pipes[i*Proot + k][1], (&m), sizeof(struct msg));
				count++;
			}
		}
		// in its column
		for(k=0; k<Proot; k++){
			if(k!=i){
				struct msg m;
				m.value = REQUEST;
				m.id = pIndex;
				write(pipes[k*Proot + j][1], (&m), sizeof(struct msg));
				count++;
			}
		}
		printf(1, "[.] Sent %d REQUEST messages | Id: %d\n", count, pIndex);
	}

	// permission from itself without message transmission	}
	// requests sent

	// fail count
	int failcount[Proot*Proot];
	for(i=0; i<Proot; i++){
		for(j=0; j<Proot; j++){
			failcount[i*Proot + j] = 0;
		}
	}
	printf(1, "Got Failed from Count\n");
	for(i=0; i<Proot; i++){
		for(j=0; j<Proot; j++){
			printf(1, "%d ", failcount[i*Proot + j]);
		}
		printf(1, "\n");
	}
	// locked subset
	int lockedSubset[Proot*Proot];
	for(i=0; i<Proot; i++){
		for(j=0; j<Proot; j++){
			lockedSubset[i*Proot + j] = 0;
		}
	}
	printf(1, "Locked Subset\n");
	for(i=0; i<Proot; i++){
		for(j=0; j<Proot; j++){
			printf(1, "%d ", lockedSubset[i*Proot + j]);
		}
		printf(1, "\n");
	}
	for(;;){}
	// if request message mark itself as locked if not already locked
	// and returns a locked reply
	struct msg reply;
	for(;;){
		read(pipes[pIndex][0], &reply, sizeof(struct msg));
		if(reply.value==REQUEST){
			if(lock.value==UNLOCKED){
				lock.value = LOCKED;
				lock.lockedByID = reply.id;

				struct msg imlocked;
				imlocked.id = pIndex;
				imlocked.value = lock.value;
				
				write(pipes[reply.id][1], &imlocked, sizeof(struct msg));
				// printf(1, "%d\n", pIndex);
			}
			else if(reply.value==LOCKED){
				// other locking request precedes current
				int minIdInQueue = MAX_INT;
				if(sizeOfQueue<=0){
					printf(1, "[*] Case didn't thought\n");
					// why is the size zero if I'm locked; Who locked me, must have been in my queue
				}
				for(i=0; i<sizeOfQueue; i++){
					if(waitQueue[i].id < minIdInQueue){
						minIdInQueue = waitQueue[i].id;
					}
				}
				if(minIdInQueue < reply.id){
					// send failed
					struct msg failed;
					failed.value = FAILED;
					failed.id = pIndex;
					write(pipes[reply.id][1], &failed, sizeof(struct msg));
					waitQueue[sizeOfQueue] = reply;
					sizeOfQueue++;	
				}else{
					if(reply.id < lock.lockedByID){
						// current request in lower priority than locked by 
						// send inquire to whom I've already send LOCKED
						struct msg inquire;
						inquire.value = INQUIRE;
						inquire.id = pIndex;
						write(pipes[lock.lockedByID][1], &inquire, sizeof(struct msg));
					}else{
						// current request is even higher priority then already locked
						// send failed
						struct msg failed;
						failed.value = FAILED;
						failed.id = pIndex;
						write(pipes[reply.id][1], &failed, sizeof(struct msg));
						waitQueue[sizeOfQueue] = reply;
						sizeOfQueue++;
					}
				}
			}
		}
		else if(reply.value==INQUIRE){
			// if you have failed till now then send relinquish
			int isFailed=0;
			for(i=0; i<Proot; i++){
				for(j=0; j<Proot; j++){
					if(failcount[i*Proot + j] > 0){
						isFailed = 1;
						goto outside;
					}
				}
			}
			outside:
			if(isFailed){
				struct msg relinq;
				relinq.value = RELINQUISH;
				relinq.id = pIndex;
				write(pipes[reply.id][1], &relinq, sizeof(struct msg));
			}
				
			// else ?
			// inquired - i used my lock
			// what should I send now?
			// defers a reply?
			// this means it will now recieve a release
		}
		else if(reply.value==FAILED){
			// add in your fail count
			failcount[reply.id] = 1;
			lockedSubset[reply.id] = 0;
			// if fail count is max ?
 			// (2*Proot) - 2
		}
		else if(reply.value==RELINQUISH){
			if(sizeOfQueue<=0){
				printf(1, "There is no queue to remove from!\n");
			}
			else{
				int minIdInQueue = MAX_INT;
				for(i=0; i<sizeOfQueue; i++){
					if(waitQueue[i].id < minIdInQueue){
						minIdInQueue = waitQueue[i].id;
					}
				}
				if(minIdInQueue==MAX_INT){
					printf(1, "what just happened!\n");
				}else{
					struct msg locked;
					locked.id = pIndex;
					locked.value=LOCKED;
					write(pipes[minIdInQueue][1], &locked, sizeof(struct msg));
				}
			}
			// if relinquished then add it in our queue
			waitQueue[sizeOfQueue] == reply.id;
			sizeOfQueue++;
		}
		else if(reply.value==RELEASE){
			// remove request from my waitqueue;
			for(i=0; i<sizeOfQueue; i++){
				if(waitQueue[i].id == reply.id){
					waitQueue[i].id = MAX_INT;
				}
			}

			// send the most priority request a locked message
			if((sizeOfQueue<=0) || (minIdInQueue == MAX_INT)){
				// mark status to unlocked
				lock.value=UNLOCKED;
				lock.lockedByID=0;
			}
			else{
				
				struct msg locked;
				locked.id = pIndex;
				locked.value=LOCKED;
				write(pipes[minIdInQueue][1], &locked, sizeof(struct msg));
				int minIdInQueue = MAX_INT;
				for(i=0; i<sizeOfQueue; i++){
					if(waitQueue[i].id < minIdInQueue){
						minIdInQueue = waitQueue[i].id;
					}
				}
			}
		}
		else if(reply.value==LOCKED){
			lockedSubset[reply.id] = 1;

			// check if gotten all locks
			int countLocked=0;
			for(i=0; i<Proot; i++){
				for(j=0; j<Proot; j++){
					if(lockedSubset[i*Proot + j] > 0){
						countLocked++;
					}
				}
			}

			if(countLocked==(2*(Proot-1))){
				// got all the locks

				// Lock aqcuired
				printf(1, "%d aqcuired the lock at time %d\n", getpid(), uptime());

				if(Processes[pIndex]==2){
					sleep(2);
				}

				// Lock release
				printf(1, "%d released the lock at time %d\n", getpid(), uptime());
				int ki = (int)(pIndex/4);
				int kj = pIndex % 4;
				// in its rows
				for(i=0; i<Proot; i++){
					if(i!=kj){
						struct msg m;
						m.value = RELEASE;
						m.id = pIndex;
						write(pipes[ki*Proot + i][1], (&m), sizeof(struct msg));
					}
				}
				// in its column
				for(i=0; i<Proot; i++){
					if(i!=ki){
						struct msg m;
						m.value = RELEASE;
						m.id = pIndex;
						write(pipes[i*Proot + kj][1], (&m), sizeof(struct msg));
					}
				}
				// remove this process from all wait queues;
			}
			// clear my failcount;
			for(i=0; i<Proot; i++){
				for(j=0; j<Proot; j++){
					failcount[i*Proot + j] = 0;
				}
			}
			// clear my locked count;
			for(i=0; i<Proot; i++){
				for(j=0; j<Proot; j++){
					lockedSubset[i*Proot + j] = 0;
				}
			}
		}
	}
	exit();
}
