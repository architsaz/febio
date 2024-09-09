#!/bin/bash

# List of machines
machines=("loki" "hades" "marduk" "heise" "attila" "ishtar")
# List of cases
cases=($(cat cases.txt)) 
# Track the number of cases completed
completed_cases=0
total_cases=${#cases[@]}
echo "Total # cases: $total_cases"
echo "Total # mashines: ${#machines[@]}"

