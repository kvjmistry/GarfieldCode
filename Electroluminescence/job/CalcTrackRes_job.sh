#!/bin/bash
#SBATCH -J Trackres # A single job name for the array
#SBATCH -c 1 # Number of cores
#SBATCH -p shared # Partition
#SBATCH --mem 4000 # Memory request (6Gb)
#SBATCH -t 0-12:00 # Maximum execution time (D-HH:MM)
#SBATCH -o Trackres_%A_%a.out # Standard output
#SBATCH -e Trackres_%A_%a.err # Standard error

echo "Initialising NEXUS environment" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
start=`date +%s`

# Set the configurable variables
JOBNAME="TrackRes"
Mode="Aligned"
FILES_PER_JOB=1
N_EVENTS=1000
CONFIG=NEW.eminus.config.mac
INIT=NEW.eminus.init.mac

# Create the directory
cd $SCRATCH/guenette_lab/Users/$USER/
mkdir -p $JOBNAME/$Mode/jobid_"${SLURM_ARRAY_TASK_ID}"
cd $JOBNAME/$Mode/jobid_"${SLURM_ARRAY_TASK_ID}"

# Copy the files over
cp ~/packages/GarfieldCode/Electroluminescence/config/* .
cp ~/packages/GarfieldCode/Electroluminescence/CalcTrackRes.py .
cp ~/packages/GarfieldCode/Electroluminescence/Maps/*.h5 .

# Setup nexus and run
echo "Setting Up NEXUS and IC" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
source ~/packages/nexus/setup_nexus.sh
source ~/packages/IC/setup_IC.sh

for i in $(eval echo "{1..${FILES_PER_JOB}}"); do

    # Replace the seed in the file	
    SEED=$((${N_EVENTS}*${FILES_PER_JOB}*(${SLURM_ARRAY_TASK_ID} - 1) + ${N_EVENTS}*${i}))
    echo "The seed number is: ${SEED}" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
    sed -i "s#.*random_seed.*#/nexus/random_seed ${SEED}#" ${CONFIG}
    sed -i "s#.*start_id.*#/nexus/persistency/start_id ${SEED}#" ${CONFIG}

    # NEXUS
    echo "Running NEXUS" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
    nexus -n $N_EVENTS ${INIT} 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

    # Now run the Track resolution script
    python CalcTrackRes.py $Mode | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

    echo; echo; echo;
done

# Cleaning up

# Remove the config files if not the first jobid
if [ ${SLURM_ARRAY_TASK_ID} -ne 1 ]; then
    rm -v *.mac 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
    rm -v NEW.eminus.next | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
fi

echo "FINISHED....EXITING" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

end=`date +%s`
let deltatime=end-start
let hours=deltatime/3600
let minutes=(deltatime/60)%60
let seconds=deltatime%60
printf "Time spent: %d:%02d:%02d\n" $hours $minutes $seconds | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt