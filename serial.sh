#!/usr/bin/env bash
cd logs
qsub <<EOF
#!/usr/bin/env bash
#PBS -l select=1:ncpus=24
#PBS -l walltime=4:00:00
#PBS -j oe
#PBS -V
#PBS -N tsp-$1

# load modules
module load gcc/11.2.0

PROJ="\${PBS_O_HOME}/parallel-tsp"

cd \${PBS_O_WORKDIR}
echo "\$(date) Iniciando $1 com 1 nos e 1 procs"
hyperfine -r 5 --export-json tsp-${1}.json "\${PROJ}/build/tsp \${PROJ}/data/${1}.data"
echo "\$(date) Fim"
EOF
