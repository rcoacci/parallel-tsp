#!/usr/bin/env bash
cd logs
qsub <<EOF
#!/usr/bin/env bash
#PBS -l select=1:ncpus=24
#PBS -l walltime=08:00:00
#PBS -j oe
#PBS -V
#PBS -N serial-$1

# load modules
module load gcc/11.2.0

PROJ="\${PBS_O_HOME}/parallel-tsp"
HYPERFINE="hyperfine -r 5 --export-json serial-${1}.json" 
cd \${PBS_O_WORKDIR}
rm -rf ./serial-${1}.log
echo "\$(date) Iniciando $1 com 1 nos e 1 procs"
\${HYPERFINE} "\${PROJ}/build/tsp \${PROJ}/data/${1}.data >> ./serial-${1}.log"
echo "\$(date) Fim"
EOF
