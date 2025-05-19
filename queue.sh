#!/bin/bash
code_name=febio4
# List of machines
machines=("ishtar" "loki" "hades" "attila" "heise")
# List of cases
lists_cases_dir=/dagon1/achitsaz/runfebio/all_labeled_cases.txt

if [ -f "$lists_cases_dir" ]; then 
    cases=($(cat "$lists_cases_dir"))
else
    echo "ERROR: File does not exist at: $lists_cases_dir"
    exit 1
fi
 
completed_cases=0

total_cases=${#cases[@]}
echo "Total # cases in the list: $total_cases"
echo "Total # of available mashines: ${#machines[@]}"

run_case_on_machine () {
    local dir_name=/dagon1/achitsaz/runfebio
    echo "-> Running $2 on machine $1"
    if [ $1 == "ishtar" ]; then
        cd $code_dir 
        /dagon1/achitsaz/FEBio/scripts.feb/mkrdata.sh $2 $dir_name
        cd $dir_name/$2/msa.1/ 
        nohup ./run.sh $2 nocorr 0 0 > run.log 2>&1 &
    else
        ssh $1 "/dagon1/achitsaz/FEBio/scripts.feb/mkrdata.sh $2 $dir_name"
        ssh $1 "cd $dir_name/$2/msa.1/ && nohup ./run.sh $2 nocorr 0 0 > run.log 2>&1 &" &
    fi 
}

while [ $completed_cases -lt $total_cases ]; do
    echo "* starting to find a available machine for case : ${cases[0]}"
    find_machine=0
    for machine in "${machines[@]}"; do
        #check if the machine run a task
        if [ $machine == "ishtar" ]; then
            running_task=$(pgrep febio4 | wc -l)
        else
            running_task=$(ssh $machine "pgrep febio4 | wc -l")
        fi
        if [ "$running_task" -eq 0 ] && [ ${#cases[@]} -gt 0 ]; then
            find_machine=1
            case=${cases[0]}
            cases=("${cases[@]:1}")
            run_case_on_machine "$machine" "$case"
            if [ ${#cases[@]} -eq 0 ]; then
                echo "All cases completed!"
                exit
            fi 
        fi
    done    
    completed_cases=$(($total_cases-${#cases[@]}))
    if [ $find_machine -eq 0 ]; then
        sleep 1m
    else
        sleep 10
    fi    
done

