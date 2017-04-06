open XBase
open Params

let system = XSys.command_must_succeed_or_virtual

(*****************************************************************************)
(** Parameters *)

let arg_virtual_run = XCmd.mem_flag "virtual_run"
let arg_virtual_build = XCmd.mem_flag "virtual_build"
let arg_nb_runs = XCmd.parse_or_default_int "runs" 1
let arg_mode = Mk_runs.mode_from_command_line "mode"
let arg_skips = XCmd.parse_or_default_list_string "skip" []
let arg_onlys = XCmd.parse_or_default_list_string "only" []
let arg_max_proc = XCmd.parse_or_default_int "max_proc" 40

let run_modes =
  Mk_runs.([
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
  Printf.sprintf "results_%s.txt" exp_name

let file_plots exp_name =
  Printf.sprintf "plots_%s.pdf" exp_name

let formatter =
 Env.format (Env.(
  [
   ("n", Format_custom (fun n -> sprintf "%s" n));
   ("proc", Format_custom (fun n -> sprintf "Nb. cores %s" n));
   ]
  ))

(** Evaluation functions *)

let eval_exectime = fun env all_results results ->
  Results.get_mean_of "exectime" results

let eval_exectime_stddev = fun env all_results results ->
  Results.get_stddev_of "exectime" results

let nb_proc = arg_max_proc

let mk_proc = mk int "proc" nb_proc

let mk_numa_interleave = mk int "numa_interleave" 1
                            
let mk_numa_firsttouch = mk int "numa_interleave" 0

let mk_threshold = mk int "threshold"

let thresholds =
  let rec f n =
    if n = 0 then
      []
    else
      n :: f (n / 2)
  in
  f 10000

let single_byte_threshold = List.nth thresholds 3
    
let big_n = 804800000

let small_n = 400000

let tiny_n = 10000

let mk_algorithm = mk string "algorithm"

let mk_item_szb = mk int "item_szb"

let mk_use_hash = mk int "use_hash"

let mk_n = mk int "n"

(*****************************************************************************)
(** Single-byte experiment *)

module ExpSingleByte = struct

let name = "single_byte"

let prog = "./granularity.virtual"

let make() =
  build "." ["granularity_bench.unmk100"] arg_virtual_build

let mk_item_szb = mk int "item_szb" 1

let mk_use_hash = mk int "use_hash" 0

let thresholds = XList.take 6 thresholds

let mk_thresholds = mk_list int "threshold" thresholds

let mk_n = mk int "n" big_n
        
let mk_common = mk_n & mk_item_szb & mk_use_hash

let mk_common_parallel = mk_common & mk_numa_interleave & mk_proc
                                                            
let mk_common_sequential = mk_common & mk_numa_firsttouch

let mk_algorithm_sequential = mk_algorithm "sequential"

let mk_algorithm_parallel_without_gc = mk_algorithm "parallel_without_gc"
                                                    
let mk_algorithm_parallel_with_gc = mk_algorithm "parallel_with_gc"

let mk_sequential = mk_common_sequential & mk_algorithm_sequential

let mk_parallel_without_gc = mk_common_sequential & mk_algorithm_parallel_without_gc

let mk_parallel_with_gc = mk_common_sequential & mk_algorithm_parallel_with_gc & mk_thresholds

let mk_parallel_with_gc_maxproc = mk_common_parallel & mk_algorithm_parallel_with_gc & (mk_threshold single_byte_threshold)

let mk_parallel_without_gc_maxproc = mk_common_parallel & mk_algorithm_parallel_without_gc

let run() =
  Mk_runs.(call (run_modes @ [
    Output (file_results name);
    Timeout 400;
    Args (
      mk_prog prog 
   & (mk_sequential
   ++ mk_parallel_without_gc
   ++ mk_parallel_with_gc
   ++ mk_parallel_with_gc_maxproc
   ++ mk_parallel_without_gc_maxproc           
     ))]))

let check = nothing  (* do something here *)

let plot() = ()
   (*
    Mk_bar_plot.(call ([
      Bar_plot_opt Bar_plot.([
         X_titles_dir Vertical;
         Y_axis [ Axis.Is_log false;] ]);
      Formatter formatter;
      Charts mk_unit;
      Series mk_proc;
      X ( mk_algo_sim ++ mk_algo_sta ++ (mk_algo_dyn & mk_thresholds) );
      Input (file_results name);
      Output (file_plots name);
      Y_label "Number of operations per second per core";
      Y eval_nb_operations_per_second;
  ]))
    *)

let all () = select make run check plot

end

(*****************************************************************************)
(** Two-kilobyte experiment *)

module ExpTwoKilo = struct

let name = "two_kilo"

let prog = "./granularity.virtual"

let make() =
  build "." ["granularity_bench.unmk100"] arg_virtual_build

let mk_n = mk int "n" small_n

let mk_item_szb = mk int "item_szb" 2048

let mk_use_hash = mk int "use_hash" 1
        
let mk_common = mk_n & mk_item_szb & mk_use_hash

let mk_common_parallel = mk_common & mk_numa_interleave & mk_proc

let mk_thresholds = mk_list int "threshold" thresholds
                                                            
let mk_parallel_with_gc_maxproc = mk_common_parallel & ExpSingleByte.mk_algorithm_parallel_with_gc & mk_thresholds

let run() =
  Mk_runs.(call (run_modes @ [
    Output (file_results name);
    Timeout 400;
    Args (
      mk_prog prog 
   &  mk_parallel_with_gc_maxproc)]))

let check = nothing  (* do something here *)

let plot() = ()
   (*
    Mk_bar_plot.(call ([
      Bar_plot_opt Bar_plot.([
         X_titles_dir Vertical;
         Y_axis [ Axis.Is_log false;] ]);
      Formatter formatter;
      Charts mk_unit;
      Series mk_proc;
      X ( mk_algo_sim ++ mk_algo_sta ++ (mk_algo_dyn & mk_thresholds) );
      Input (file_results name);
      Output (file_plots name);
      Y_label "Number of operations per second per core";
      Y eval_nb_operations_per_second;
  ]))
    *)

let all () = select make run check plot

end

(*****************************************************************************)
(** Threshold experiment *)

module ExpThresholds = struct

let name = "thresholds"

let prog = "./granularity.virtual"

let make() =
  build "." ["granularity_bench.unmk100"] arg_virtual_build

let mk_thresholds = mk_list int "threshold" [1;single_byte_threshold]

let mk_common = mk_numa_interleave & mk_proc & ExpSingleByte.mk_algorithm_parallel_with_gc

let mk_with_char =
  (mk_item_szb 1) & (mk_use_hash 0) & (mk_n big_n)
                                          
let mk_with_hash_small =
    (mk_item_szb 2048) & (mk_use_hash 1) & (mk_n small_n)
                                             
let mk_with_hash_big =
    (mk_item_szb 131072) & (mk_use_hash 1) & (mk_n tiny_n)

let mk_configurations = mk_with_char ++ mk_with_hash_small ++ mk_with_hash_big
                                                             
let run() =
  Mk_runs.(call (run_modes @ [
    Output (file_results name);
    Timeout 400;
    Args (
      mk_prog prog 
   &  mk_common
   &  mk_thresholds
   &  mk_configurations)]))

let check = nothing  (* do something here *)

let plot() = ()
   (*
    Mk_bar_plot.(call ([
      Bar_plot_opt Bar_plot.([
         X_titles_dir Vertical;
         Y_axis [ Axis.Is_log false;] ]);
      Formatter formatter;
      Charts mk_unit;
      Series mk_proc;
      X ( mk_algo_sim ++ mk_algo_sta ++ (mk_algo_dyn & mk_thresholds) );
      Input (file_results name);
      Output (file_plots name);
      Y_label "Number of operations per second per core";
      Y eval_nb_operations_per_second;
  ]))
    *)

let all () = select make run check plot

end

(*****************************************************************************)
(** Oracle-guided experiment *)

module ExpOracleGuided = struct

let name = "oracle_guided"

let prog = "./granularity_bench.unks100"

let make() =
  build "." [prog] arg_virtual_build
        
let mk_common = mk_proc
        
let mk_algorithm_parallel_with_oracle_guided =
  mk string "algorithm" "parallel_with_oracle_guided"

let mk_algorithm_parallel_with_oracle_guided_and_seq_alt_body =
  mk string "algorithm" "parallel_with_oracle_guided_and_seq_alt_body"

let mk_algorithm_parallel_with_level1_reduce =
  mk string "algorithm" "parallel_with_level1_reduce"

let mk_algorithm_manual =
  ExpSingleByte.mk_algorithm_parallel_with_gc & ExpThresholds.mk_thresholds

let pretty_algorithm n =
  if n = "parallel_with_oracle_guided" then
    "BOGUS"
  else if n = "parallel_with_oracle_guided_and_seq_alt_body" then
    "Oracle guided"
  else if n = "parallel_with_level1_reduce" then
    "Reduce (level 1)"
  else if n = "parallel_with_gc" then
    "Manual granularity control"
  else
    "<unknown algorithm>"

let pretty_n n = "" (*
  let sn = int_of_string n in
  if sn = big_n then
    "large"
  else if sn = small_n then
    "medium"
  else if sn = tiny_n then
    "small"
  else
    "<unknown problem size>" *)

let pretty_item_szb n =
  if n = "1" then
    "char"
  else if n = "2048" then
    "2k char"
  else if n = "131072" then
    "100k char"
  else
    "<unknown nb char>"
      
let formatter =
 Env.format (Env.(
  [
    ("proc", Format_custom (fun n -> sprintf "Nb. cores %s" n));
    ("use_hash", Format_custom (fun n -> ""));
    ("n", Format_custom pretty_n);
    ("item_szb", Format_custom pretty_item_szb);
    ("algorithm", Format_custom pretty_algorithm);
   ]
  ))            
                                                  
let mk_configurations =
     mk_algorithm_manual
     (*  ++ mk_algorithm_parallel_with_oracle_guided*)
       ++ mk_algorithm_parallel_with_oracle_guided_and_seq_alt_body
  ++ mk_algorithm_parallel_with_level1_reduce
                                                  
let run() =
  Mk_runs.(call (run_modes @ [
    Output (file_results name);
    Timeout 400;
    Args (
      mk_prog prog
   &  mk_common
   &  mk_configurations
   &  ExpThresholds.mk_configurations)]))

let check = nothing  (* do something here *)

let plot() = 
    Mk_bar_plot.(call ([
      Bar_plot_opt Bar_plot.([
         X_titles_dir Vertical;
         Y_axis [ Axis.Is_log false; Axis.Lower (Some 0.); Axis.Upper(Some 0.6);] ]);
      Formatter formatter;
      Charts mk_unit;
      Series mk_configurations;
      X ExpThresholds.mk_configurations;
      Input (file_results name);
      Output (file_plots name);
      Y_label "Time (s)";
      Y eval_exectime;
  ]))

let all () = select make run check plot

end

(*****************************************************************************)
(** Nested Single Byte experiment *)

module ExpNestedSingleByte = struct

let name = "nested_single_byte"

let prog = "./granularity.virtual"

let make() =
  build "." ["granularity_bench.unmk100"] arg_virtual_build

let big_n = 40000
let small_n = 1500
let tiny_n = 150

let mk_item_szb = mk int "item_szb" 1

let mk_use_hash = mk int "use_hash" 0

let thresholds = XList.take 6 thresholds

let mk_thresholds = mk_list int "threshold" thresholds

let mk_n = mk int "n" big_n
        
let mk_common = mk_n & mk_item_szb & mk_use_hash

let mk_common_parallel = mk_common & mk_proc
                                                            
let mk_common_sequential = mk_common 

let mk_algorithm_sequential = mk_algorithm "sequential"

let mk_nested_parallel_with_gc = mk_algorithm "nested_parallel_with_gc"
                                                        
let run() =
  Mk_runs.(call (run_modes @ [
    Output (file_results name);
    Timeout 400;
    Args (
     mk_prog prog 
   & mk_nested_parallel_with_gc
   & mk_thresholds
      )]))

let check = nothing  (* do something here *)

let plot() = ()
   (*
    Mk_bar_plot.(call ([
      Bar_plot_opt Bar_plot.([
         X_titles_dir Vertical;
         Y_axis [ Axis.Is_log false;] ]);
      Formatter formatter;
      Charts mk_unit;
      Series mk_proc;
      X ( mk_algo_sim ++ mk_algo_sta ++ (mk_algo_dyn & mk_thresholds) );
      Input (file_results name);
      Output (file_plots name);
      Y_label "Number of operations per second per core";
      Y eval_nb_operations_per_second;
  ]))
    *)

let all () = select make run check plot

end

(*****************************************************************************)
(** Nested Oracle-guided experiment *)

module ExpNestedOracleGuided = struct

let name = "nested_oracle_guided"

let prog = "./granularity_bench.unks100"

let make() =
  build "." ["granularity_bench.unks100"] arg_virtual_build
        
let mk_common = mk_proc

let mk_algorithm_parallel_with_gc =
  mk string "algorithm" "nested_parallel_with_gc"
        
let mk_algorithm_parallel_with_level2 =
  mk string "algorithm" "nested_parallel_with_level2"

let mk_thresholds = mk_list int "threshold" [1;5000]
     
let mk_algorithm_manual =
  mk_algorithm_parallel_with_gc & mk_thresholds

let mk_with_char =
  (mk_item_szb 1) & (mk_use_hash 0) & (mk_n ExpNestedSingleByte.big_n)
                                          
let mk_with_hash_small =
    (mk_item_szb 2048) & (mk_use_hash 1) & (mk_n ExpNestedSingleByte.small_n)
                                             
let mk_with_hash_big =
    (mk_item_szb 131072) & (mk_use_hash 1) & (mk_n ExpNestedSingleByte.tiny_n)

let mk_configurations = mk_with_char ++ mk_with_hash_small ++ mk_with_hash_big

let pretty_algorithm n =
  if n = "nested_parallel_with_gc" then
    "Manual granularity control"
  else if n = "nested_parallel_with_level2" then
    "Oracle guided"
  else
    "<unknown algorithm>"

let pretty_n n = ""
(*  let sn = int_of_string n in
  if sn = ExpNestedSingleByte.big_n then
    "large"
  else if sn = ExpNestedSingleByte.small_n then
    "medium"
  else if sn = ExpNestedSingleByte.tiny_n then
    "small"
else
n *)

let pretty_item_szb n =
  let sn = int_of_string n in
  if sn = 1 then
    "char"
  else if sn = 2048 then
    "2k char"
  else if sn = 131072 then
    "100k char"
  else
    "<unknown nb char>"
      
let formatter =
 Env.format (Env.(
  [
    ("proc", Format_custom (fun n -> sprintf "Nb. cores %s" n));
    ("use_hash", Format_custom (fun n -> ""));
    ("n", Format_custom pretty_n);
    ("item_szb", Format_custom pretty_item_szb);
    ("algorithm", Format_custom pretty_algorithm);
   ]
  ))            
                                                  
let mk_algorithms =
     mk_algorithm_manual
  ++ mk_algorithm_parallel_with_level2
                                                  
let run() =
  Mk_runs.(call (run_modes @ [
    Output (file_results name);
    Timeout 400;
    Args (
      mk_prog prog
   &  mk_common
   & mk_algorithms
   &  mk_configurations)]))

let check = nothing  (* do something here *)

let plot() = 
    Mk_bar_plot.(call ([
      Chart_opt Chart.([
            Legend_opt Legend.([
               Legend_pos Top_right
               ])]);
      Bar_plot_opt Bar_plot.([
         X_titles_dir Vertical;
         Y_axis [ Axis.Is_log false; Axis.Lower (Some 0.); Axis.Upper(Some 1.6);] ]);
      Formatter formatter;
      Charts mk_unit;
      Series mk_algorithms;
      X mk_configurations;
      Input (file_results name);
      Output (file_plots name);
      Y_label "Time (s)";
      Y eval_exectime;
  ]))

let all () = select make run check plot

end

(*****************************************************************************)
(** Merkle Tree experiment *)
(* now defunct; see new version in bench.ml *)
                                 
(*
module ExpMerkleTree = struct

let name = "merkletree"

let oracle_prog = "merkletree_bench.unks100"

let manual_prog = "merkletree_bench.manc"

let make() =
  build "." [oracle_prog; manual_prog;] arg_virtual_build

let mk_digests = mk_list string "digest" ["pbbs32";"sha256";"sha384";"sha512";]

let mk_proc = mk int "proc" 40

let mk_parallel_common = mk_proc & (mk string "algorithm" "parallel")

let mk_sequential_common = mk string "algorithm" "sequential"

let mk_block_szb_lg n = mk int "block_szb_lg" n

let mk_nb_blocks_lg n = mk int "nb_blocks_lg" n

let sizes = [(9, 21); (13, 17); (*(17, 13);*) (18, 12); (24, 6)]

let mk_sizes =
  let mks = List.map (fun (bs, nbs) -> (mk_block_szb_lg bs) & (mk_nb_blocks_lg nbs)) sizes in
  List.fold_left (fun acc x -> x ++ acc) (List.hd mks) (List.tl mks)

let mk_progs =
  (mk_prog oracle_prog) ++ (mk_prog manual_prog)
                                                        
let run() =
  Mk_runs.(call (run_modes @ [
    Output (file_results name);
    Timeout 400;
    Args (
     mk_progs
   & mk_digests
   & mk_sizes
   & mk_parallel_common
      )]))

let check = nothing  (* do something here *)

let formatter =
 Env.format (Env.(
  [
    ("proc", Format_custom (fun n -> sprintf "Nb. cores %s" n));
    ("use_hash", Format_custom (fun n -> ""));
    ("digest", Format_custom (function | "pbbs32" -> "PBBS hash" | "sha256" -> "SHA256" | "sha384" -> "SHA384" | "sha512" -> "SHA512" | _ -> ""));
    ("mode", Format_custom (fun n -> "")(*(fun n -> if n = "manual" then "cilk_for" else "oracle guided")*));
    ("prog", Format_custom (fun n ->
                             if n = oracle_prog then
                               "cilk_for"
                             else
                               "oracle guided"));
    ("block_szb_lg", Format_custom (fun n -> sprintf "B=2^%s" n));
    ("nb_blocks_lg", Format_custom (fun n -> sprintf "N=2^%s" n));
   ]
  ))            

let eval_relative = fun env all_results results ->
  let cilk_results = ~~ Results.filter_by_params all_results (
                          from_env (Env.add (Env.filter_keys ["digest"; "block_szb_lg"; "nb_blocks_lg"] env) "prog" (Env.Vstring manual_prog))) in
  if cilk_results = [] then Pbench.error ("no results for manual mode");
  let v = Results.get_mean_of "exectime" results in
  let b = Results.get_mean_of "exectime" cilk_results in
  100.0 *. (v /. b -. 1.0)

let eval_relative_stddev = fun env all_results results ->
  let cilk_results = ~~ Results.filter_by_params all_results (
                          from_env (Env.add (Env.filter_keys ["digest"; "block_szb_lg"; "nb_blocks_lg"] env) "prog" (Env.Vstring manual_prog))) in
  if cilk_results = [] then Pbench.error ("no results from manual mode");
  try
  let b = Results.get_mean_of "exectime" cilk_results in
  let times = Results.get Env.as_float "exectime" results in
  let rels = List.map (fun v -> 100.0 *. (v /. b -. 1.0)) times in
  XFloat.list_stddev rels
  with Results.Missing_key _ -> nan

let plot() =
  Mk_bar_plot.(call ([
      Chart_opt Chart.([
            Legend_opt Legend.([
               Legend_pos Bottom_left
               ])]);
      Bar_plot_opt Bar_plot.([
         X_titles_dir Vertical;
         Y_axis [ Axis.Is_log false; Axis.Lower (Some (-100.0)); Axis.Upper(Some (30.0));] ]);
      Formatter formatter;
      Charts mk_unit;(*mk_digests;*)
      Series (mk_progs & mk_digests);
      X mk_sizes;
      Input (file_results name);
      Output (file_plots name);
      Y_label "% relative to algorithm with cilk_for";
      Y eval_relative;
      Y_whiskers eval_relative_stddev;
  ]))


let all () = select make run check plot

end
 *)
(*****************************************************************************)
(** Main *)

let _ =
  let arg_actions = XCmd.get_others() in
  let bindings = [
    "single_byte", ExpSingleByte.all;
    "two_kilo", ExpTwoKilo.all;
    "thresholds", ExpThresholds.all;
    "oracle_guided", ExpOracleGuided.all;
    "nested_single_byte", ExpNestedSingleByte.all;
    "nested_oracle_guided", ExpNestedOracleGuided.all;
    (*    "merkletree", ExpMerkleTree.all;*)
  ]
  in
  Pbench.execute_from_only_skip arg_actions [] bindings;
  ()
