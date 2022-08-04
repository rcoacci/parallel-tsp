#!/usr/bin/env bash
cds
qsub <<EOF
#!/usr/bin/env bash
#PBS -l select=1:ncpus=48:ompthreads=$2
#PBS -l walltime=24:00:00
#PBS -j oe
#PBS -V
#PBS -N tsp-omp

# load modules
module load gcc/11.2.0

PROJ="${PBS_O_HOME}/parallel-tsp"
cd ${PBS_O_WORKDIR}
echo "$(date) Iniciando..."
${PROJ}/build/tsp-omp ${PROJ}/data/$1
echo "$(date) Fim"
EOF
