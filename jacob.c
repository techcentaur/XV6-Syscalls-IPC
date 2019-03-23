#include "types.h"
#include "stat.h"
#include "user.h"

// #include <stdio.h> 
// #include <stdlib.h> 
// #include <unistd.h> 
// #include <sys/types.h> 
// #include <string.h> 
// #include <sys/wait.h> 

#define N 20
#define E 0.00001
#define T 100.0
#define P 10
#define L 20000

#define MSGSIZE 4

float fabsm(float a){
  if(a<0)
  return -1*a;
return a;
}

int main(int argc, char *argv[])
{
  printf(1, "[!] N: %d | [!] P: %d\n", N, P);
  int i = 0;
  int j = 0;
  float mean;
  float u[N][N];
  printf(1, "[!] Ignore: %d %d\n", i, j);

  mean = 0.0;
  for (i = 0; i < N; i++){
    u[i][0] = u[i][N-1] = u[0][i] = T;
    u[N-1][i] = 0.0;
    mean += u[i][0] + u[i][N-1] + u[0][i] + u[N-1][i];
  }
  mean /= (4.0 * N);
  printf(1, "[!] Mean: %d\n", (int)mean);

  int p_pid = getpid();
  printf(1, "[*] Parent: %d\n", p_pid);
  int ratio = ((N-2)/P);
  // printf(1, "ratio: %d\n", ratio);
  int start, end;
  int x;
  int pipes[P-1][4];
  int pipesToParent[P][4];
  int finalsendPipe[P][2];

  for(i=0; i<P-1; i++){
    int p[2];
    if(pipe(p) < 0){
      printf(1, "Pipe not created!");
      exit();
    }
    pipes[i][0] = p[0]; // read P2
    pipes[i][1] = p[1]; // write P1
    
    if(pipe(p) < 0){
      printf(1, "Pipe not created!");
      exit();
    }
    pipes[i][2] = p[0]; // read P1
    pipes[i][3] = p[1]; // write P2
  }


  // for(i=0; i<(P-1); i++){
  //   for(j=0; j<4; j++){
  //     printf(1, "%d", pipes[i][j]);
  //   }
  //   printf(1, "\n");
  // }

  // parent-chilld comm
  for(i=0; i<P; i++){
    int p[2];

    if(pipe(p) < 0){
      printf(1, "Pipe not created!");
      exit();
    }
    pipesToParent[i][0] = p[0]; //parent read
    pipesToParent[i][1] = p[1]; //child writes


    if(pipe(p) < 0){
      printf(1, "Pipe not created!");
      exit();
    }
    pipesToParent[i][2] = p[0]; //child reads
    pipesToParent[i][3] = p[1]; //parent writes 
  }

  // for(i=0; i<(P); i++){
  //   for(j=0; j<4; j++){
  //     printf(1, "%d", pipesToParent[i][j]);
  //   }
  //   printf(1, "\n");
  // }

  for(i=0; i<P; i++){
    int p[2];

    if(pipe(p) < 0){
      printf(1, "Pipe not created!");
      exit();
    }
    finalsendPipe[i][0] = p[0]; //parent reads
    finalsendPipe[i][1] = p[1]; //children write
  }

  // for(i=0; i<(P); i++){
  //   for(j=0; j<2; j++){
  //     printf(1, "%d", finalsendPipe[i][j]);
  //   }
  //   printf(1, "\n");
  // }


  float diff;
  int count=0;


  for(x=0; x<P; x++){
    int cid1 = fork();
    if(cid1==0){
      diff=0;
      // printf(1, "Child no %d with ID %d\n", x, getpid());  

      start = x*ratio;
      end = (x+1)*ratio;
      if(x==0) {start=0;}
      if(x==P-1){end=N-2;}

      // printf(1, "start: %d | end: %d\n", start, end);

      float values[2+(end-start)][N];
      for(i=0; i<(end-start)+2; i++){
        for(j=0; j<N; j++){
          if(start==0){
            if(i==0){
              values[i][j]=T;
            }
            else{
              if((j==0) || (j==N-1)){
                values[i][j] = T;
              }else{
                values[i][j] = mean;
              }
            }
          }
          else if(end==N-2){
            if(i==(end-start)+1){
              if(j==0){
                values[i][j] = T;
              }else{
                values[i][j] = 0.0;
              }
            }
            else{
              if((j==0) || (j==N-1)){
                values[i][j] = T;
              }else{
                values[i][j] = mean;
              }
            }
          }
          else{
            if((j==0) || (j==N-1)){
              values[i][j] = T;
            }else{
              values[i][j] = mean;
            }
          }
        }
      }

      // fill up the ghost values
      float w[2+(end-start)][N];
      for(i=0; i<(end-start)+2; i++){
        for(j=0; j<N; j++){
          w[i][j] = 0;
        }
      }

      // P2 read | P1 write | P1 read | P2 write
      // P3 read | P2 write | P2 read | P3 write
      // P4 read | P3 write | P3 read | P4 write

    // loop
    for(;;){
      diff=0;
      // calculate the new mean:
      for(i=1; i<(end-start)+1; i++){
        for(j=1; j<N-1; j++){
          w[i][j] = (values[i-1][j] + values[i+1][j] + 
                        values[i][j-1] + values[i][j+1])/4.0;
          if(fabsm(w[i][j] - values[i][j]) > diff)
            diff = fabsm(w[i][j]- u[i][j]);
        } 
      }

      for (i =1; i<(end-start)+1; i++){
        for (j =1; j<N-1; j++){
          values[i][j] = w[i][j];
        }
      }

      // for(i=0; i<(end-start)+2; i++){
      //   for(j=0; j<N; j++){
      //     printf(1, "%d ", (int)values[i][j]);
      //   }
      //   printf(1, "\n");
      // }

      // for(;;){}

      write(pipesToParent[x][1], (char*)(&diff), MSGSIZE);
      read(pipesToParent[x][2], (char*)(&diff), MSGSIZE);

      count += 1;
      if(diff <= E || count > L){
        if(x==0){
          printf(1, "Iterations: %d\n", count);
        }
        // printf(1, "[#] Condition satisfied!\n");
        for(i=1; i<(end-start)+1; i++){
          for(j=1; j<N-1; j++){
            // printf(1, "send: %d %d\n", (int)values[i][j], getpid());
            write(finalsendPipe[x][1], (char*)(&values[i][j]), MSGSIZE);
          }
        }
        exit();
      }

      for(j=1; j<N-1; j++){
        if(start==0){
          write(pipes[x][1], (char*)(&values[(end-start)][j]), MSGSIZE);
        }else if(end==N-2){
          write(pipes[x-1][3], (char*)(&values[1][j]), MSGSIZE);
        }else{ 
          write(pipes[x-1][3], (char*)(&values[1][j]), MSGSIZE);
          write(pipes[x][1], (char*)(&values[(end-start)][j]), MSGSIZE);          
        }
      }
      for(j=1; j<N-1; j++){
        if(start==0){
          float msg=0;
          read(pipes[x][2], (char*)(&msg), MSGSIZE);
          values[(end-start)+1][j] = msg;
        }else if(end==N-2){
          float msg=0;
          read(pipes[x-1][0], (char*)(&msg), MSGSIZE);
          values[0][j] = msg;
        }else{ 
          float msg=0;
          read(pipes[x-1][0], (char*)(&msg), MSGSIZE);
          values[0][j] = msg;

          msg = 0;
          read(pipes[x][2], (char*)(&msg), MSGSIZE);
          values[(end-start)+1][j] = msg;
        }
      }
    }
    exit();
    }
  }


  float output[N][N];
  for(i=0; i<N; i++){
    for(j=0; j<N; j++){
      if(i==0){
        output[i][j] = T;
      }else if(j==0){
        output[i][j] = T;
      }else if(j==N-1){
        if(i==N-1){
          output[i][j] = 0.0;
        }else{
          output[i][j] = T;
        }
      }else{
        output[i][j] = 0.0;
      }
      // printf(1, "%d ", (int)output[i][j]);
    }
    // printf(1, "\n");
  }


  for(;;){
      float maxdiff=0;
      for(i=0; i<P; i++){
        float tempdiff=0;
        read(pipesToParent[i][0], (char*)(&tempdiff), MSGSIZE);
        if(tempdiff > maxdiff){
          maxdiff = tempdiff;
        }
      }
      if((int)maxdiff != 0){
        for(i=0; i<P; i++){
          write(pipesToParent[i][3], (char*)(&maxdiff), MSGSIZE);
        }
      }else{
        for(i=0; i<P; i++){
          write(pipesToParent[i][3], (char*)(&maxdiff), MSGSIZE);
        }
        int k;
        for(i=0;i<P;i++){
            start = i*ratio;
            end = (i+1)*ratio;
            if(i==0) {start=0;}
            if(i==P-1){end=N-2;}

            // printf(1, "%d - %d\n", start, end);
            for(j=start+1; j<end+1; j++){
              for(k=1; k<N-1; k++){ 
                read(finalsendPipe[i][0], (char*)(&output[j][k]), MSGSIZE);
              }
            }
        }
        break;
      }

  }

  for(i=0; i<N; i++){
    for(j=0; j<N; j++){
      printf(1, "%d ", (int)output[i][j]);
    }
    printf(1, "\n");
  }

  wait();
  printf(1, "[#] Is this the end?\n");
  exit();

}
