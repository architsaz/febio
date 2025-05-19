#!/bin/bash
#
# Description:
#   This script prepares and runs a simulation based on provided arguments.
#   It sets OpenMP environment variables, checks for required executables,
#   logs run information, and executes actions based on the chosen option.
#
# Usage:
#   ./run_simulation.sh <casename> <option> <start_step> <last_step>
#
# Arguments:
#   <casename>   : Identifier for the simulation case.
#   <option>     : One of the available options: mknjmask | highcurv | nocorr | corrbynj | enhance.
#   <start_step> : The starting step (numerical).
#   <last_step>  : The ending step (numerical).
#
# Example:
#   ./run_simulation.sh Case01 highcurv 0 10
#
# Notes:
#   - Make sure that the required executables and files exist in the specified paths.
#   - The script uses different executables based on the hostname.
#
# Help:
#   ./run_simulation.sh --help
#

# Function to display the help message
show_help() {
    grep '^#' "$0" | cut -c 3-
    exit 0
}

# Display help if requested
if [ "$1" = "--help" ]; then
    show_help
fi

# Argument validation: ensure exactly four arguments are provided
if [ "$#" -ne 4 ]; then
    echo "Error: Provided $# arguments; four arguments are required."
    echo "Usage: <casename> <option> <start_step> <last_step>"
    exit 1
fi

# Assigning arguments to variables
casename="$1"
option="$2"
start_step="$3"
last_step="$4"

# Set OpenMP environment variables
ncpu=12
export OMP_NUM_THREADS="$ncpu"
export KMP_STACKSIZE="1g"
export OMP_DYNAMIC="FALSE"

# Set resource limits to unlimited for stack and virtual memory
ulimit -s unlimited
ulimit -v unlimited

# Determine executable paths based on hostname
if [ "$(hostname)" = "L-P4000-N05218" ]; then
    febmkr_EXE="../../../build/febmkr_exec"
    febio4_EXE="/opt/FEBioStudio/bin/febio4"
else
    febmkr_EXE="/dagon1/achitsaz/mylib/EXECs/febmkr_exec"
    febio4_EXE="/dagon1/achitsaz/app/FEBioStudio/bin/febio4"
fi

# Check if the executables exist
if [ ! -e "$febmkr_EXE" ]; then
  echo "Error: Could not find executable: $febmkr_EXE"
  exit 1
fi

if [ ! -e "$febio4_EXE" ]; then
  echo "Error: Could not find executable: $febio4_EXE"
  exit 1
fi

# Display the executables being used and OpenMP settings
echo "Using febio4 executable: $febio4_EXE"
echo "Using febmkr executable: $febmkr_EXE"
env | grep OMP_NUM_THREADS

# Log information file (run.info)
INFO="run.info"
if [ -e "$INFO" ] || [ -e "${INFO}.gz" ]; then
  echo "Error: $INFO (or ${INFO}.gz) already exists!"
  exit 1
fi

# Write environment and run information to the info file
{
    echo "host=$(hostname)"
    echo "working_directory=$(pwd)"
    env | grep OMP_NUM_THREADS
    echo "pid=$$"
    echo "UniqueID=$(date +%s)-${casename}-${option}"
} > "$INFO"

echo "Running in background ... [pid: $$]"

# Run simulation based on provided option
case "$option" in
    nocorr|highcurv)
        # Option: nocorr or highcurv with fixed start step 0
        "$febmkr_EXE" "$casename" "$option" 0 > out 2>&1 
        "$febio4_EXE" -i pres_0.feb >> out 2>&1
        # Check for the termination message in pres_0.log
        checkresult=$(grep "N O R M A L" "pres_0.log")
        if [ "$checkresult" = " N O R M A L   T E R M I N A T I O N" ]; then
            echo "$checkresult without Young's Modulus modification."
        fi
        ;;
    mknjmask)
        # Option: mknjmask with provided start_step parameter
        "$febmkr_EXE" "$casename" "$option" "$start_step" > out 2>&1
        ;;    
    corrbynj|enhance)
        # Option: corrbynj or enhance loops from start_step to last_step
        for i in $(seq "$start_step" "$last_step"); do
            echo "Starting loop iteration $i for option $option..."
            if [ "$i" -eq 0 ]; then
                "$febmkr_EXE" "$casename" nocorr 0 > out 2>&1
                "$febio4_EXE" -i pres_0.feb >> out 2>&1
            else
                "$febmkr_EXE" "$casename" "$option" "$i" >> out 2>&1
                "$febio4_EXE" -i pres_"$i".feb >> out 2>&1
            fi
            checkresult=$(grep "N O R M A L" "pres_${i}.log")
            if [ "$checkresult" = " N O R M A L   T E R M I N A T I O N" ]; then
                echo "$checkresult at iteration $i with option ($option) Young's Modulus modification."
                break
            fi
        done
        ;;
    *)
        echo "Invalid option: $option"
        echo "Usage: $0 {mknjmask|nocorr|corrbynj|highcurv|enhance} <start_step> <last_step>"
        exit 1
        ;;
esac
