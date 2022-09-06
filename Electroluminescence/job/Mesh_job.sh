#!/bin/bash
#SBATCH -J Mesh # A single job name for the array
#SBATCH -c 1 # Number of cores
#SBATCH -p shared # Partition
#SBATCH --mem 4000 # Memory request (6Gb)
#SBATCH -t 0-5:00 # Maximum execution time (D-HH:MM)
#SBATCH -o Mesh_%A_%a.out # Standard output
#SBATCH -e Mesh_%A_%a.err # Standard error

start=`date +%s`

# Set the configurable variables
JOBNAME="Mesh"
N_El=10
N_EVENTS=1

# Create the directory
cd $SCRATCH/guenette_lab/Users/$USER/
mkdir -p $JOBNAME/jobid_"${SLURM_ARRAY_TASK_ID}"
cd $JOBNAME/jobid_"${SLURM_ARRAY_TASK_ID}"

# Setup nexus and run
echo "Setting Up Garfield" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
source /n/home05/kvjmistry/packages/garfieldpp/setup_garfieldpp.sh

# Calculate the unique seed number	
SEED=$((${N_EVENTS}*(${SLURM_ARRAY_TASK_ID} - 1) + ${N_EVENTS}*${i}))
echo "The seed number is: ${SEED}" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

# NEXUS
echo "Running Garfield" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
/n/home05/kvjmistry/packages/GarfieldCode/Electroluminescence/build/Mesh ${SEED} ${N_El} ${SEED} 1 ${SLURM_ARRAY_TASK_ID} 0 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

echo; echo; echo;

echo "FINISHED....EXITING" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

end=`date +%s`
let deltatime=end-start
let hours=deltatime/3600
let minutes=(deltatime/60)%60
let seconds=deltatime%60
printf "Time spent: %d:%02d:%02d\n" $hours $minutes $seconds | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt