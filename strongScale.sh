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

module load mpi/mpich/3.1.3-gcc-4.7.1

LOGDIR="/home/vharsh2/cs554/project/logs"



for i in {1..10};
do
COMMAND="mpirun -np 1 /home/vharsh2/cs554/project/bin/tc"
#echo $COMMAND
$COMMAND
done


for i in {1..10};
do
COMMAND="mpirun -np 2 /home/vharsh2/cs554/project/bin/tw"
#echo $COMMAND
$COMMAND
done


for i in {1..10};
do
COMMAND="mpirun -np 2 /home/vharsh2/cs554/project/bin/ts"
#echo $COMMAND
$COMMAND
done


#BN=100
#for p in $(seq 2 1 15);
#do
# N=$((BN * p))
# for i in {1..3};
# do
#        COMMAND="mpirun -np ${p} /home/vharsh2/cs554/project/bin/1DcolumnBlock ${N}"
#        echo $COMMAND
#        $COMMAND
# done
#done

#for p in $(seq 2 1 15);
#do
# N=$((BN * p)) 
# for i in {1..3};
# do
#        COMMAND="mpirun -np ${p} /home/vharsh2/cs554/project/bin/1DrowBlock ${N}"
#        echo $COMMAND
#        $COMMAND
# done
#done

#is=( 2 2 2 3 3 3 4 4 5)
#js=( 2 3 4 3 4 5 4 5 5)
#for idx in "${!is[@]}";
#do
#        p1=${is[$idx]}
#        p2=${js[$idx]}
#        p=$((p1*p2))
#        N=$((BN * p))
#        for i in {1..3};
#        do
#        COMMAND="mpirun -np ${p} /home/vharsh2/cs554/project/bin/2Dhavoc ${p1} ${p2} ${N}"
#        echo $COMMAND
#        $COMMAND
#        done
#done


mpirun -np 25 /home/vharsh2/cs554/project/bin/2DhavocJS 5 5 3000
mpirun -np 25 /home/vharsh2/cs554/project/bin/1DrowBlockJS  3000
mpirun -np 25 /home/vharsh2/cs554/project/bin/1DcolumnBlockJS 3000

