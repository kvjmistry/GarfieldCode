#!/bin/bash

# Declare an array of string with type, these are the EL drift vel values to run
declare -a RotArray=("Aligned" "Shifted")

# Iterate the string array using for loop
for i in ${!RotArray[@]}; do
   ROT=${RotArray[$i]}
   echo "Making jobscripts for Mode ROT: $ROT"
   mkdir -p TrackRes_$ROT
   cd  TrackRes_$ROT
   cp ../CalcTrackRes_job.sh .
   sed -i "s#.*SBATCH -J.*#\#SBATCH -J Trackres_${ROT} \# A single job name for the array#" CalcTrackRes_job.sh
   sed -i "s#.*SBATCH -o.*#\#SBATCH -o Trackres_${ROT}_%A_%a.out \# Standard output#" CalcTrackRes_job.sh
   sed -i "s#.*SBATCH -e.*#\#SBATCH -e Trackres_${ROT}_%A_%a.err \# Standard error#" CalcTrackRes_job.sh
   sed -i "s#.*KrROT=.*#KrROT=${FileArray[$i]}#" CalcTrackRes_job.sh
   sed -i "s#.*Mode=.*#Mode=${ROT}#" CalcTrackRes_job.sh
   sbatch --array=1-100 CalcTrackRes_job.sh
   cd ..
done