#!/bin/bash

# Define arrays
# voltages=(2500 3000 3500 4000 5000 6000 7000 8000)
# fieldMaps=(2 4 6 8 10 12 14 16 18 20)

voltages=(2500)
fieldMaps=(2)

SCRIPT=ATPC_job.sh

# Iterate through radii
for r in {1..10}; do
    echo "Current Radius = $r"
    for voltage in "${voltages[@]}"; do
        echo "Current Voltage = $voltage"
        # RunSimulation is a placeholder for the actual command you want to run
        echo "Running simulation with fieldMap: ${fieldMaps[r-1]}, voltage: $voltage, radius: $r"
        sed -i "s#.*VOLTAGE=.*#VOLTAGE=${voltage}#" ATPC_job.sh
        sed -i "s#.*HEX=.*#HEX=${fieldMaps[r-1]}#" ATPC_job.sh
        sed -i "s#.*RADIUS=.*#RADIUS=${r}#" ATPC_job.sh
        sbatch ${SCRIPT}
    done
done