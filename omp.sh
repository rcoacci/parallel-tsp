#!/usr/bin/env bash
cd logs
qsub <<EOF
#!/usr/bin/env bash
#PBS -l select=1:ncpus=48:ompthreads=$2
#PBS -l walltime=8:00:00
#PBS -j oe
#PBS -V
#PBS -N tsp-omp-$1-$2

# load modules
module load gcc/11.2.0

PROJ="\${PBS_O_HOME}/parallel-tsp"
cd \${PBS_O_WORKDIR}
echo "\$(date) Iniciando $1 com $2 procs"
hyperfine -r 5 --export-json tsp-omp-${1}-${2}.json "\${PROJ}/build/tsp-omp \${PROJ}/data/${1}.data"
echo "\$(date) Fim"
EOF
