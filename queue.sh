#!/bin/bash
code_name=febio4
# List of machines
machines=("ishtar" )
# List of cases
cases=($(cat cases.txt)) 
# Track the number of cases completed
completed_cases=0
total_cases=${#cases[@]}
echo "Total # cases in the list: $total_cases"
echo "Total # of available mashines: ${#machines[@]}"

run_case_on_machine () {
    local dir_name=test
    local code_dir=/dagon1/achitsaz/FEBio
    echo "Running $2 on machine $1"
    if [ $1 == "ishtar" ]; then
        cd $code_dir 
        /dagon1/achitsaz/FEBio/scripts/mkrdata.sh $2 $dir_name
        cd $code_dir/$dir_name/$2/msa.1/ 
        nohup ./run.sh $2 nocorr 0 0 > run.log 2>&1 &
    else
        ssh $1 "cd $code_dir && /dagon1/achitsaz/FEBio/scripts/mkrdata.sh $2 $dir_name"
        ssh $1 "cd $code_dir/$dir_name/$2/msa.1/ && nohup ./run.sh $2 nocorr 0 0 > run.log 2>&1 &" &
    fi 
}

while [ $completed_cases -lt $total_cases ]; do
    for machine in "${machines[@]}"; do
        echo "check machine $machine"
        #check if the machine run a task
        if [ $machine == "ishtar" ]; then
            running_task=$(pgrep febio4 | wc -l)
        else
            running_task=$(ssh $machine "pgrep febio4 | wc -l")
        fi
        echo "running_task: $running_task"
        if [ "$running_task" -eq 0 ] && [ ${#cases[@]} -gt 0 ]; then
            case=${cases[0]}
            cases=("${cases[@]:1}")
            run_case_on_machine "$machine" "$case"
        fi
        echo "Total # cases in the list: ${#cases[@]}"
    done    
    completed_cases=$(($total_cases-${#cases[@]}))

    sleep 1m
done

echo "All cases completed!"
