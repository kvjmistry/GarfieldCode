#!/bin/bash

JOB=Aligned
#JOB=Rotated
#JOB=Shifted
PRESSURE=13.5
EMODE=20 # Electric field config
EXTENSION=""

# Set the electric field mode
if [ "$EMODE" -eq 20 ]; then
    EXTENSION=""
elif [ "$EMODE" -eq 15 ]; then
    EXTENSION="_E15kV"
else
    EXTENSION="_E30kV"
fi

echo "The electric field is: ${EXTENSION}"

mkdir $JOB
cd $JOB
cp ../Mesh_job.sh .

# Replace a few of the commands
sed -i "s#.*TYPE=.*#TYPE=$JOB#" Mesh_job.sh
sed -i "s#.*PRESSURE=.*#PRESSURE=$PRESSURE#" Mesh_job.sh

# Aligned
if [ "$JOB" = "Aligned" ]; then
    sed -i "s#.*MPHFILE=.*#MPHFILE=\"Aligned_Mesh_Data_Rings${EXTENSION}.mphtxt\"#" Mesh_job.sh
    sed -i "s#.*DATAFILE=.*#DATAFILE=\"Aligned_Mesh_Data_Rings${EXTENSION}.txt\"#" Mesh_job.sh
#Rotated
elif [ "$JOB" = "Rotated" ]; then
    sed -i "s#.*MPHFILE=.*#MPHFILE=\"Aligned_Mesh_Data_Rings${EXTENSION}.mphtxt\"#" Mesh_job.sh
    sed -i "s#.*DATAFILE=.*#DATAFILE=\"Aligned_Mesh_Data_Rings${EXTENSION}.txt\"#" Mesh_job.sh
# Shifted
else
    sed -i "s#.*MPHFILE=.*#MPHFILE=\"Aligned_Mesh_Data_Rings${EXTENSION}.mphtxt\"#" Mesh_job.sh
    sed -i "s#.*DATAFILE=.*#DATAFILE=\"Aligned_Mesh_Data_Rings${EXTENSION}.txt\"#" Mesh_job.sh
fi


sbatch --array=1-4000 Mesh_job.sh