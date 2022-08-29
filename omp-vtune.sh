#!/usr/bin/env bash
cd vtune-results
qsub <<EOF
#!/usr/bin/env bash
#PBS -l select=1:ncpus=48:ompthreads=$2
#PBS -l walltime=8:00:00
#PBS -j oe
#PBS -V
#PBS -N tsp-omp-$1-$2

# load modules
module load gcc/11.2.0 /sw/intel/oneapi/modulefiles/vtune/2021.2.0

PROJ="\${PBS_O_HOME}/parallel-tsp"

VTUNE_ARGS="-quiet -collect hpc-performance -k analyze-openmp=true -k stack-size=4096 -result-dir=./omp-$1-${2}hpc -finalization-mode=full -search-dir=\$PROJ/build/ -search-dir=\$PROJ/src"

cd \${PBS_O_WORKDIR}
echo "\$(date) Iniciando $1 com $2 procs"
vtune \$VTUNE_ARGS -- \${PROJ}/build/tsp-omp \${PROJ}/data/${1}.data
echo "\$(date) Fim"
EOF
