#!/usr/bin/env bash
#PBS -l select=1:ncpus=48
#PBS -l walltime=00:30:00
#PBS -j oe
#PBS -V
#PBS -N tsp-serial

# load modules
module load gcc/11.2.0
cd ${PBS_O_WORKDIR}
build/tsp data/fri26.data
