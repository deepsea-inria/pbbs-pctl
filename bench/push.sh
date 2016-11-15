#!/bin/bash

ocamlopt -w d -I ../../pbench/xlib/  -I ../../pbench/lib/ unix.cmxa str.cmxa ../../pbench/xlib/XBase.ml ../../pbench/xlib/XFloat.ml ../../pbench/xlib/XInt.ml ../../pbench/xlib/XList.ml ../../pbench/xlib/XMath.ml ../../pbench/xlib/XOption.ml ../../pbench/xlib/XString.ml ../../pbench/xlib/XOpt.ml ../../pbench/xlib/XFile.ml ../../pbench/xlib/XSys.ml ../../pbench/xlib/XCmd.ml  ../../pbench/lib//pbench.ml ../../pbench/lib//env.ml ../../pbench/lib//params.ml ../../pbench/lib//runs.ml ../../pbench/lib//results.ml ../../pbench/lib//legend.ml ../../pbench/lib//axis.ml ../../pbench/lib//latex.ml ../../pbench/lib//rtool.ml ../../pbench/lib//chart.ml ../../pbench/lib//bar_plot.ml ../../pbench/lib//scatter_plot.ml ../../pbench/lib//mk_bar_plot.ml ../../pbench/lib//mk_runs.ml ../../pbench/lib//mk_scatter_plot.ml ../../pbench/lib//mk_table.ml bench.ml -o bench.pbench

./bench.pbench compare -size large -benchmark comparison_sort,blockradix_sort,remove_duplicates,suffix_array,convex_hull,nearest_neighbours,ray_cast -only plot -proc 40 -exts unke100

./bench.pbench compare -size small -benchmark delaunay_refine -only plot -proc 40 -exts unke100

./bench.pbench compare -size medium -benchmark delaunay -only plot -proc 40 -exts unke100

for i in $( ls _results/*.r ); do
    cp $i ~/Work/deepsea/2016/draft-pctl/_results
    R --file=$i
done

cp _results/plots*.pdf ~/Work/deepsea/2016/draft-pctl/_results
