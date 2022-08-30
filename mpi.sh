#!/usr/bin/env bash
cd logs
qsub <<EOF
#!/usr/bin/env bash
#PBS -l select=$2:ncpus=48:mpiprocs=$3
#PBS -l walltime=4:00:00
#PBS -j oe
#PBS -V
#PBS -N mpi-$1-$2-$3

# load modules
module load gcc/11.2.0 openmpi-gnu/4.1.1

PROJ="\${PBS_O_HOME}/parallel-tsp"
MPIEXTRA="--mca btl_openib_allow_ib true --timestamp-output --oversubscribe"
HYPERFINE="hyperfine -r 5 --export-json mpi-${1}-${2}-${3}.json"

cd \${PBS_O_WORKDIR}
rm -f ./mpi-${1}-${2}-${3}.log
echo "\$(date) Iniciando $1 com $2 nos e $3 procs"
\${HYPERFINE} "mpirun \${MPIEXTRA} \${PROJ}/build/tsp-mpi \${PROJ}/data/${1}.data >> ./mpi-${1}-${2}-${3}.log"
echo "\$(date) Fim"
EOF
