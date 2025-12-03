import numpy as np
import pandas as pd
import os
import re

dir_runfebio = "/dagon1/achitsaz/runfebio/"
dir_approved_case = "/dagon1/achitsaz/runfebio/all_cases.txt"
report_file = os.path.join(dir_runfebio, "check_run_all_cases.csv")
studies = ["pst.1","pst.2"]
# Read approved cases
with open(dir_approved_case, "r") as f:
    list_labeled_cases = [line.strip() for line in f]

# Initialize results lists
case_names = []
corrected_bc = []
wall_mask = []
check_status_msa_1 = []
last_solved_time_1 = []
check_status_msa_2 = []
last_solved_time_2 = []
run_time_msa_1 = []
run_time_msa_2 = []
for case in list_labeled_cases:
    case_names.append(case)

    # Check if BCmask.txt exists
    BCmask_path = os.path.join(dir_runfebio, case, "data", "BCmask.zfem.labels")
    corrected_bc.append(1 if os.path.exists(BCmask_path) else 0)

    # Check if wall mask exists
    wallmask_path = os.path.join(dir_runfebio, case, "data", f"{case}.wall")
    wall_mask.append(1 if os.path.exists(wallmask_path) else 0)    

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
accept_status_msa1 = []
accept_status_msa2 = []
for i in range(len(list_labeled_cases)):
    accept_status.append(1 if last_solved_time_1[i] > 0.95 and last_solved_time_2[i] > 0.95 else 0)
    accept_status_msa1.append(1 if last_solved_time_1[i] > 0.95 else 0)
    accept_status_msa2.append(1 if last_solved_time_2[i] > 0.95 else 0)
# check availabilty of extracted in-plane stress  
extracted_stress_status = []
extracted_stress_status_msa1 = []
extracted_stress_status_msa2 = []
for case in list_labeled_cases:
    path1 = os.path.join(dir_runfebio,case,studies[0],"stress_analysis_0.vtk")
    path2 = os.path.join(dir_runfebio,case,studies[1],"stress_analysis_0.vtk")
    extracted_stress_status_msa1.append(1 if os.path.exists(path1) else 0 )
    extracted_stress_status_msa2.append(1 if os.path.exists(path2) else 0 )
    extracted_stress_status.append(1 if (os.path.exists(path1) and os.path.exists(path2)) else 0 )

# Create DataFrame
report_df = pd.DataFrame({
    "case": case_names,
    "approved_both_status": extracted_stress_status,
    "approved_msa1_status": extracted_stress_status_msa1,
    "approved_msa2_status": extracted_stress_status_msa2,
    "run_both_status": accept_status,
    "run_msa1_status": accept_status_msa1,
    "run_msa2_status": accept_status_msa2,
    "wall_mask_exist": wall_mask,
    "BC_corrected": corrected_bc,
    "exist_msa1": check_status_msa_1,
    "last_time_msa1": last_solved_time_1,
    "run_time_msa1": run_time_msa_1,
    "exist_msa2": check_status_msa_2,
    "last_time_msa2": last_solved_time_2,
    "run_time_msa2": run_time_msa_2
})

# Save to CSV
report_df.to_csv(report_file, index=False)
print(f"Saved report to {report_file}")
