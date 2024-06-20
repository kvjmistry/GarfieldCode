#!/bin/bash

echo "Starting Job" 

JOBID=$1
echo "The JOBID number is: ${JOBID}" 

JOBNAME=$2
echo "The JOBNAME number is: ${JOBNAME}" 

echo "JOBID $1 running on `whoami`@`hostname`"
start=`date +%s`

SCRIPT=$3
echo "Script name is: ${SCRIPT}"
start=`date +%s`

ls -ltrh

# Setup nexus
echo "Setting Up Garfield" 
source /software/garfieldpp/setup_garfield.sh

# Set the configurable variables
N_EVENTS=1
PRESSURE=4.2
MPHFILE="Rotated_Mesh_Data_Rings.mphtxt"
DATAFILE="Rotated_Mesh_Data_Rings.txt"
TYPE="Rotated"

# Calculate the unique seed number	
SEED=$((${N_EVENTS}*${JOBID} + ${N_EVENTS}))
echo "The seed number is: ${SEED}" 

# NEXUS
echo "Running Garfield"
# evt id, num e-, seed, grid, jobid, mode [Aligned, Rotated, Shifted] gridfile datafile pressure
./Mesh ${SEED} ${N_EVENTS} ${SEED} 1 ${JOBID} ${TYPE} ${MPHFILE} ${DATAFILE} ${PRESSURE}

echo; echo; echo;

echo "FINISHED....EXITING"

ls -ltrh

end=`date +%s`
let deltatime=end-start
let hours=deltatime/3600
let minutes=(deltatime/60)%60
let seconds=deltatime%60
printf "Time spent: %d:%02d:%02d\n" $hours $minutes $seconds