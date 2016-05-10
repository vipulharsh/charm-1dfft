#!/bin/bash
#
#PBS -l walltime=01:00:00
#PBS -l nodes=4:ppn=7
#PBS -N myjob 
#PBS -q cse
#
# End of embedded QSUB options
#

# Run the hello world executable (a.out)


~/cs598-project/gennodelist.sh > ~/cs598-project/.nodelist


module load gcc/4.7.1 
module load mpi/mpich/3.1.3-gcc-4.7.1


sizes=(256 1024 4096 16384 65536 262144)
procs=(2 4 8 16)
for idx in "${!sizes[@]}";
do
for pdx in "${!procs[@]}";
do
  	fftsize=${sizes[$idx]}
  	numproc=${procs[$pdx]}
 	for i in {1..3};
 	do
        COMMAND="mpirun -np ${numproc} /home/vharsh2/cs598-project/fftw/parfftw  ${fftsize}"
        #echo $COMMAND
        #$COMMAND
 	done
 	for i in {1..3};
 	do
        COMMAND="/home/vharsh2/cs598-project/charmrun +p${numproc} ~/cs598-project/fft  ${numproc} ${fftsize} ++nodelist ~/cs598-project/.nodelist"
        echo $COMMAND
        $COMMAND
 	done
 	for i in {1..3};
 	do
        COMMAND="/home/vharsh2/cs598-project/charmrun +p${numproc} ~/cs598-project/transfft  ${numproc} ${fftsize} ++nodelist ~/cs598-project/.nodelist"
        #echo $COMMAND
        #$COMMAND
 	done
done
done
