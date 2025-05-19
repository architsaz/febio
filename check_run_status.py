import numpy as np
import pandas as pd
import os
import re

dir_runfebio = "/dagon1/achitsaz/runfebio/"
dir_approved_case = "/dagon1/achitsaz/runfebio/all_labeled_cases.txt"
report_file = os.path.join(dir_runfebio, "run_report.csv")
studies = ["pst.1","pst.2"]
# Read approved cases
with open(dir_approved_case, "r") as f:
    list_labeled_cases = [line.strip() for line in f]

# Initialize results lists
case_names = []
corrected_bc = []
check_status_msa_1 = []
last_solved_time_1 = []
check_status_msa_2 = []
last_solved_time_2 = []
run_time_msa_1 = []
run_time_msa_2 = []
for case in list_labeled_cases:
    case_names.append(case)

    # Check if BCmask.txt exists
    BCmask_path = os.path.join(dir_runfebio, case, "data", "BCmask.txt")
    corrected_bc.append(1 if os.path.exists(BCmask_path) else 0)

    # Check msa.1 status
    msa1_log = os.path.join(dir_runfebio, case, "msa.1", "pres_0.log")
    normal_found_1 = False
    last_time_1 = 0.0
    runtime = 0.0
    if os.path.exists(msa1_log):
        with open(msa1_log, "r") as file:
            lines = file.readlines()
            text = ''.join(lines)
            if "N O R M A L" in text:
                normal_found_1 = True
                last_time_1 = 2.0
            else:
                times = re.findall(r'Time\s*=\s*([0-9]*\.?[0-9]+)', text)
                if times:
                    last_time_1 = max(map(float, times))
            
            for row in lines:
                if "Total elapsed time" in row:
                    match = re.search(r"\(([\d\.]+) sec\)", row)
                    if match:
                        runtime = float(match.group(1))

    check_status_msa_1.append(1 if normal_found_1 else 0)
    last_solved_time_1.append(last_time_1)
    run_time_msa_1.append(runtime)

    # Check msa.2 status
    msa2_log = os.path.join(dir_runfebio, case, "msa.2", "pres_0.log")
    normal_found_2 = False
    last_time_2 = 0.0
    runtime = 0.0
    if os.path.exists(msa2_log):
        with open(msa2_log, "r") as file:
            lines = file.readlines()
            text = ''.join(lines)
            if "N O R M A L" in text:
                normal_found_2 = True
                last_time_2 = 2.0
            else:
                times = re.findall(r'Time\s*=\s*([0-9]*\.?[0-9]+)', text)
                if times:
                    last_time_2 = max(map(float, times))
            for row in lines:
                if "Total elapsed time" in row:
                    match = re.search(r"\(([\d\.]+) sec\)", row)
                    if match:
                        runtime = float(match.group(1))
                  
    check_status_msa_2.append(1 if normal_found_2 else 0)
    last_solved_time_2.append(last_time_2)
    run_time_msa_2.append(runtime) 
    
# find approved run
accept_status = []
for i in range(len(list_labeled_cases)):
    accept_status.append(1 if last_solved_time_1[i] > 0.95 and last_solved_time_2[i] > 0.95 else 0)
# check availabilty of extracted in-plane stress  
extracted_stress_status = []
for case in list_labeled_cases:
    path1 = os.path.join(dir_runfebio,case,studies[0],"stress_analysis_0.vtk")
    path2 = os.path.join(dir_runfebio,case,studies[1],"stress_analysis_0.vtk")
    extracted_stress_status.append(1 if (os.path.exists(path1) and os.path.exists(path2)) else 0 )

# Create DataFrame
report_df = pd.DataFrame({
    "case": case_names,
    "2Dstress_status": extracted_stress_status,
    "accept_status": accept_status,
    "BC_corrected": corrected_bc,
    "status_msa1": check_status_msa_1,
    "last_time_msa1": last_solved_time_1,
    "run_time_msa1": run_time_msa_1,
    "status_msa2": check_status_msa_2,
    "last_time_msa2": last_solved_time_2,
    "run_time_msa2": run_time_msa_2
})

# Save to CSV
report_df.to_csv(report_file, index=False)
print(f"Saved report to {report_file}")
