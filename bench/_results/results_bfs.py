import sys
from statistics import mean,stdev

values = dict()

def read_from_file(filename, algo, keys):
    inf = open(filename)
    current_file = None
    current_mode = None
    current_proc = ''
    current = None
    for line in inf.readlines():
        if 'infile' in line:
            current_file = line.split()[1]
        if 'prog' in line:
            current_mode = line.split()[-1].split(".")[-1]
        if 'lib_type' in line:
            current_mode = (current_mode if line.split()[1] == 'pctl' else 'pbbs') + " " + algo
        if 'proc' in line:
            current_proc = int(line.split()[1])
        if '---' in line:
            current = (current_file, current_proc)
            continue
        if '===' in line:
            current = None
            continue
        if current == None:
            continue
        if "large" not in current_file:
            continue
        if current not in values:
            values[current] = dict()
        if current_mode not in values[current]:
            values[current][current_mode] = dict()
        tokens = line.split()
        key = " ".join(tokens[:-1])
        if key[-1] == ":":
            key = key[:-1]
        if key not in keys:
            continue
        if key not in values[current][current_mode]:
            values[current][current_mode][key] = []
        values[current][current_mode][key].append(float(tokens[-1]))

read_from_file(sys.argv[1], "bfs", ["exectime"])
read_from_file(sys.argv[2], "pbfs", ["exectime"])

procs = [1,10,20,30,39,40]

exts = ["pbbs bfs", "unke30 bfs", "unke100 bfs", "pbbs pbfs", "unke30 pbfs", "unke100 pbfs"]

print("\\begin{tabular}{|c|c|c|c|c|c|c|c|}")
print("\\hline")
print("input & processors & bfs pbbs & bfs unke30 & bfs unke100 & pbfs pbbs & pbfs unke30 & pbfs unke100 \\\\\\hline")

previous_filename = None
for params in sorted(values):
     (f, p) = params
     if previous_filename != f:
          previous_filename = f
          prepare = f.replace("_", ".").replace("/", ".").split(".")

          show = prepare[2] + "_" + prepare[3]
          
          print("\\multirow{" + str(len(procs)) + "}{*}{" + show.replace('_', '\\_') + "}")

     if p not in procs:
          continue

     time_pbbs = mean(values[params]["pbbs bfs"]["exectime"])
     percent_deviation = int(stdev(values[params]["pbbs bfs"]["exectime"]) / time_pbbs * 100)
     print("&" + str(p) + " & {0:.3f}".format(time_pbbs) + " (" + str(percent_deviation) + "\\%) ")

     for ext in exts:
         if ext == "pbbs bfs":
             continue

         relative_time = mean(values[params][ext]["exectime"]) / time_pbbs
         diff = int((relative_time - 1) * 100)
         percent_deviation = 0 if mean(values[params][ext]["exectime"]) < 0.001 else int(stdev(values[params][ext]["exectime"]) / mean(values[params][ext]["exectime"]) * 100)

         if diff == 0:
             print(" & 0\\% ")
         elif diff > 0:
             print(" & +" + str(diff) + "\\% ")
         else:
             print(" & " + str(diff) + "\\% ")
         print("(" + str(percent_deviation) + "\\%) ")
     if procs[-1] != p:
          print("\\\\\\cline{2-8}")
     else:
          print("\\\\\\hline")

print("\\end{tabular}")