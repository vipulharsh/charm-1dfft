#include <fftw3-mpi.h>
# include <stdlib.h>
# include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
# include <time.h>
#include <math.h>

int main(int argc, char **argv){

      const ptrdiff_t N0 = 4194304 ; //2^22
      fftw_plan plan;
      fftw_complex *data,*dataOut;
      ptrdiff_t alloc_local, local_ni, local_i_start, i, j,local_no, local_o_start;
      
      int index,size;
      MPI_Init(&argc, &argv);
      fftw_mpi_init();
      
      MPI_Comm_rank(MPI_COMM_WORLD,&index);
      MPI_Comm_size(MPI_COMM_WORLD,&size);

      /* get local data size and allocate */
      alloc_local = fftw_mpi_local_size_1d(N0, MPI_COMM_WORLD,FFTW_FORWARD, FFTW_ESTIMATE,
                                      &local_ni, &local_i_start,&local_no, &local_o_start);
      data = fftw_alloc_complex(alloc_local);
      dataOut = fftw_alloc_complex(alloc_local);
      
      /* initialize data to some function my_function(x,y) */
      for (i = 0; i < local_ni; ++i) {
            data[i][0] =rand() / (double)RAND_MAX;
            data[i][1] =rand() / (double)RAND_MAX;
      }


      /* create plan  */
      plan = fftw_mpi_plan_dft_1d(N0, data, data2, MPI_COMM_WORLD,
                             FFTW_FORWARD, FFTW_ESTIMATE);
      
      double startwtime, endwtime;
      if(index==0){
            startwtime = MPI_Wtime();
      }

      fftw_execute(plan);

      if(index==0){
            endwtime = MPI_Wtime();
            printf("wall clock time = %f\n", endwtime-startwtime);
      }

      fftw_destroy_plan(plan);
      MPI_Finalize();
}