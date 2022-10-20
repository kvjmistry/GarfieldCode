#!/bin/bash

Option="Kr"
# Option=""
SCRIPT=CalcTrackRes${Option}_job.sh

# Declare an array of string with type, these are the EL drift vel values to run
declare -a RotArray=("Aligned" "Shifted" "Rot30")

# Iterate the string array using for loop
for i in ${!RotArray[@]}; do
   ROT=${RotArray[$i]}
   echo "Making jobscripts for Mode ROT: $ROT"
   mkdir -p TrackRes_$ROT$Option
   cd  TrackRes_$ROT$Option
   cp ../$SCRIPT .
   sed -i "s#.*SBATCH -J.*#\#SBATCH -J Trackres_${ROT} \# A single job name for the array#" $SCRIPT
   sed -i "s#.*SBATCH -o.*#\#SBATCH -o Trackres_${ROT}_%A_%a.out \# Standard output#" $SCRIPT
   sed -i "s#.*SBATCH -e.*#\#SBATCH -e Trackres_${ROT}_%A_%a.err \# Standard error#" $SCRIPT
   sed -i "s#.*KrROT=.*#KrROT=${FileArray[$i]}#" $SCRIPT
   sed -i "s#.*Mode=.*#Mode=${ROT}#" $SCRIPT
   sbatch --array=1-100 $SCRIPT
   cd ..
done