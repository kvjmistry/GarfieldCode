#!/bin/bash
#SBATCH -J Mesh # A single job name for the array
#SBATCH --nodes=1
#SBATCH --mem 4000 # Memory request (6Gb)
#SBATCH -t 0-24:00 # Maximum execution time (D-HH:MM)
#SBATCH -o Hex_%A_%a.out # Standard output
#SBATCH -e Hex_%A_%a.err # Standard error

start=`date +%s`

format_number() {
    local number=$1
    if [ $number -lt 10 ]; then
        printf "0%d" $number
    else
        printf "%d" $number
    fi
}

# Set the configurable variables
JOBNAME="GarfieldATPC"
VOLTAGE=8000
HEX=20
RADIUS=10

HEXFOLDER=$(format_number ${HEX})

MPHFILE="/home/argon/Projects/Krishan/garfieldpp/ATPC/build/${HEXFOLDER}mmHex/${HEX}mmHex.mphtxt"

# Create the directory
cd /media/argon/HDD_8tb/
mkdir -p $JOBNAME/${HEX}mm/${VOLTAGE}/jobid_"${SLURM_ARRAY_TASK_ID}"
cd $JOBNAME/${HEX}mm/${VOLTAGE}/jobid_"${SLURM_ARRAY_TASK_ID}"

# Setup nexus and run
echo "Setting Up Garfield" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
source /home/argon/Projects/Krishan/garfieldpp/setup_garfield.sh

# NEXUS
echo "Running Garfield" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt
/home/argon/Projects/Krishan/GarfieldCode/ATPC/build/ATPC ${RADIUS} ${VOLTAGE} ${MPHFILE} 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

echo; echo; echo;

echo "FINISHED....EXITING" 2>&1 | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt

end=`date +%s`
let deltatime=end-start
let hours=deltatime/3600
let minutes=(deltatime/60)%60
let seconds=deltatime%60
printf "Time spent: %d:%02d:%02d\n" $hours $minutes $seconds | tee -a log_nexus_"${SLURM_ARRAY_TASK_ID}".txt