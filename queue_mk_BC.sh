#!/bin/bash

code_name=mk_BC_by_pmask.py
machines=("ishtar" "loki" "hades" "attila" "marduk")
# machines=("ishtar")

list_dir=/dagon1/achitsaz/runfebio/problematic_run_cases.txt
use_test_case=false  # Set to false to read from file

if [ "$use_test_case" = true ]; then
    raw_cases=("agh111.1")
else
    if [ -f "$list_dir" ]; then 
        mapfile -t raw_cases < "$list_dir"
    else
        echo "ERROR: Case list not found at: $list_dir"
        exit 1
    fi
fi
# Generate 1 jobs per case
jobs=()
for case in "${raw_cases[@]}"; do
    jobs+=("$case")
done

total_jobs=${#jobs[@]}
echo "Total # of jobs to run: $total_jobs"
echo "Total # of machines: ${#machines[@]}"

run_case_on_machine () {
    local machine=$1
    local case_id=$2

    local case_dir=/dagon1/achitsaz/runfebio
    local code_dir=/dagon1/achitsaz/FEBio/scripts.feb

    local command="cd $case_dir/$case_id/data/ && nohup python3 $code_dir/$code_name ../msa.1/checkinput_0.vtk $case_id.flds.zfem > BCmask.log 2>&1 &"

    echo "-> Dispatching: $case_id on $machine"

    if [ "$machine" == "ishtar" ]; then
        eval "$command"
    else
        ssh "$machine" "$command" &
    fi
}

while [ ${#jobs[@]} -gt 0 ]; do
    echo "* Waiting for machine availability..."
    machine_found=0

    for machine in "${machines[@]}"; do
        if [ "$machine" == "ishtar" ]; then
            running_task=$(pgrep python3 | wc -l)
        else
            running_task=$(ssh "$machine" "pgrep python3 | wc -l")
        fi

        if [ "$running_task" -eq 0 ]; then
            machine_found=1

            # Get and remove first job
            IFS=' ' read -r case_id <<< "${jobs[0]}"
            jobs=("${jobs[@]:1}")

            run_case_on_machine "$machine" "$case_id"
            break
        fi
    done

    if [ $machine_found -eq 0 ]; then
        sleep 1m
    else
        sleep 10
    fi
done

echo "✅ All jobs dispatched!"
