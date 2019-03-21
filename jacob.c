#include "types.h"
#include "stat.h"
#include "user.h"

#define N 11
#define E 0.00001
#define T 100.0
#define P 4
#define L 20000

#define MSGSIZE 8

float fabsm(float a){
  if(a<0)
  return -1*a;
return a;
}

int main(int argc, char *argv[])
{
  printf(1, "[!] N: %d | [!] P: %d\n", N, P);
  // float diff;
  int i = 0;
  int j = 0;
  float mean;
  float u[N][N];
  // float w[N][N];
  printf(1, "[!] Ignore: %d %d\n", i, j);

  // int count=0;
  mean = 0.0;
  for (i = 0; i < N; i++){
    u[i][0] = u[i][N-1] = u[0][i] = T;
    u[N-1][i] = 0.0;
    mean += u[i][0] + u[i][N-1] + u[0][i] + u[N-1][i];
    // printf(1, "%d\n", (int)mean);
  }
  mean /= (4.0 * N);
  printf(1, "[!] Mean: %d\n", (int)mean);
  // print u[n][n] 
  // for(i=0; i<N; i++){
  //   for(j=0; j<N; j++){
  //     printf(1, "%d ", (int)u[i][j]);
  //   }
  //   printf(1, "\n");
  // }

  int p_pid = getpid();
  printf(1, "[*] Parent: %d\n", p_pid);
  int ratio = ((N-2)/P);
  // printf(1, "ratio: %d\n", ratio);
  int start, end;
  int x;
  for(x=0; x<P; x++){
    int cid1 = fork();
    if(cid1==0){
      printf(1, "Child no %d with ID %d\n", x, getpid());  
      // printf(1, "Mean: %d\n", (int)mean);

      start = x*ratio;
      end = (x+1)*ratio;
      if(x==0) {start=0;}
      if(x==P-1){end=N-2;}
      // sleep(x*2);
      printf(1, "start: %d | end: %d\n", start, end);

      float values[2+(end-start)][N];
      for(i=0; i<(end-start)+2; i++){
        for(j=0; j<N; j++){
          if(start==0){
            if(i==0){
              values[i][j]=100.0;
            }
            else{
              if((j==0) || (j==N-1)){
                values[i][j] = 100.0;
              }else{
                values[i][j] = mean;
              }
            }
          }
          else if(end==N-2){
            if(i==(end-start)+1){
              if(j==0){
                values[i][j] = 100.0;
              }else{
                values[i][j] = 0.0;
              }
            }
            else{
              if((j==0) || (j==N-1)){
                values[i][j] = 100.0;
              }else{
                values[i][j] = mean;
              }
            }
          }
          else{
            if((j==0) || (j==N-1)){
              values[i][j] = 100.0;
            }else{
              values[i][j] = mean;
            }
          }
          printf(1, "%d ", (int)values[i][j]);
        }
        printf(1, "\n");
      }

      // 5*getpid()+1; //up // read
      // 5*getpid()+2; //down // read
      // 5*getpid()+3; //up // write
      // 5*getpid()+4; //down // write

      // [fd-readend, fd-writeend]
      int pipe1[2];
      int pipe2[2];
      
      if(end==N-2){
        // last process does nothing (all pipes created)
      }else{
        pipe1[0] = 5*(getpid()+1)+1;
        pipe1[1] = 5*getpid()+4;
        pipe2[0] = 5*getpid()+2;
        pipe2[1] = 5*(getpid()+1)+3;
        printf(1, "%d %d\n", pipe1[0], pipe1[1]);
        printf(1, "%d %d\n", pipe2[0], pipe2[1]);
        if(pipe(pipe1) < 0){
          printf(1, "Error in Pipe creation\n");
          exit();
        }
        if(pipe(pipe2) < 0){
          printf(1, "Error in Pipe creation\n");
          exit();
        }
      }

      // send 
      for(j=1; j<N-1; j++){
        if(start==0){
        // dont send above first block
          // down
          write(5*getpid()+4, (char*)(&values[(end-start)][j]), MSGSIZE);
          printf(1, "%d\n", (int)values[(end-start)][j]);
        }else if(end==N-2){
        // don't send below last block
          // up 
          write(5*getpid()+3, (char*)(&values[1][j]), MSGSIZE);
          printf(1, "%d\n", (int)values[1][j]);
        }else{
        // send above and below
          // up 
          write(5*getpid()+3, (char*)(&values[1][j]), MSGSIZE);
          printf(1, "%d\n", (int)values[1][j]);
          // down
          write(5*getpid()+4, (char*)(&values[(end-start)][j]), MSGSIZE);          
          printf(1, "%d\n", (int)values[(end-start)][j]);
        }
      }
      // recieve
      for(j=1; j<N-1; j++){
        if(start==0){
        // no recieve above first block
          // down
          float msg=0;
          read(5*getpid()+2, (char*)(&msg), MSGSIZE);
          values[(end-start)+1][j] = msg;
          printf(1, "%d\n", (int)msg);
        }else if(end==N-2){
        // don't send below last block
          // up 
          float msg=0;
          read(5*getpid()+1, (char*)(&msg), MSGSIZE);
          values[0][j] = msg;
          printf(1, "%d\n", (int)msg);
        }else{
        // send above and below
          // up 
          float msg=0;
          read(5*getpid()+1, (char*)(&msg), MSGSIZE);
          values[0][j] = msg;
          printf(1, "%d\n", (int)msg);
          // down
          msg = 0;
          read(5*getpid()+2, (char*)(&msg), MSGSIZE);
          values[(end-start)+1][j] = msg;
          printf(1, "%d\n", (int)msg);
        }
      }
      exit();
    }
    wait();
  }
  printf(1, "[#] Is this the end?\n");
  exit();

  // int count=0;
  // mean = 0.0;
  // for (i = 0; i < N; i++){
  //   u[i][0] = u[i][N-1] = u[0][i] = T;
  //   u[N-1][i] = 0.0;
  //   mean += u[i][0] + u[i][N-1] + u[0][i] + u[N-1][i];
  // }
  // mean /= (4.0 * N);
  // for (i = 1; i < N-1; i++ )
  //   for ( j= 1; j < N-1; j++) u[i][j] = mean;
  // for(;;){
  //   diff = 0.0;
  //   for(i =1 ; i < N-1; i++){
  //     for(j =1 ; j < N-1; j++){
  //       w[i][j] = ( u[i-1][j] + u[i+1][j]+
  //             u[i][j-1] + u[i][j+1])/4.0;
  //       if( fabsm(w[i][j] - u[i][j]) > diff )
  //         diff = fabsm(w[i][j]- u[i][j]); 
  //     }
  //   }
  //     count++;
         
  //   if(diff<= E || count > L){ 
  //     break;
  //   }
  
  //   for (i =1; i< N-1; i++) 
  //     for (j =1; j< N-1; j++) u[i][j] = w[i][j];
  // }
  // for(i =0; i <N; i++){
  //   for(j = 0; j<N; j++)
  //     printf(1,"%d ",((int)u[i][j]));
  //   printf(1,"\n");
  // }

}
