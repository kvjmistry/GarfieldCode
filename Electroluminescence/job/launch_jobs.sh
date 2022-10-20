#!/bin/bash

# JOB=Aligned
#JOB=Rotated
#JOB=Shifted
JOB=Rot30

mkdir $JOB
cd $JOB
cp ../${JOB}_Mesh_job.sh .

sbatch --array=1-2000 ${JOB}_Mesh_job.sh