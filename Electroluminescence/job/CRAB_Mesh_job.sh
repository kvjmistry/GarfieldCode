#!/bin/bash
#SBATCH -J CRAB # A single job name for the array
#SBATCH --nodes=1
#SBATCH --mem 4000 # Memory request (6Gb)
#SBATCH -t 0-12:00 # Maximum execution time (D-HH:MM)
#SBATCH -o CRAB_%A_%a.out # Standard output
#SBATCH -e CRAB_%A_%a.err # Standard error

start=`date +%s`

# Set the configurable variables
JOBNAME="Mesh"
TYPE="CRAB"
N_EVENTS=20

# Create the directory
cd /media/argon/NVME1/Krishan/
mkdir -p $JOBNAME/$TYPE/jobid_"${SLURM_ARRAY_TASK_ID}"
cd $JOBNAME/$TYPE/jobid_"${SLURM_ARRAY_TASK_ID}"

# Setup nexus and run
echo "Setting Up Garfield" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
source /home/argon/Projects/Krishan/garfieldpp/setup_garfield.sh

# Calculate the unique seed number	
SEED=$((${N_EVENTS}*(${SLURM_ARRAY_TASK_ID} - 1) + ${N_EVENTS}))
echo "The seed number is: ${SEED}" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

# NEXUS
echo "Running Garfield" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
# evt id, num e-, seed, grid, jobid, mode [align, rot, shift]
/home/argon/Projects/Krishan/GarfieldCode/Electroluminescence/build/CRAB ${SEED} ${N_EVENTS} ${SEED} 1 ${SLURM_ARRAY_TASK_ID} 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

echo; echo; echo;

echo "FINISHED....EXITING" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

end=`date +%s`
let deltatime=end-start
let hours=deltatime/3600
let minutes=(deltatime/60)%60
let seconds=deltatime%60
printf "Time spent: %d:%02d:%02d\n" $hours $minutes $seconds | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
