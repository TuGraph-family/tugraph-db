import sys

input_file = sys.argv[1]
fin = open(input_file, "r")
output_file = sys.argv[2]
fout = open(output_file, "w+")
weight = 1
if len(sys.argv) > 3:
    weight = int(sys.argv[3])

line = fin.readline()
while line:
    line = line.replace(" ", ",")
    fout.write(line)
    line = fin.readline()

fin.close()
fout.close()
