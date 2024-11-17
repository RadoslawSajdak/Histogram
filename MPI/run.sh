#!/bin/bash -l
#SBATCH -J histogram_omp
#SBATCH -A ${GRANT_NAME}-cpu
#SBATCH -N 1
#SBATCH --ntasks-per-node=2
#SBATCH --time=00:10:00
#SBATCH -p plgrid-testing
#SBATCH --output=histogram_omp.out
mpiexec ./main