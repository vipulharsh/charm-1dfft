#include <charm++.h>
#include "complex.h"
#include "transfft.decl.h"
#include <math.h>
#include <assert.h>

/* readonly */ CProxy_transfft fftarr;
/* readonly */ CProxy_Main mainproxy;
/* readonly */ int nchares; 
/* readonly */ int N;

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
		if((N & (N-1)) || (nchares & (nchares-1)) || (nchares*nchares>N)){
			printUsage();
			printf("Incorrect Arguments, nchares & N should be powers of 2, nchares should be <= root(N)\n");
			CkExit();
		}
		printf("nchares: %d, N: %d \n", nchares, N);
		mainproxy = thisProxy;
		finished = 0;
		fftarr = CProxy_transfft::ckNew(nchares);
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


class transfft : public CBase_transfft{
	complex *arr;
	int nelem;
	int numrows;
	int rootN, logN;
	int me;
	int step;
	double start_time, end_time;
	public:
	
	transfft_SDAG_CODE

	transfft(){
		_sdag_init();
	}

	transfft(CkMigrateMessage *m){};

	void setdata(){
		for(int i=0; i<nelem; i++){
			arr[i].x = me*(N/nchares)+i;
			arr[i].y = 0;
		}
	}

	void print(){
		for(int i=0; i<nelem; i++){
			printf("(%Lf, %Lf) - %d\n", arr[i].x, arr[i].y, me);
		}
	}

	void init(){
		nelem = N/nchares;
		logN = 0;
		while((1<<logN)<N){
			logN++;
		}
		assert((1<<logN)==N);
		rootN = (1<<(logN/2));
		numrows = rootN/nchares;
		assert(rootN * numrows == nelem);
		assert(numrows * nchares == rootN);
		arr= new complex[nelem];
		me = this->thisIndex;
		setdata();
		start_time = CmiWallTimer();
		//Handling All-to-All comm. : any other way?
		complex *scratch = new complex[numrows * numrows];
		for(int i=0; i<nchares; i++){
			int basecol = i*numrows;
			for(int r=0; r<numrows; r++)
				memcpy(scratch + r*numrows, &arr[r*rootN + basecol], numrows * sizeof(complex));
			thisProxy[i]._recvfirst(numrows*numrows, scratch, me);
		}
		delete(scratch);
	}


	void recvfirst(int size, complex* data, int from){
		assert(size==numrows*numrows);
		int baserow = from * numrows;
		for(int c=0; c<numrows; c++){
			int base = c*rootN + baserow;
			for(int r=0; r<numrows; r++){
				//column major
				arr[base + r] = data[r*numrows+c]; 
			}
		}
	}

	void firstphase(){
		for(step=0; (1<<step)<rootN; step++){
			int currN = rootN>>step;
			int divN = N>>step;
			int nblocks = (1<<step);
			for(int col=0; col<numrows; col++){
				for(int block=0; block<nblocks; block++){
					for(int j=0; j<currN/2; j++){
						int p = (col*rootN) + (block*currN) + j;
						int q = p + (currN/2);
						complex temp = arr[p];
						arr[p] +=  arr[q];
						arr[q] = temp-arr[q];
						int glblcol = me*numrows+col;
						int glblrow = p - (col*rootN);
						int num = ((glblrow*rootN)+glblcol)%divN;
						//printf("p:%d, q:%d, j:%d, glblrow:%d, glblcol: %d, num:%d, divN: %d, me: %d\n", p, q, j, glblrow, glblcol, num, divN, me);
						arr[q] *= ei2PI(-(ld)num/divN);
					}
				}
			}
		}
		complex *scratch = new complex[numrows*numrows];
		for(int i=0; i<nchares; i++){
			int baserow = i*numrows;
			for(int c=0; c<numrows; c++)
				memcpy(scratch + c*numrows, &arr[c*rootN+baserow], numrows * sizeof(complex));
			fftarr[i]._recvsecond(numrows*numrows, scratch, me);
		}
		delete(scratch);
	}

	void recvsecond(int size, complex *data, int from){
		int basecol = from * numrows;
		for(int r=0; r<numrows; r++){
		   int base = r*rootN + basecol;	
			for(int c=0; c<numrows; c++)
				arr[base + c] = data[c*numrows + r];
		}
	}

	void secondphase(){
		while(N>>step > 1){
			int currN = (N>>step);
			assert(currN <= nelem);
			int nblocks = nelem/currN;
			for(int i=0; i<nblocks; i++){
				for(int j=0; j<currN/2; j++){
					int p = i*(currN) + j;
					int q = p + currN/2;
					//printf("Op %d <--> %d , i: %d, j: %d, currN: %d\n", p, q, i, j, currN);	
					complex temp = arr[p];
					arr[p] += arr[q];
					arr[q] = temp-arr[q];
					arr[q] *= ei2PI(-(ld)j/currN);
				}	
			}
			++step;
		}
		end_time = CmiWallTimer();
      		if(CkMyPe()==0)
      			printf("transFFT Size: %d, nchares: %d,  Walltime: %lf sec\n", N, nchares, end_time-start_time);
		//print();
	}

};


#include "transfft.def.h"
