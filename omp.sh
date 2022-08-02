#!/usr/bin/env bash
#PBS -l select=1:ncpus=48:ompthreads=24

#PBS -l walltime=24:00:00
#PBS -j oe
#PBS -V
#PBS -N tsp-omp

# load modules
module load gcc/11.2.0

cd ${PBS_O_WORKDIR}
echo "$(date) Iniciando..."
build/tsp-omp data/dantzig42.data
echo "$(date) Fim"
