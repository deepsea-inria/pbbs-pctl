open XBase
open Params

(**
Before running this script the dependencies should be properly installed.
The pbbs-pctl, pctl and pbbs-include repositories should appear in the same folder at the same level.

This bench script supports following modes and parameters.
Modes:
generate - generates all the files need for the specified sizes and benchmarks
compare - compiles, runs for the specified binaries, sizes of inputs and benchmarks and builds plots for benchmarks
to compare with pbbs

Parameters of the run.
Benchmark (list of strings):
The list of the benchmarks to run on. The supported benchmarks:
- blockradix_sort
- comparison_sort
- remove_duplicates
- suffix_array (most of the input data is pregenerated)
- convex_hull
- nearest_neighbours
- ray_cast (only has pregenerated input data)
- bfs (only has pregenerated input data)
- delaunay (works only on sizes <= medium)
- delaunay_refine (works only on size small)

Exts (list of strings):
The extensions of the binaries to compare. The most interesting:
- manc, the version using manual gc
- norm100, the version using original gc and kappa 100
- unke100, the version using the proposed gc and kappa 100
More modes could be found in Makefile.

Size (list of strings string):
The specified size of the inputs to run binaries on. The available sizes:
- small
- medium
- large

Proc (list of integers):
The number of processors to run binaries on.

Runs (int):
The number of times to get time-data for each test run.

The typical list of commands to generate:
./bench.ml generate -benchmark blockradix_sort,comparison_sort,remove_duplicates,suffix_array,convex_hull,nearest_neighbours -size large
./bench.ml generate -benchmark delaunay -size medium
./bench.ml generate -benchmark delaunay_refine -size small

The typical list of commands to compare approaches:
./bench.ml compare -benchmark blockradix_sort,comparison_sort,remove_duplicates,suffix_array,convex_hull,nearest_neighbours,ray_cast,bfs -size large -runs 6 -exts norm100,unke100 -proc 1,10,20,30,39
./bench.ml compare -benchmark delaunay -size medium -runs 6 -exts norm100,unke100 -proc 1,10,20,30,39
./bench.ml compare -benchmark delaunay_refine -size small -runs 6 -exts norm100,unke100 -proc 1,10,20,30,39
**)

let system = XSys.command_must_succeed_or_virtual

(*****************************************************************************)
(** Parameters *)

let arg_virtual_run = XCmd.mem_flag "virtual_run"
let arg_virtual_build = XCmd.mem_flag "virtual_build"
let arg_nb_runs = XCmd.parse_or_default_int "runs" 1
let arg_mode = Mk_runs.mode_from_command_line "mode"
let arg_skips = XCmd.parse_or_default_list_string "skip" []
let arg_onlys = XCmd.parse_or_default_list_string "only" []
let arg_sizes = XCmd.parse_or_default_list_string "size" ["all"]
let arg_benchmarks = XCmd.parse_or_default_list_string "benchmark" ["all"]
let arg_proc = XCmd.parse_or_default_list_int "proc" [1; 10; 39]
let arg_extension = XCmd.parse_or_default_string "ext" "norm"

let run_modes = Mk_runs.([
	 Mode arg_mode;
	 Virtual arg_virtual_run;
	 Runs arg_nb_runs; ])

(*****************************************************************************)
(** Steps *)

let select make run check plot =
   let arg_skips =
      if List.mem "run" arg_skips && not (List.mem "make" arg_skips)
         then "make"::arg_skips
         else arg_skips
      in
   Pbench.execute_from_only_skip arg_onlys arg_skips [
      "make", make;
      "run", run;
      "check", check;
      "plot", plot;
      ]

let nothing () = ()

(*****************************************************************************)
(** Files and binaries *)

let build path bs is_virtual =
   system (sprintf "make -C %s -j %s" path (String.concat " " bs)) is_virtual

let file_results exp_name =
  Printf.sprintf "_results/results_%s_%s.txt" exp_name arg_extension

let file_plots exp_name =
  Printf.sprintf "_plots/plots_%s_%s.pdf" exp_name arg_extension

let file_tables_src exp_name =
  Printf.sprintf "tables_%s.tex" exp_name

let file_tables exp_name =
  Printf.sprintf "tables_%s.pdf" exp_name
                 
(** Evaluation functions *)

let eval_exectime = fun env all_results results ->
  Results.get_mean_of "exectime" results

let eval_exectime_stddev = fun env all_results results ->
  Results.get_stddev_of "exectime" results


let default_formatter =
 Env.format (Env.(format_values  [ "size"; ]))

(*****************************************************************************)
(** Input data **)
                        
type size = Small | Medium | Large
                               
let string_of_size = function
   | Small -> "small"
   | Medium -> "medium"
   | Large -> "large"

let size_of_string = function
   | "small" -> Small
   | "medium" -> Medium
   | "large" -> Large
   | _ -> Pbench.error "invalid argument for argument size"

let arg_sizes =
   match arg_sizes with
   | ["all"] -> ["small"; "medium"; "large"]
   | _ -> arg_sizes

let sequence_benchmarks = ["comparison_sort"; "blockradix_sort"; "remove_duplicates";
                      "suffix_array"; "convex_hull"; "nearest_neighbours"; "ray_cast"; "delaunay"; "delaunay_refine"; "bfs"; ]

let arg_benchmarks = 
   match arg_benchmarks with
   | ["all"] -> sequence_benchmarks
   | ["sequence"] -> sequence_benchmarks
   | _ -> arg_benchmarks

let use_sizes =
   List.map size_of_string arg_sizes

let mk_sizes =
   mk_list string "size" (List.map string_of_size use_sizes)
                       
let mk_generator generator = mk string "generator" generator

let types_list = function
  | "comparison_sort" -> [ "array_double"; "array_string"; ]
  | "blockradix_sort" -> [ "array_int"; "array_pair_int_int"; ]
  | "remove_duplicates" -> [ "array_int"; "array_string"; ] (** "array_pair_string_int"; ]**)
  | "suffix_array" -> [ "string"; ]
  | "convex_hull" -> [ "array_point2d"; ]
  | "nearest_neighbours" -> [ "array_point2d"; "array_point3d"; ]
  | "ray_cast" -> []
  | "reduce" -> [ "array_int"; "array_double"; ]
  | "scan" -> [ "array_int"; "array_double"; ]
  | "loop" -> []
  | "delaunay" -> [ "array_point2d"; ]
  | "delaunay_refine" -> [ "triangles_point2d"; ]
  | "bfs" | "pbfs" -> [ "graph"; ]
  | _ -> Pbench.error "invalid benchmark"

let generators_list = function
  | "comparison_sort" -> (function n ->
    let nb_swaps nb = int_of_float (sqrt (float_of_int nb)) in
    function
    | "array_double" -> [
        mk_generator "random";
        mk_generator "exponential";
        ((mk_generator "almost_sorted") & (mk int "nb_swaps" (nb_swaps n)));
      ]
    | "array_string" -> [
        mk_generator "trigrams";
      ]
    | _ -> Pbench.error "invalid type")
  | "blockradix_sort" -> (function n ->
    function
    | "array_int" -> [
        mk_generator "random";
        mk_generator "exponential";
      ]
    | "array_pair_int_int" -> [
(** TODO solve the problem with the output file name **)
        ((mk_generator "random") & (mk int "m2" 256));
        ((mk_generator "random") & (mk int "m2" n));
      ]
    | _ -> Pbench.error "invalid type")
  | "remove_duplicates" -> (function n ->
    function
    | "array_int" -> [
        mk_generator "random";
        ((mk_generator "random_bounded") & (mk int "m" 100000));
        mk_generator "exponential";
      ]
    | "array_string" -> [
        mk_generator "trigrams";
      ]
(**    | "array_pair_string_int" -> [
        mk_generator "trigrams";
      ]**)
    | _ -> Pbench.error "invalid type")
  | "suffix_array" -> (function n ->
    function
    | "string" -> [
       mk_generator "trigrams";
     ]
    | _ -> Pbench.error "invalid type")
  | "convex_hull" -> (function n ->
    function
    | "array_point2d" -> [
        mk_generator "in_circle";
        mk_generator "kuzmin";
        mk_generator "on_circle";
      ]
    | _ -> Pbench.error "invalid type")
  | "nearest_neighbours" -> (function n ->
    function
    | "array_point2d" -> [
        mk_generator "in_square";
        mk_generator "kuzmin";
      ]
    | "array_point3d" -> [
        mk_generator "in_cube";
        mk_generator "on_sphere";
        mk_generator "plummer"; 
      ]
    | _ -> Pbench.error "invalid_type")
  | "ray_cast" -> (function n -> function typ -> [])
  | "reduce" -> (function n -> 
     function
     | "array_int" -> [
         mk_generator "random";
       ]
     | "array_double" -> [
         mk_generator "random";
       ]
     | _ -> Pbench.error "invalid_type")
  | "scan" -> (function n ->
     function
     | "array_int" -> [
         mk_generator "random";
        ]
     | "array_double" -> [
          mk_generator "random";
        ]
     | _ -> Pbench.error "invalid_type")
  | "loop" -> (function n -> function typ -> [])
  | "delaunay" -> (function n -> 
    function
    | "array_point2d" -> [
        mk_generator "in_square_delaunay";
        mk_generator "kuzmin_delaunay";
      ]
    | _ -> Pbench.error "invalid_type")
  | "delaunay_refine" -> (function n ->
    function
    | "triangles_point2d" -> [
        mk_generator "delaunay_in_square_refine";
        mk_generator "delaunay_kuzmin_refine";
      ]
    | _ -> Pbench.error "invalid_type")
  | "bfs" | "pbfs" -> (function n -> function typ -> [])
  | _ -> Pbench.error "invalid benchmark"

let graphfile_of n = "_data/" ^ n ^ ".bin"

let pretty_graph_name n =
  if (graphfile_of "europe") = n then
    "europe"
  else if (graphfile_of "rgg") = n then
    "rgg"
  else if (graphfile_of "twitter") = n then
    "twitter"
  else if (graphfile_of "delaunay") = n then
    "delaunay"
  else if (graphfile_of "usa") = n then
    "usa"
  else if (graphfile_of "livejournal") = n then
    "livejournal"
  else if (graphfile_of "wikipedia") = n then
    "wikipedia-2007"
  else if (graphfile_of "grid_sq_large") = n then
    "square-grid"
  else if (graphfile_of "paths_100_phases_1_large") = n then
    "par-chains-100"
  else if (graphfile_of "phased_524288_single_large") = n then
    "trees_524k"
  else if (graphfile_of "phased_low_50_large") = n then
    "phases-50-d-5"
  else if (graphfile_of "phased_mix_10_large") = n then
    "phases-10-d-2"
  else if (graphfile_of "random_arity_100_large") = n then
    "random-arity-100"
  else if (graphfile_of "tree_2_512_1024_large") = n then
    "trees-512-1024"
  else if (graphfile_of "unbalanced_tree_trunk_first_large") = n then
    "trunk-first" 
  else if (graphfile_of "3Dgrid_J_10000000") = n then
    "3D-grid"
  else if (graphfile_of "randLocalGraph_J_5_10000000") = n then
    "random"
  else if (graphfile_of "rMatGraph_J_5_10000000") = n then
    "rMat"
  else if (graphfile_of "cube_large") = n then
    "cube-grid"
  else if (graphfile_of "rmat24_large") = n then
    "rmat24"
  else if (graphfile_of "rmat27_large") = n then
    "rmat27"
  else     
    n

                                      
let graphfiles = List.map graphfile_of
  ["livejournal"; "twitter"; "wikipedia"; "rgg"; "delaunay"; "usa"; "europe"; "tree_2_512_1024_large"; "random_arity_100_large";
   "rmat27_large"; "phased_mix_10_large"; "rmat24_large"; "phased_low_50_large"; "cube_large";
   "phased_524288_single_large"; "grid_sq_large"; "paths_100_phases_1_large"; "unbalanced_tree_trunk_first_large"(*;
   "3Dgrid_J_10000000"; "rMatGraph_J_5_10000000"; "randLocalGraph_J_5_10000000";*)
  ]

let mk_generate_sequence_inputs benchmark : Params.t =
  match benchmark with
  | "ray_cast" ->
    (mk string "type" "ray_cast_test") &
    (((mk int "n" 1000000) & (mk string "generator" "in_cube") & (mk string "outfile" "_data/incube_ray_cast_1m.bin")) ++
     ((mk int "n" 2000000) & (mk string "generator" "in_cube") & (mk string "outfile" "_data/incube_ray_cast_2m.bin")) ++
     ((mk int "n" 1000000) & (mk string "generator" "on_sphere") & (mk string "outfile" "_data/onsphere_ray_cast_1m.bin")) ++
     ((mk int "n" 2000000) & (mk string "generator" "on_sphere") & (mk string "outfile" "_data/onsphere_ray_cast_2m.bin")) ++
     ((mk int "n" 5000000) & (mk string "generator" "on_sphere") & (mk string "outfile" "_data/onsphere_ray_cast_5m.bin")) ++
     ((mk int "n" 10000000) & (mk string "generator" "on_sphere") & (mk string "outfile" "_data/onsphere_ray_cast_10m.bin")))
  | _ ->
    let load = function
      | Small -> 1000000
      | Medium -> 10000000
      | Large -> 100000000
    in
    let mk_outfile size typ mk_generator =
      mk string "outfile" (sprintf "_data/%s_%s_%s.bin" typ (XList.to_string "_" (fun (k, v) -> sprintf "%s" (Env.string_of_value v)) (Params.to_env mk_generator)) size) in
    let mk_type typ = mk string "type" typ in
    let mk_n n = mk int "n" n in
    let mk_size size = (mk_n (load size)) & (mk string "!size" (string_of_size size)) in
    let use_types = types_list benchmark in
    let mk_generators = generators_list benchmark in
    Params.concat (~~ List.map use_sizes (fun size ->
    Params.concat (~~ List.map use_types (fun typ ->
    let n = load size in
    Params.concat (~~ List.map (mk_generators n typ) (fun mk_generator ->
    (   mk_type typ
      & mk_size size
      & mk_generator
      & (mk_outfile (string_of_size size) typ mk_generator) )))))))

(*let graphs = ["chain"; "cube"; "grid_sq"; "paths_100_phases_1"; "paths_20_phases_1"; "paths_524288_phases_1"; "paths_8_phases_1";
    "phased_524288_single"; "phased_low_50"; "phased_mix_10"; "random_arity_100"; "random_arity_3"; "random_arity_8"; "rmat24"; "rmat27";
    "tree_2_512_1024"; "tree_2_512_512"; "tree_binary"; "tree_depth_2"]*)
let graphs = ["cube"; "rmat24"; "rmat27"]
(*["grid_sq"; "phased_524288_single"; "phased_low_50"; "phased_mix_10"; "random_arity_100"; "tree_2_512_1024";
  "paths_100_phases_1"; "cube"; "rmat24"; "rmat27"]*)

let mk_files_inputs benchmark : Params.t =
  let mk_type typ = mk string "type" typ in
  let mk_infile infile = mk string "infile" infile in
  let mk_infile2 infile2 = mk string "infile2" infile2 in
  let mk_outfile outfile = mk string "outfile" outfile in
  match benchmark with
  | "suffix_array" ->
    (mk_type "string" &
    ((mk_infile "data/chr22.dna.txt" & mk_outfile "_data/chr22.dna.bin") ++
     (mk_infile "data/etext99.txt" & mk_outfile "_data/etext99.bin") ++
     (mk_infile "data/wikisamp.xml.txt" & mk_outfile "_data/wikisamp.xml.bin")))
  | "ray_cast" ->
    (mk_type "ray_cast_test" &
    ((mk_infile "data/happyTriangles.txt" & mk_infile2 "data/happyRays.txt" & mk_outfile "_data/happy_ray_cast_dataset.bin") ++
     (mk_infile "data/angelTriangles.txt" & mk_infile2 "data/angelRays.txt" & mk_outfile "_data/angel_ray_cast_dataset.bin") ++
     (mk_infile "data/dragonTriangles.txt" & mk_infile2 "data/dragonRays.txt" & mk_outfile "_data/dragon_ray_cast_dataset.bin") ++
(*     (mk_infile "data/xyzrgb_dragon_triangles.txt" & mk_infile2 "data/xyzrgb_dragon_rays.txt" & mk_outfile "_data/xyzrgb_dragon_ray_cast_dataset.bin") ++*)
     (mk_infile "data/xyzrgb_manuscript_triangles.txt" & mk_infile2 "data/xyzrgb_manuscript_rays.txt" & mk_outfile "_data/xyzrgb_manuscript_ray_cast_dataset.bin") ++
     (mk_infile "data/turbine_triangles.txt" & mk_infile2 "data/turbine_rays.txt" & mk_outfile "_data/turbine_ray_cast_dataset.bin")
         ))
  | "loop" ->
    (mk_type "pair_int_int" &
    ((mk_infile "data/loop_109_10.txt" & mk_outfile "_data/loop_109_10.bin") ++
     (mk_infile "data/loop_109_1000.txt" & mk_outfile "_data/loop_109_1000.bin") ++
     (mk_infile "data/loop_109_30000.txt" & mk_outfile "_data/loop_109_30000.bin") ++
     (mk_infile "data/loop_109_3000000.txt" & mk_outfile "_data/loop_109_3000000.bin") ++
     (mk_infile "data/loop_109_100000000.txt" & mk_outfile "_data/loop_109_100000000.bin") ++
     (mk_infile "data/loop_1010_10.txt" & mk_outfile "_data/loop_1010_10.bin") ++
     (mk_infile "data/loop_1010_1000.txt" & mk_outfile "_data/loop_1010_1000.bin") ++
     (mk_infile "data/loop_1010_100000.txt" & mk_outfile "_data/loop_1010_100000.bin") ++
     (mk_infile "data/loop_1010_33222591.txt" & mk_outfile "_data/loop_1010_33222591.bin") ++
     (mk_infile "data/loop_1010_1000000000.txt" & mk_outfile "_data/loop_1010_1000000000.bin")))
  | "bfs" | "pbfs" ->
    (mk_type "graph" &
    ((*(mk_infile "data/3Dgrid_J_10000000.txt" & mk_outfile "_data/3Dgrid_J_10000000.bin") ++
     (mk_infile "data/randLocalGraph_J_5_10000000.txt" & mk_outfile "_data/randLocalGraph_J_5_10000000.bin") ++
     (mk_infile "data/rMatGraph_J_5_10000000.txt" & mk_outfile "_data/rMatGraph_J_5_10000000.bin") ++*)
   
     (let mk_input graph size = mk string "infile" (sprintf "/home/rainey/new-sc15-graph/graph/bench/_data/%s_%s.adj_bin" graph size) in
      let mk_output graph size = mk string "outfile" (sprintf "_data/%s_%s.bin" graph size) in
      Params.concat(~~ List.map arg_sizes (fun size ->
      Params.concat(~~ List.map graphs (fun graph ->
        (mk_input graph size & mk_output graph size)
     )))))(* ++*)

     (* real *)
(*     (mk_infile "/home/rainey/graphdata/livejournal1.adj_bin" & mk_outfile "_data/livejournal.bin") ++
     (mk_infile "/home/rainey/graphdata/europe.adj_bin" & mk_outfile "_data/europe.bin") ++
     (mk_infile "/home/rainey/graphdata/rgg.adj_bin" & mk_outfile "_data/rgg.bin") ++
     (mk_infile "/home/rainey/graphdata/delaunay.adj_bin" & mk_outfile "_data/delaunay.bin") ++
     (mk_infile "/home/rainey/graphdata/wikipedia-20070206.adj_bin" & mk_outfile "_data/wikipedia-20070206.bin") ++
     (mk_infile "/home/rainey/graphdata/twitter.adj_bin" & mk_outfile "_data/twitter.bin") ++
     (mk_infile "/home/rainey/graphdata/usa.adj_bin" & mk_outfile "_data/usa.bin") ++*)
       
       (* new *)
(*     (mk_infile "/home/rainey/new-sc15-graph/graph/bench/_data/unbalanced_tree_trunk_first_large.adj_bin" & mk_outfile "_data/unbalanced_tree_trunk_first_large.bin")*)

     (*(mk_infile "/home/rainey/pasl/graph/bench/graph/paths_2_phases_1_large.adj_bin" & mk_outfile "_data/paths_2_phases_1_large.bin") ++
     (mk_infile "/home/rainey/pasl/graph/bench/graph/paths_3percent_large.adj_bin" & mk_outfile "_data/paths_3percent_large.bin") ++
     (mk_infile "/home/rainey/pasl/graph/bench/graph/paths_5percent_large.adj_bin" & mk_outfile "_data/paths_5percent_large.bin") ++
     (mk_infile "/home/rainey/pasl/graph/bench/graph/phased_mix_100_large.adj_bin" & mk_outfile "_data/phased_mix_100_large.bin"))*)
    ))
  | _ -> Params.concat ([])

let mk_sequence_inputs benchmark : Params.t =
  (mk_generate_sequence_inputs benchmark) ++ (mk_files_inputs benchmark)

let sequence_descriptor_of mk_sequence_inputs =
   ~~ List.map (Params.to_envs mk_sequence_inputs) (fun e ->
      (Env.get_as_string e "type"),
      (Env.get_as_string e "outfile")
      )

let mk_sequence_input_names benchmark =
  let mk2 = sequence_descriptor_of (mk_sequence_inputs benchmark) in
  Params.eval (Params.concat (~~ List.map mk2 (fun (typ,outfile) ->
    (mk string "type" typ) &
    (mk string "infile" outfile)
  )))            

let mk_lib_types =
  mk_list string "lib_type" [ "pctl"; "pbbs"; ]
              
(*****************************************************************************)
(** Input-data generator *)

module ExpGenerate = struct

let name = "generate"

let prog = "./sequence_data.norm"

let make() =
  build "." [prog] arg_virtual_build

let run() =
  Mk_runs.(call (run_modes @ [
    Output (file_results name);
    Timeout 400;
    Args (
      mk_prog prog
      & (Params.concat (~~ List.map arg_benchmarks (fun benchmark ->
         mk_sequence_inputs benchmark)))
    )
  ]))

let check = nothing  (* do something here *)

let plot() = ()

let all () = select make run check plot

end

(*****************************************************************************)
(** Comparison-sort benchmark *)

module ExpEvaluate = struct

let mk_proc = mk_list int "proc" arg_proc

let prog_names = function
  | "comparison_sort" -> "samplesort"
  | "blockradix_sort" -> "blockradixsort"
  | "remove_duplicates" -> "deterministichash"
  | "suffix_array" -> "suffixarray"
  | "convex_hull" -> "quickhull"
  | "nearest_neighbours" -> "nearestneighbours"
  | "ray_cast" -> "raycast"
  | "reduce" -> "reduce"
  | "scan" -> "scan"
  | "loop" -> "loop"
  | "delaunay" -> "delaunay"
  | "delaunay_refine" -> "delaunayrefine"
  | "bfs" -> "bfs"
  | "pbfs" -> "pbfs"
  | _ -> Pbench.error "invalid benchmark"

let prog benchmark =
  sprintf "./%s_bench.%s" (prog_names benchmark) arg_extension

let make() =
  List.iter (fun benchmark ->
    build "." [prog benchmark] arg_virtual_build
  ) arg_benchmarks

let run() =
  List.iter (fun benchmark ->
    Mk_runs.(call (run_modes @ [
      Output (file_results benchmark);
      Timeout 400;
      Args (
          mk_prog ((*"numactl --interleave=all " ^*) (prog benchmark))
      & (
        mk_sequence_input_names benchmark
      )
      & mk_lib_types & mk_proc)]))
  ) arg_benchmarks

let check = nothing  (* do something here *)

let plot() =
  List.iter (fun benchmark ->
    let eval_relative = fun env all_results results ->
      let pbbs_results = ~~ Results.filter_by_params all_results (
        from_env (Env.add (Env.filter_keys ["type"; "infile"; "proc"] env) "lib_type" (Env.Vstring "pbbs"))
      ) in
      if pbbs_results = [] then Pbench.error ("no results for pbbs library");
      let v = Results.get_mean_of "exectime" results in
      let b = Results.get_mean_of "exectime" pbbs_results in
(*      print_string (Printf.sprintf "%f %f\n" v b);
      print_string ((String.concat " " (List.map string_of_float (Results.get Env.as_float "exectime" results))) ^ "\n");*)
      v /. b
      in
    Mk_bar_plot.(call ([
      Bar_plot_opt Bar_plot.([
         X_titles_dir Vertical;
         Y_axis [Axis.Lower (Some 0.)] ]);
      Formatter default_formatter;
      Charts mk_proc;
      Series mk_lib_types;
      X (mk_sequence_input_names benchmark);
      Input (file_results benchmark);
      Output (file_plots benchmark);
      Y_label "exectime";
      Y eval_relative;
    ]))
  ) arg_benchmarks

let all () = select make run check plot

end

module ExpComparison = struct

let mk_proc = mk_list int "proc" arg_proc

let prog_names = function
  | "comparison_sort" -> "samplesort"
  | "blockradix_sort" -> "blockradixsort"
  | "remove_duplicates" -> "deterministichash"
  | "suffix_array" -> "suffixarray"
  | "convex_hull" -> "quickhull"
  | "nearest_neighbours" -> "nearestneighbours"
  | "ray_cast" -> "raycast"
  | "loop" -> "loop"
  | "delaunay" -> "delaunay"
  | "delaunay_refine" -> "delaunayrefine"
  | "bfs" -> "bfs"
  | "pbfs" -> "pbfs"
  | x -> Pbench.error "invalid benchmark " ^ x

let extensions = XCmd.parse_or_default_list_string "exts" [ "manc"; "norm"; "unko"; "unke"]

let no_pbbs = XCmd.mem_flag "nopbbs"

let my_mk_progs =
(*  ((mk string "lib_type" "pbbs") & (mk string "prog" "bfs_bench.manc"))
  ++*)   ((mk string "lib_type" "pctl") & (mk string "prog" "bfs_bench.unkm30"))
         ++ ((mk string "lib_type" "pctl") & (mk string "prog" "bfs_bench.unkm100"))
         ++ ((mk string "lib_type" "pctl") & (mk string "prog" "pbfs_bench.unkm30"))
         ++ ((mk string "lib_type" "pctl") & (mk string "prog" "pbfs_bench.unkm100"))
         ++ ((mk string "lib_type" "pbbs") & (mk string "prog" "pbfs_bench.manc"))
                            
let prog benchmark extension =
  sprintf "%s_bench.%s" (prog_names benchmark) extension

let make() =
  List.iter (fun benchmark ->
    List.iter (fun extension ->
      build "." [prog benchmark extension] arg_virtual_build
    ) ("manc" :: extensions)
  ) arg_benchmarks

let mk_pctl_progs benchmark = 
  ((mk_list string "prog" (
              List.map (fun ext -> (*"numactl --interleave=all " ^ *) (prog benchmark ext)) extensions)) & (mk string "lib_type" "pctl")) ++
    (if no_pbbs then (fun e -> []) else ((mk_prog ((*"numactl --interleave=all " ^ *) (prog benchmark "manc"))) & (mk string "lib_type" "pbbs")))

let run() =
  List.iter (fun benchmark ->
    Mk_runs.(call (run_modes @ [
      Output (Printf.sprintf "_results/results_%s.txt" benchmark);
      Timeout 400;
      Args (
        (mk_pctl_progs benchmark)
      & (mk_sequence_input_names benchmark)
      & mk_proc)]))
    ) arg_benchmarks

let check = nothing

let mk_graphs = mk_list string "infile" graphfiles

let pretty_graph_name n =
  if (graphfile_of "europe") = n then
    "europe"
  else if (graphfile_of "rgg") = n then
    "rgg"
  else if (graphfile_of "twitter") = n then
    "twitter"
  else if (graphfile_of "delaunay") = n then
    "delaunay"
  else if (graphfile_of "usa") = n then
    "usa"
  else if (graphfile_of "livejournal") = n then
    "livejournal"
  else if (graphfile_of "wikipedia") = n then
    "wikipedia-2007"
  else if (graphfile_of "grid_sq_large") = n then
    "square-grid"
  else if (graphfile_of "paths_100_phases_1_large") = n then
    "par-chains-100"
  else if (graphfile_of "phased_524288_single_large") = n then
    "trees_524k"
  else if (graphfile_of "phased_low_50_large") = n then
    "phases-50-d-5"
  else if (graphfile_of "phased_mix_10_large") = n then
    "phases-10-d-2"
  else if (graphfile_of "random_arity_100_large") = n then
    "random-arity-100"
  else if (graphfile_of "tree_2_512_1024_large") = n then
    "trees-512-1024"
  else if (graphfile_of "unbalanced_tree_trunk_first_large") = n then
    "unbalanced-tree"
  else if (graphfile_of "3Dgrid_J_10000000") = n then
    "3D-grid"
  else if (graphfile_of "randLocalGraph_J_5_10000000") = n then
    "random"
  else if (graphfile_of "rMatGraph_J_5_10000000") = n then
    "rMat"
  else if (graphfile_of "cube_large") = n then
    "cube"
  else if (graphfile_of "rmat24_large") = n then
    "rmat24"
  else if (graphfile_of "rmat27_large") = n then
    "rmat27"
  else
    n

let eval_relative baseline_prog = fun env all_results results ->
  let _ = Printf.printf "p=%s\n" (Env.get_as_string env "prog") in
  let pbbs_results = ~~ Results.filter_by_params all_results (
                          from_env (Env.add (
                                        Env.add (Env.filter_keys ["type"; "infile"; "proc"] env) "lib_type" (Env.Vstring "pbbs"))
                                            "prog" (Env.Vstring baseline_prog))
                        ) in
  if pbbs_results = [] then Pbench.error ("no results for pbbs library");
  let v = Results.get_mean_of "exectime" results in
  let b = Results.get_mean_of "exectime" pbbs_results in
  100.0 *. (v /. b -. 1.0)

let eval_relative_stddev baseline_prog = fun env all_results results ->
  let pbbs_results = ~~ Results.filter_by_params all_results (
                          from_env (Env.add (
                                        Env.add (Env.filter_keys ["type"; "infile"; "proc"] env) "lib_type" (Env.Vstring "pbbs"))
                                            "prog" (Env.Vstring baseline_prog))
                        ) in
  if pbbs_results = [] then Pbench.error ("no results for pbbs library");
  try 
  let b = Results.get_mean_of "exectime" pbbs_results in
  let times = Results.get Env.as_float "exectime" results in
  let rels = List.map (fun v -> 100.0 *. (v /. b -. 1.0)) times in
  XFloat.list_stddev rels
  with Results.Missing_key _ -> nan

let eval_relative_main = fun env all_results results ->
  let pbbs_results = ~~ Results.filter_by_params all_results (
                          from_env (Env.add (Env.filter_keys ["type"; "infile"; "proc"] env) "lib_type" (Env.Vstring "pbbs"))) in
  if pbbs_results = [] then Pbench.error ("no results for pbbs library");
  let v = Results.get_mean_of "exectime" results in
  let b = Results.get_mean_of "exectime" pbbs_results in
  100.0 *. (v /. b -. 1.0)

let eval_relative_stddev_main = fun env all_results results ->
  let pbbs_results = ~~ Results.filter_by_params all_results (
                          from_env (Env.add (Env.filter_keys ["type"; "infile"; "proc"] env) "lib_type" (Env.Vstring "pbbs"))
                        ) in
  if pbbs_results = [] then Pbench.error ("no results for pbbs library");
  try 
  let b = Results.get_mean_of "exectime" pbbs_results in
  let times = Results.get Env.as_float "exectime" results in
  let rels = List.map (fun v -> 100.0 *. (v /. b -. 1.0)) times in
  XFloat.list_stddev rels
  with Results.Missing_key _ -> nan

let pretty_prog p =
  if p = "bfs_bench.unkm30" then
    "Seq. neighbor list, oracle guided, kappa := 30usec"
  else if p = "bfs_bench.unkm100" then
    "Seq. neighbor list, oracle guided, kappa := 100usec"
  else if p = "pbfs_bench.unkm30" then
    "Par. neighbor list, oracle guided, kappa := 30usec"
  else if p = "pbfs_bench.unkm100" then
    "Par. neighbor list, oracle guided, kappa := 100usec"
  else if p = "bfs_bench.manc" then
    "Seq. neighbor list, PBBS BFS"
  else if p = "pbfs_bench.manc" then
    "Par. neighbor list PBBS BFS"
  else
    p

let bfs_formatter =
 Env.format (Env.(
  [
   ("proc", Format_custom (fun n -> ""));
   ("lib_type", Format_custom (fun n -> ""));
   ("infile", Format_custom pretty_graph_name);
   ("prog", Format_custom pretty_prog);
   ]
              ))

let inputfile_of n = "_data/" ^ n ^ "_large.bin"

let pretty_input_name n =
  if (inputfile_of "array_double_random") = n then
    "random"
  else if (inputfile_of "array_double_exponential") = n then
    "exponential"
  else if (inputfile_of "array_double_almost_sorted_10000") = n then
    "almost sorted"
  else if (inputfile_of "array_string_trigrams") = n then
    "trigrams"

  else if (inputfile_of "array_int_random") = n then
    "random"
  else if (inputfile_of "array_int_exponential") = n then
    "exponential"
  else if (inputfile_of "array_pair_int_int_random_256") = n then
    "random kvp 256"
  else if (inputfile_of "array_pair_int_int_random_100000000") = n then
    "random kvp $10^6$"

  else if (inputfile_of "array_string_trigrams") = n then
    "trigrams"
  else if (inputfile_of "array_int_random") = n then
    "random"
  else if (inputfile_of "array_int_random_bounded_100000") = n then
    "random unbounded"
  else if (inputfile_of "array_int_exponential") = n then
    "exponential"

  else if "_data/chr22.dna.bin" = n then
    "dna"
  else if "_data/etext99.bin" = n then
    "text"
  else if "_data/string_trigrams_large.bin" = n then
    "trigrams"
  else if "_data/wikisamp.xml.bin" = n then
    "wiki"
      
  else if (inputfile_of "array_point2d_in_circle") = n then
    "in circle"
  else if (inputfile_of "array_point2d_kuzmin") = n then
    "kuzmin"
  else if (inputfile_of "array_point2d_on_circle") = n then
    "on circle"

  else if (inputfile_of "array_point2d_in_square") = n then
    "in square"
  else if (inputfile_of "array_point2d_kuzmin") = n then
    "kuzmin"
  else if (inputfile_of "array_point2d_in_square_delaunay") = n then
    "in square"
  else if (inputfile_of "array_point2d_kuzmin_delaunay") = n then
    "kuzmin"
  else if (inputfile_of "array_point3d_in_cube") = n then
    "in cube"
  else if (inputfile_of "array_point3d_plummer") = n then
    "plummer"
  else if (inputfile_of "array_point3d_on_sphere") = n then
    "on sphere"

  else if "_data/happy_ray_cast_dataset.bin" = n then
    "happy"
  else if "_data/angel_ray_cast_dataset.bin" = n then
    "angel"
  else if "_data/dragon_ray_cast_dataset.bin" = n then
    "dragon"

  else if "_data/array_point2d_in_square_medium.bin" = n then
    "in square"
  else if "_data/array_point2d_kuzmin_medium.bin" = n then
    "kuzmin"

  else if "_data/triangles_point2d_delaunay_in_square_refine_large.bin" = n then
    "in square"
  else if "_data/triangles_point2d_delaunay_kuzmin_refine_large.bin" = n then
    "kuzmin"

  else if "_data/cube_large.bin" = n then
    "cube-grid"
  else if "_data/rmat24_large.bin" = n then
    "rMat24"
  else if "_data/rmat27_large.bin" = n then
    "rMat27"
      
  else
    n
            
let main_formatter =
 Env.format (Env.(
  [
   ("proc", Format_custom (fun n -> ""));
   ("lib_type", Format_custom (fun n -> ""));
   ("infile", Format_custom pretty_input_name);
   ("prog", Format_custom (fun n -> ""));
   ("type", Format_custom (fun n -> ""));
   ]
  ))


let plot() = (
  (* BFS plotting *) (*
  let baseline_bfs_prog = "bfs_bench.manc" in
  Mk_bar_plot.(call ([
                      Chart_opt Chart.([
            Legend_opt Legend.([
               Legend_pos Top_left
               ])]);
      Bar_plot_opt Bar_plot.([
                              Chart_opt Chart.([Dimensions (13.,8.) ]);
         X_titles_dir Vertical;
         Y_axis [Axis.Lower (Some (-100.0)); Axis.Upper (Some (100.0))] ]);
      Formatter bfs_formatter;
      Charts mk_proc;
      Series my_mk_progs;
      X mk_graphs;
      Input "_results/results_bfs_main.txt";
      Output "plots_bfs.pdf";
      Y_label "% relative to original PBBS BFS";
      Y (eval_relative baseline_bfs_prog);
      Y_whiskers (eval_relative_stddev baseline_bfs_prog);
                    ])); *)
  (************)
  (* Main plot *)
  (*
  let mk_inputs = (mk_sequence_input_names "suffix_array")
                  ++ (mk_sequence_input_names "blockradix_sort")
  in
  let mk_suffixarray =
    ((mk string "lib_type" "pctl") & (mk string "prog" "suffixarray_bench.unke100"))
    ++     ((mk string "lib_type" "pctl") & (mk string "prog" "blockradixsort_bench.unke100"))
  in
  let mk_progs = mk_suffixarray in *)

  (* 
     - manc : authors' manual control  
     - norm<x> : without bootstrapping / kappa := x
     - unke<x> : some mode that doesn't seem to be relevant anymore
     - unkm<x> : oracle guided / kappa := x
   *)

  let mk_pctl_prog benchmark extension lib_type = 
    (mk string "prog" (prog benchmark extension)) & (mk string "lib_type" lib_type)
  in

  let pretty_extension ext =
    let l = String.length ext in
    let plen = 4 in
    if l < plen then
      "<unknown extension>"
    else if l = plen then
      sprintf "Oracle guided (%d %s)" 300 "$\\mu$"
    else
      let p = String.sub ext 0 plen in
      let mu = int_of_string (String.sub ext plen (l - plen)) in
      sprintf "Oracle guided (%d %s)" mu "$\\mu$"
  in

  let nb_benchmarks = List.length arg_benchmarks in
  let experiment_name = "pbbs" in
  let nb_extensions = List.length extensions in
  let tex_file = file_tables_src experiment_name in
  let pdf_file = file_tables experiment_name in
    Mk_table.build_table tex_file pdf_file (fun add ->
      let ls = String.concat "|" (XList.init nb_extensions (fun _ -> "l")) in
      let hdr = Printf.sprintf "p{1cm}l|l|%s" ls in
      add (Latex.tabular_begin hdr);                                    
      Mk_table.cell ~escape:true ~last:false add (Latex.tabular_multicol 2 "l|" "Application/input");
      Mk_table.cell ~escape:true ~last:false add "\\begin{tabular}[x]{@{}c@{}}Time (s)\\\\original\\end{tabular}";
      ~~ List.iteri extensions (fun i ext ->
        let last = i + 1 = nb_extensions in
        let label = pretty_extension ext in
        Mk_table.cell ~escape:true ~last:last add label);
      add Latex.tabular_newline;
      ~~ List.iteri arg_benchmarks (fun benchmark_i benchmark ->
        Mk_table.cell add (Latex.tabular_multicol 2 "l|" (sprintf "\\textbf{%s}" (Latex.escape benchmark)));
        add Latex.tabular_newline;
        let results_file = Printf.sprintf "_results/results_%s.txt" benchmark in
        let all_results = Results.from_file results_file in
        let results = all_results in
        let env = Env.empty in
        let mk_rows = mk_sequence_input_names benchmark in
        let env_rows = mk_rows env in
        ~~ List.iter env_rows (fun env_rows ->  (* loop over each input for current benchmark *)
          let results = Results.filter env_rows results in
          let env = Env.append env env_rows in
          let row_title = main_formatter env_rows in
          let _ = Mk_table.cell ~escape:true ~last:false add "" in
          let _ = Mk_table.cell ~escape:true ~last:false add row_title in
          let pbbs_str = 
            let [col] = (mk_pctl_prog benchmark "manc" "pbbs") env in
            let env = Env.append env col in
            let results = Results.filter col results in
            let v = eval_exectime env all_results results in
            let e = eval_exectime_stddev env all_results results in 
            Printf.sprintf "%.3f (%.2f%s) " v e "$\\sigma$"
          in
          let _ = Mk_table.cell ~escape:false ~last:false add pbbs_str in
          ~~ List.iteri extensions (fun i ext ->
            let last = i + 1 = nb_extensions in
            let pctl_str = 
              let [col] = (mk_pctl_prog benchmark ext "pctl") env in
              let env = Env.append env col in
              let results = Results.filter col results in
              let v = eval_relative_main env all_results results in
              let s = if v < 0.0 then "" else "+" in
              let e = eval_exectime_stddev env all_results results in 
              Printf.sprintf "%s%.2f%s (%.2f%s)" s v "\\%" e "$\\sigma$"
            in
            Mk_table.cell ~escape:false ~last:last add pctl_str);
          add Latex.tabular_newline);
        ());
      add Latex.tabular_end;
      add Latex.new_page;
      ());
                         
  List.iter (fun benchmark -> 
    Mk_bar_plot.(call ([
             Chart_opt Chart.([
              Legend_opt Legend.([
                 Legend_pos Top_left
                 ])]);
        Bar_plot_opt Bar_plot.([
                                Chart_opt Chart.([Dimensions (6.,8.) ]);
           X_titles_dir Vertical;
           Y_axis [Axis.Lower (Some (-40.0)); Axis.Upper (Some (40.0))] ]);
        Formatter main_formatter;
        Charts mk_proc;
        Series (mk_sequence_input_names benchmark);
      X (mk_pctl_progs benchmark);
      Input (Printf.sprintf "_results/results_%s.txt" benchmark);
      Output (Printf.sprintf "plots_%s.pdf" benchmark);
        Y_label "% relative to original PBBS";
        Y eval_relative_main;
        Y_whiskers eval_relative_stddev_main;
                      ])) ) arg_benchmarks;
 ()  
)

let all () = select make run check plot

end

(*****************************************************************************)
(** Main *)

let _ =
  let arg_actions = XCmd.get_others() in
  let bindings = [
    "generate", ExpGenerate.all;
    "evaluate", ExpEvaluate.all;
    "compare",  ExpComparison.all;
  ]
  in
  system("mkdir -p _data") arg_virtual_run;
  system("mkdir -p _plots") arg_virtual_run;
  Pbench.execute_from_only_skip arg_actions [] bindings;
  ()
