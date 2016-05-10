#include <charm++.h>
#include "complex.h"
#include "fft.decl.h"

#include <assert.h>

/* readonly */ CProxy_fft fftarr;
/* readonly */ CProxy_Main mainproxy;
/* readonly */ int nchares; 
/* readonly */ int N;
/* readonly */ int ITERATION;



//Main chare
class Main : public CBase_Main{
   int finished; 
   public:
   Main(CkArgMsg *m){
      if(m->argc < 3){
	 CkExit();
      }
      nchares = atoi(m->argv[1]);
      N = atoi(m->argv[2]);
      if((N & (N-1)) || (nchares & (nchares-1)) || (N<nchares)){
	 printUsage();
	 printf("Incorrect Arguments, nchares & N should be powers of 2\n");
	 CkExit();
      }
      printf("nchares: %d, N: %d \n", nchares, N);
      fftarr = CProxy_fft::ckNew(nchares);
      mainproxy = thisProxy;
      finished = 0;
      ITERATION = 0;
      while(nchares>>ITERATION > 1)
	 ITERATION++;
      fftarr.run();
   }
   Main(CkMigrateMessage *m){}
   void finish(){
      finished++;
      if(finished==nchares)
	 CkExit();
   }
   void printUsage(){
      printf("Usage: ./charmrun +p<Processors> ./fft <nchares> <N>\n");
   }
};


class fft : public CBase_fft{
   complex *arr;
   int nelem;
   int me;
   int step;
   int it;
   double start_time, end_time;

   public:
   
   fft_SDAG_CODE

   fft(){
      _sdag_init();
   }

   fft(CkMigrateMessage *m){};

   void setdata(){
      for(int i=0; i<nelem; i++){
	 arr[i].x = me*(N/nchares)+i;
	 arr[i].y = 0;
      }
      //print();	
   }
   void print(){
      for(int i=0; i<nelem; i++){
	 printf("(%Lf, %Lf) - %d\n", arr[i].x, arr[i].y, me);
      }
   }

   void start(){
      nelem = N/nchares;
      arr= new complex[nelem];
      me = this->thisIndex;
      step = 0;
      setdata();
      start_time = CmiWallTimer();
      int dest = (me+nchares/2)%nchares;
      //printf("Sending to %d - %d\n", dest, me);
      //print();
      if(nchares > 1)
	 fftarr[dest].recv(step, nelem, arr);
      else
	 calclocal();      
   }


   //synchronization :: SDAG?	
   void recvvalues(int size, complex* data){
      assert(size==nelem);
      int currN = (N>>step);
      //ckout<<"N: "<<N<<", step: "<<step<<" - "<<me<<endl;
      if((me*(N/nchares))/(currN>>1) == 0){
	 for(int i=0; i<size; ++i){
	    arr[i] += data[i];		
	 }
      }
      else{
	 for(int i=0; i<size; i++){
	    arr[i] = data[i] - arr[i];
	    int ind = ((me * (N/nchares)) + i)%(currN/2);
	    //printf("ind: %d, me: %d, i: %d, currN: %d \n", ind, me, i, currN);
	    arr[i] *= ei2PI(-(ld)ind/currN); 
	 }
      }
      ++step;
      int currnchare = nchares>>step;
      if(currnchare>1){
	 int dest = ((me/currnchare)*currnchare) + 
	    (me%currnchare + (currnchare/2))%currnchare;
	 //printf("dest: %d, step: %d  - me; %d \n", dest, step,  me);
	 fftarr[dest].recv(step, nelem, arr);
      }
      else{
	 calclocal();
      }			
   }
 
   void calclocal(){
      while(N>>step > 1){
	 //print();
	 int currN = (N>>step);
	 assert(currN <= nelem);
	 int nblocks = nelem/currN;
	 //printf("N: %d, currN: %d, nelem: %d\n", N, currN, nelem);
	 for(int i=0; i<nblocks; i++){
	    for(int j=0; j<currN/2; j++){
	       int p = i*(currN) + j;
	       int q = p + currN/2;
	       //printf("Op %d <--> %d , i: %d, j: %d, currN: %d\n", p, q, i, j, currN);	
	       //printf("(%Lf,%Lf) (%Lf, %Lf)\n", arr[p].x, arr[p].y, arr[q].x, arr[q].y);
	       complex temp = arr[p];
	       arr[p] += arr[q];
	       arr[q] = temp-arr[q];
	       arr[q] *= ei2PI(-(ld)j/currN);
	       //printf("(%Lf,%Lf) (%Lf, %Lf)\n", arr[p].x, arr[p].y, arr[q].x, arr[q].y);
	    }	
	 }
	 //print();
	 ++step;
      }
      end_time = CmiWallTimer();
      if(CkMyPe()==0)
      	printf("FFT Size: %d, nchares: %d,  Walltime: %lf sec\n", N, nchares, end_time-start_time);
      //print();
      mainproxy.finish();
   }
};


#include "fft.def.h"
