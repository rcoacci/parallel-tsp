#!/usr/bin/env bash
#PBS -l select=2:ncpus=48:mpiprocs=24
#PBS -l walltime=00:30:00
#PBS -j oe
#PBS -V
#PBS -N tsp-mpi

# load modules
module load gcc/11.2.0 openmpi-gnu/4.1.1

cd ${PBS_O_WORKDIR}
echo "$(date) Iniciando..."
mpirun build/tsp-mpi data/fri26.data
echo "$(date) Fim"
