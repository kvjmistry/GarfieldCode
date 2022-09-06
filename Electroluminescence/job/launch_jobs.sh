#!/bin/bash

JOB=Aligned

mkdir $JOB
cd $JOB
cp ../Mesh_job.sh .

sbatch --array=1-10 Mesh_job.sh