#!/usr/bin/env bash
cd logs
qsub <<EOF
#!/usr/bin/env bash
#PBS -l select=$2:ncpus=48:mpiprocs=$3
#PBS -l walltime=24:00:00
#PBS -j oe
#PBS -V
#PBS -N tsp-mpi-$1-$3

# load modules
module load gcc/11.2.0 openmpi-gnu/4.1.1

PROJ="\${PBS_O_HOME}/parallel-tsp"
MPIEXTRA="--mca btl_openib_allow_ib true --timestamp-output --oversubscribe"

cd \${PBS_O_WORKDIR}
echo "\$(date) Iniciando $1 com $2 nos e $3 procs"
hyperfine -r 5 --export-json tsp-mpi-${1}-${2}-${3}.json "mpirun \${MPIEXTRA} \${PROJ}/build/tsp-mpi \${PROJ}/data/${1}.data"
echo "\$(date) Fim"
EOF
