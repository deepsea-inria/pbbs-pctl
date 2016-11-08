#!/bin/bash

target=_results/results_bfs_main.txt

cp _results/results_bfs_real.txt $target
cat _results/results_bfs_new.txt >> $target

cat _results/results_pbfs_real.txt >> $target
cat _results/results_pbfs_new.txt >> $target

ocamlopt -w d -I ../../pbench/xlib/  -I ../../pbench/lib/ unix.cmxa str.cmxa ../../pbench/xlib/XBase.ml ../../pbench/xlib/XFloat.ml ../../pbench/xlib/XInt.ml ../../pbench/xlib/XList.ml ../../pbench/xlib/XMath.ml ../../pbench/xlib/XOption.ml ../../pbench/xlib/XString.ml ../../pbench/xlib/XOpt.ml ../../pbench/xlib/XFile.ml ../../pbench/xlib/XSys.ml ../../pbench/xlib/XCmd.ml  ../../pbench/lib//pbench.ml ../../pbench/lib//env.ml ../../pbench/lib//params.ml ../../pbench/lib//runs.ml ../../pbench/lib//results.ml ../../pbench/lib//legend.ml ../../pbench/lib//axis.ml ../../pbench/lib//latex.ml ../../pbench/lib//rtool.ml ../../pbench/lib//chart.ml ../../pbench/lib//bar_plot.ml ../../pbench/lib//scatter_plot.ml ../../pbench/lib//mk_bar_plot.ml ../../pbench/lib//mk_runs.ml ../../pbench/lib//mk_scatter_plot.ml ../../pbench/lib//mk_table.ml bench.ml -o bench.pbench && ./bench.pbench compare -size large -benchmark bfs -only plot -proc 40 -exts unke100,unke30
