#!/usr/bin/env bash
cd vtune-results
qsub <<EOF
#!/usr/bin/env bash
#PBS -l select=$2:ncpus=48:mpiprocs=$3
#PBS -l walltime=8:00:00
#PBS -j oe
#PBS -V
#PBS -N tsp-mpi-$1-$3

# load modules
module load gcc/11.2.0 openmpi-gnu/4.1.1 /sw/intel/oneapi/modulefiles/vtune/2021.2.0 

PROJ="\${PBS_O_HOME}/parallel-tsp"
MPIEXTRA="--mca btl_openib_allow_ib true --timestamp-output --oversubscribe"
VTUNE_ARGS="-quiet -collect hpc-performance -k analyze-openmp=true -k stack-size=4096 -result-dir=./mpi-$1-$2-${3}hpc -finalization-mode=full -search-dir=\$PROJ/build/ -search-dir=\$PROJ/src "
cd \${PBS_O_WORKDIR}
echo "\$(date) Iniciando $1 com $2 nos e $3 procs"
mpirun \${MPIEXTRA} vtune \$VTUNE_ARGS -- \${PROJ}/build/tsp-mpi \${PROJ}/data/${1}.data
echo "\$(date) Fim"
EOF

