import argparse
import sys
import shutil
# Parsing argument
parser=argparse.ArgumentParser(
    description= "This script read the vtk file which included Press_mask field and write txt file for modifying Boundary Condition by chePen3D app."
)
parser.add_argument("vtk_dir", type=str, help="Direction of vtk file which included Press_mask filed.")
parser.add_argument("zfem_dir", type=str, help="Direction of zfem file which included points and connectivity of elements.")
args = parser.parse_args()
vtk_file = args.vtk_dir
# copy the vtk file in the root directory
shutil.copy(args.zfem_dir,"./BCmask.zfem")
# read vtk file
find_field = False
passed_line = 0
nelem = 0
field = []
try: 
    with open(vtk_file, "r") as f:
        lines=f.readlines()
        for row in lines:
            line =row.strip()
            if passed_line > 0:
                passed_line -= 1
                continue
            if "Press_mask" in line:
                find_field = True
                passed_line = 2
                continue
            if "CELLS" in line:
                value = line.split()
                nelem = int(value[1])
                continue
            if "SCALARS" in line:
                find_field = False
                passed_line = 0
                continue
            if find_field and passed_line == 0:
                value = line.split()
                if len(value) == 1:
                    field.append(int(value[0]))
                    continue
                else:
                    print(f"number of value in line : {line} is more than 1!", file=sys.stderr)
except FileExistsError as e:
    print(f"There is not file in path {vtk_file}", file=sys.stderr)
if len(field) != nelem:
    print(f"number of element Press_mask field {len(find_field)} not match with number of elements {nelem} in cells", file=sys.stderr)
print (f"Sucessfuly read Press_mask field with {len(field)} elements!")
#write readed field in .label file 
with open ("./BCmask.zfem.labels","w") as f:
    for i in range(nelem):
        f.write(f"{str(field[i])}\n")