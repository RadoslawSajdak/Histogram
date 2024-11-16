#!/bin/bash
./export_grant.sh
srun -N 1 --ntasks-per-node=1 -p plgrid -A ${GRANT_NAME}-cpu --reservation=${RES_NAME} -t 01:30:00 --pty /bin/bash