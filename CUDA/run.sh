#!/bin/bash -l
#SBATCH -J histogram_cuda
#SBATCH -N 1
#SBATCH --ntasks-per-node=1
#SBATCH --gres=gpu:1
#SBATCH --time=00:10:00
#SBATCH --partition=plgrid-gpu-v100
#SBATCH -A plgcyfrovetares2024-gpu
#SBATCH --output=out_histogram_cuda
./main
