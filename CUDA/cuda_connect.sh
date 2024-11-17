#!/bin/bash
 srun -N 4 --ntasks-per-node=4 --gres=gpu:1 -p plgrid-gpu-v100 -A plgcyfrovetares2024-gpu -t 01:30:00 --pty /bin/bash
