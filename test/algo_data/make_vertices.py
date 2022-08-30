mport sys

output_file = sys.argv[1]
num_vertices = sys.argv[2]
f1 = open(output_file, "w+")
for i in range(0, int(num_vertices)):
    f1.write(str(i) + "\n")

