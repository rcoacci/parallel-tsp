#!/usr/bin/env bash
#PBS -l select=2:ncpus=48:mpiprocs=24
#PBS -l walltime=24:00:00
#PBS -j oe
#PBS -V
#PBS -N tsp-mpi

# load modules
module load gcc/11.2.0 openmpi-gnu/4.1.1

LOGDIR=/scratch/80061a/rcoacci/
cd ${PBS_O_WORKDIR}
echo "$(date) Iniciando..."
mpirun --mca btl_openib_allow_ib true --timestamp-output --output-filename ${LOGDIR}/${PBS_JOBNAME}.${PBS_JOBID} build/tsp-mpi data/dantzig42.data
echo "$(date) Fim"
