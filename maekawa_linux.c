// #include "types.h"
// #include "stat.h"
// #include "user.h"

// #include <stdio.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <string.h> 
#include <sys/wait.h> 
#include <time.h> 


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
	printf("ID: %d | Type: %d | PID: %d\n", m.id, m.value, getpid());
}
int main(int argc, char *argv[])
{
	// -----
	int P = 16;
	int P1 = 0;
	int P2 = 16;
	int P3 = 0;
	// -----
	printf("[#] Vars: P->%d P1->%d P2->%d P3->%d\n", P, P1, P2, P3);
	
	int i,j;

	int Proot = floorSqrt(P);
	printf("[*] P-root: %d\n", Proot);

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
				printf("Pipe couldn't be created!\n");
				exit(0);
			}
		}
	}

	for(i=0; i<Proot; i++){
		for(j=0; j<Proot; j++){
			printf("%d ", Processes[i*Proot+j]);
			// printf("", Processes[i*Proot+j]);
		}
		printf("\n" );
	}

	int pIndex=0;
	struct myLockInfo lock;
	lock.value = UNLOCKED;
	lock.lockedByID = -1;
	for(i=1; i<Proot*Proot; i++){
		int cid=fork();
		if(cid==0){
			pIndex = i;
			break;
		}
	}
	// wait(0);
	// printf("%d %d\n", pIndex, getpid());
	// sleep(pIndex);
	// let's send request for every subset for P2+P3 processes
	if(Processes[pIndex] == 2 || Processes[pIndex] == 3){
		i = (int)(pIndex/Proot);
		j = pIndex % Proot;
		// printf("%d %d %d\n", i, j, pIndex);
		int count=0;
		// in its rows
		int k;
		for(k=0; k<Proot; k++){
			if(k!=j){
				struct msg m;
				m.value = REQUEST;
				m.id = pIndex;
				write(pipes[i*Proot + k][1], (&m), sizeof(struct msg));
				// printf("%d\n", i*Proot + k);
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
				// printf("%d\n", k*Proot + j);
			}
		}
		// sending to myself
		struct msg m;
		m.value = REQUEST;
		m.id = pIndex;
		write(pipes[pIndex][1], (&m), sizeof(struct msg));
		count++;
		// printf("%d\n", pIndex);

		// printf("[.] Sent %d REQUEST messages | Id: %d\n", count, pIndex);
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
	// printf("Got Failed from Count\n");
	// for(i=0; i<Proot; i++){
	// 	for(j=0; j<Proot; j++){
	// 		printf("%d ", failcount[i*Proot + j]);
	// 	}
	// 	printf("\n");
	// }
	// locked subset
	int lockedSubset[Proot*Proot];
	for(i=0; i<Proot; i++){
		for(j=0; j<Proot; j++){
			lockedSubset[i*Proot + j] = 0;
		}
	}
	// printf("Locked Subset\n");
	// for(i=0; i<Proot; i++){
	// 	for(j=0; j<Proot; j++){
	// 		printf("%d ", lockedSubset[i*Proot + j]);
	// 	}
	// 	printf("\n");
	// }
	// for(;;){}
	// if request message mark itself as locked if not already locked
	// and returns a locked reply
	struct msg reply;
	for(;;){
		read(pipes[pIndex][0], &reply, sizeof(struct msg));
		if(reply.value==REQUEST){
			printf("REQUEST by: %d | to: %d\n", reply.id, pIndex);
			if(lock.value==UNLOCKED){
				printf("already UNLOCKED: %d - LOCKED BY: %d\n", pIndex, reply.id);
				lock.value = LOCKED;
				lock.lockedByID = reply.id;

				struct msg imlocked;
				imlocked.id = pIndex;
				imlocked.value = lock.value;
				
				write(pipes[reply.id][1], &imlocked, sizeof(struct msg));
			}
			else if(lock.value==LOCKED){
				printf("already LOCKED %d by %d | checked by%d\n", pIndex, lock.lockedByID, reply.id);
				// other locking request precedes current
				int minIdInQueue = MAX_INT;
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
					printf("EnQueue | Id: %d | Me: %d\n", reply.id, pIndex);	
				}else{
					if(reply.id < lock.lockedByID){
						// current request in lower priority than locked by 
						// send inquire to whom I've already send LOCKED
						
						printf("INQUIRE by: %d | to %d | competition %d\n", pIndex, lock.lockedByID, reply.id);
						struct msg inquire;
						inquire.value = INQUIRE;
						inquire.id = pIndex;
						waitQueue[sizeOfQueue] = reply;
						sizeOfQueue++;
						write(pipes[lock.lockedByID][1], &inquire, sizeof(struct msg));
					}else{
						// current request is even higher priority then already locked
						// send failed
						struct msg failed;
						failed.value = FAILED;
						failed.id = pIndex;
						write(pipes[reply.id][1], &failed, sizeof(struct msg));
						// printf("ENQUEUE %d | SizeQ %d\n", reply.id, pIndex);
						waitQueue[sizeOfQueue] = reply;
						sizeOfQueue++;
						printf("EnQueue | Id: %d | Me: %d\n", reply.id, pIndex);	
					}
				}
			}
		}
		else if(reply.value==INQUIRE){
			// if you have failed till now then send relinquish
			printf("INQUIRE by: %d | to %d\n", reply.id, pIndex);

			// send a relinquish in any case:
			struct msg relinq;
			relinq.value = RELINQUISH;
			relinq.id = pIndex;
			write(pipes[reply.id][1], &relinq, sizeof(struct msg));
		

			// int isFailed=0;
			// for(i=0; i<Proot; i++){
			// 	for(j=0; j<Proot; j++){
			// 		if(failcount[i*Proot + j] > 0){
			// 			isFailed = 1;
			// 			goto outside;
			// 		}
			// 	}
			// }
			// outside:
			// if(isFailed){
			// 	struct msg relinq;
			// 	relinq.value = RELINQUISH;
			// 	relinq.id = pIndex;
			// 	write(pipes[reply.id][1], &relinq, sizeof(struct msg));
			// }
				
			// else ?
			// inquired - i used my lock
			// what should I send now?
			// defers a reply?
			// this means it will now recieve a release
		}
		else if(reply.value==FAILED){
			printf("FAILED by: %d | To: %d\n", reply.id, pIndex);

			// add in your fail count
			failcount[reply.id] = 1;
			lockedSubset[reply.id] = 0;
			// if fail count is max ?
 			// (2*Proot) - 2
			int failedCount=0;
			for(i=0; i<Proot; i++){
				for(j=0; j<Proot; j++){
					if(failcount[i*Proot + j] > 0){
						failedCount++;
					}
				}
			}

			// printf("fail count: %d\n", failedCount);

		}
		else if(reply.value==RELINQUISH){
			printf("RELINQUISH by: %d | To %d\n", reply.id, pIndex);

			if(reply.id != lock.lockedByID){
			}
			else{
				if(sizeOfQueue<=0){
					printf("There is no queue to remove from!\n");
				}
				else{
					int minIdInQueue = MAX_INT;
					for(i=0; i<sizeOfQueue; i++){
						if(waitQueue[i].id < minIdInQueue){
							minIdInQueue = waitQueue[i].id;
						}
					}
					if(minIdInQueue==MAX_INT){
						printf("what just happened!\n");
					}else{
						struct msg locked;
						locked.id = pIndex;
						locked.value=LOCKED;
						printf("Send to: %d | by %d\n", minIdInQueue, pIndex);
						write(pipes[minIdInQueue][1], &locked, sizeof(struct msg));
					}
				}
				// if relinquished then add it in our queue
				printf("EnQueue | Id: %d | Me: %d\n", reply.id, pIndex);	
				
			}
		}
		else if(reply.value==RELEASE){
			printf("RELEASE by: %d | to %d\n", reply.id, pIndex);

			// printf("release from %d with queuesize %d\n", reply.id, sizeOfQueue);
			// remove request from my waitqueue;
			for(i=0; i<sizeOfQueue; i++){
				if(waitQueue[i].id == reply.id){
					waitQueue[i].id = MAX_INT;
					printf("deQueue | Id: %d | Me: %d\n", reply.id, pIndex);	
				}
			}

			// send the most priority request a locked message
			int minIdInQueue = MAX_INT;
			for(i=0; i<sizeOfQueue; i++){
				if(waitQueue[i].id < minIdInQueue){
					minIdInQueue = waitQueue[i].id;
				}
			}
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
			}
		}
		else if(reply.value==LOCKED){
			printf("LOCK from: %d and I'm %d\n", reply.id, pIndex);
			lockedSubset[reply.id] = 1;

			// for(i=0; i<Proot; i++){
			// 	for(j=0; j<Proot; j++){
			// 		printf("%d ", lockedSubset[i*Proot + j]);
			// 	}
			// 	printf("\n");
			// }			

			// check if gotten all locks
			int countLocked=0;
			for(i=0; i<Proot; i++){
				for(j=0; j<Proot; j++){
					if(lockedSubset[i*Proot + j] > 0){
						countLocked++;
					}
				}
			}

			printf("count: %d\n", countLocked);

			if(countLocked==(2*(Proot-1)+1)){
				// got all the locks

				// Lock aqcuired
				printf("%d aqcuired the lock at time %d\n", pIndex, time(NULL)/CLOCKS_PER_SEC);

				if(Processes[pIndex]==2){
					sleep(2);
					// put sleep(2000) for linux
					// put sleep(200) for xv6
				}

				// Lock release
				printf("%d released the lock at time %d\n", pIndex, time(NULL)/CLOCKS_PER_SEC);
				int ki = (int)(pIndex/Proot);
				int kj = pIndex % Proot;
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
				// send release to self
				struct msg m;
				m.value = RELEASE;
				m.id = pIndex;
				write(pipes[pIndex][1], (&m), sizeof(struct msg));

				// remove this process from all wait queues;
				// release will do that

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
			// for(;;){}
		}
	}
	exit(0);
}
