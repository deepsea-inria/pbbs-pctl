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
    
let big_n = 204800000

let small_n = 100000

let tiny_n = 1000

(*****************************************************************************)
(** Single-byte experiment *)

module ExpSingleByte = struct

let name = "single_byte"

let prog = "./granularity.virtual"

let make() =
  build "." ["granularity.unke100"] arg_virtual_build

let mk_item_szb = mk int "item_szb" 1

let mk_use_hash = mk int "use_hash" 0

let thresholds = XList.take 6 thresholds

let mk_thresholds = mk_list int "threshold" thresholds

let mk_n = mk int "n" big_n
        
let mk_common = mk_n & mk_item_szb & mk_use_hash

let mk_common_parallel = mk_common & mk_numa_interleave & mk_proc
                                                            
let mk_common_sequential = mk_common & mk_numa_firsttouch

let mk_algorithm a = mk string "algorithm" a

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
  build "." ["granularity.unke100"] arg_virtual_build

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
  build "." ["granularity.unke100"] arg_virtual_build

let mk_item_szb = mk int "item_szb"

let mk_use_hash = mk int "use_hash"

let mk_n = mk int "n"

let mk_nb_hash_iters = mk int "nb_hash_iters"

let mk_thresholds = mk_list int "threshold" [1;single_byte_threshold]

let mk_common = mk_numa_interleave & mk_proc & ExpSingleByte.mk_algorithm_parallel_with_gc

let mk_with_char =
  (mk_item_szb 1) & (mk_use_hash 0) & (mk_n big_n)
                                          
let mk_with_cheap_hash =
    (mk_item_szb 2048) & (mk_use_hash 1) & (mk_n small_n)
                                             
let mk_with_expensive_hash =
    (mk_item_szb 2048) & (mk_use_hash 1) & (mk_n tiny_n) & (mk_nb_hash_iters 1000000)

let mk_configurations = mk_with_char ++ mk_with_cheap_hash ++ mk_with_expensive_hash
                                                             
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

let prog = "./granularity.virtual"

let make() =
  build "." ["granularity.unke100"] arg_virtual_build
        
let mk_common = mk_numa_interleave & mk_proc
        
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
    "Oracle guided"
  else if n = "parallel_with_oracle_guided_and_seq_alt_body" then
    "Oracle guided + alt seq body"
  else if n = "parallel_with_level1_reduce" then
    "Reduce (level 1)"
  else if n = "parallel_with_gc" then
    "Manual granularity control"
  else
    "<unknown algorithm>"

let pretty_n n =
  let sn = int_of_string n in
  if sn = big_n then
    "large"
  else if sn = small_n then
    "medium, hash"
  else if sn = tiny_n then
    "small"
  else
    "<unknown problem size>"

let pretty_item_szb n =
  if n = "1" then
    "char"
  else if n = "2048" then
    "2k char"
  else
    "<unknown nb char>"
      
let formatter =
 Env.format (Env.(
  [
    ("proc", Format_custom (fun n -> sprintf "Nb. cores %s" n));
    ("use_hash", Format_custom (fun n -> ""));
    ("n", Format_custom pretty_n);
    ("nb_hash_iters", Format_custom (fun n -> "slow hash"));
    ("item_szb", Format_custom pretty_item_szb);
    ("algorithm", Format_custom pretty_algorithm);
   ]
  ))            
                                                  
let mk_configurations =
     mk_algorithm_manual
  ++ mk_algorithm_parallel_with_oracle_guided
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
(** Main *)

let _ =
  let arg_actions = XCmd.get_others() in
  let bindings = [
    "single_byte", ExpSingleByte.all;
    "two_kilo", ExpTwoKilo.all;
    "thresholds", ExpThresholds.all;
    "oracle_guided", ExpOracleGuided.all;
  ]
  in
  Pbench.execute_from_only_skip arg_actions [] bindings;
  ()
