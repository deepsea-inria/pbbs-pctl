open XBase
open Params

let system = XSys.command_must_succeed_or_virtual

(*****************************************************************************)
(** Parameters *)

let arg_virtual_run = XCmd.mem_flag "virtual_run"
let arg_virtual_build = XCmd.mem_flag "virtual_build"
let arg_nb_runs = XCmd.parse_or_default_int "runs" 1
let arg_mode = "replace"   (* later: document the purpose of "mode" *)
let arg_skips = XCmd.parse_or_default_list_string "skip" []
let arg_onlys = XCmd.parse_or_default_list_string "only" []
let arg_sizes = XCmd.parse_or_default_list_string "size" ["all"]
                                                  
let run_modes =
  Mk_runs.([
    Mode (mode_of_string arg_mode);
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

(** Evaluation functions *)

let eval_exectime = fun env all_results results ->
   Results.get_mean_of "exectime" results

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

let use_sizes =
   List.map size_of_string arg_sizes

let mk_sizes =
   mk_list string "size" (List.map string_of_size use_sizes)
                       
let mk_generator generator = mk string "generator" generator

let types_list = function
  | "comparison_sort" -> [ "array_double"; "array_string"; ]
  | "blockradix_sort" -> [ "array_int"; "array_pair_int_int"; ]
  | "remove_duplicates" -> [ "array_int"; "array_string"; "array_pair_string_int"; ]
  | "suffix_array" -> [ "string"; ]
  | "convex_hull" -> [ "array_point2d"; ]
  | "nearest_neighbours" -> [ "array_point2d"; "array_point3d"; ]
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
        ((mk_generator "random") & (mk_list int "m2" [ 256; ]));
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
    | "array_pair_string_int" -> [
        mk_generator "trigrams";
      ]
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
  | _ -> Pbench.error "invalid benchmark"

let mk_generate_sequence_inputs benchmark : Params.t =
  let load = function
    | Small -> 1000000
    | Medium -> 10000000
    | Large -> 100000000
  in
  let mk_outfile size typ generator =
    mk string "outfile" (sprintf "_data/%s_%s_%s.bin" typ generator size)
  in
  let mk_type typ = mk string "type" typ in
  let mk_n n = mk int "n" n in
  let mk_size size = (mk_n (load size)) & (mk string "!size" (string_of_size size)) in
  let use_types = types_list benchmark in
  let mk_generators = generators_list benchmark in
  Params.concat (~~ List.map use_sizes (fun size ->
  Params.concat (~~ List.map use_types (fun typ ->
  let n = load size in
  Params.concat (~~ List.map (mk_generators n typ) (fun mk_generator ->
  let generator = Env.get_as_string (Params.to_env mk_generator) "generator" in
  (   mk_type typ
    & mk_size size
    & mk_generator
    & mk_outfile (string_of_size size) typ generator )))))))

let sequence_descriptor_of mk_generatesequence_inputs =
   ~~ List.map (Params.to_envs mk_generatesequence_inputs) (fun e ->
      (Env.get_as_string e "type"),
      (Env.get_as_string e "generator"),
      (Env.get_as_string e "!size"),
      (Env.get_as_string e "outfile")
      )

let mk_sequence_inputs benchmark =
  let mk2 = sequence_descriptor_of (mk_generate_sequence_inputs benchmark) in
  Params.eval (Params.concat (~~ List.map mk2 (fun (typ,generator,size,outfile) ->
    mk string "infile" outfile
  )))                                                            

let mk_lib_types =
  mk_list string "lib_type" [ "pctl"; "pbbs"; ]
              
(*****************************************************************************)
(** Input-data generator *)

module ExpGenerate = struct

let name = "generate"

let prog = "./input_data.opt"

let make() =
  build "." [prog] arg_virtual_build

let run() =
  Mk_runs.(call (run_modes @ [
    Output (file_results name);
    Timeout 400;
    Args (
      mk_prog prog
    & (mk_generate_sequence_inputs "nearest_neighbours"))]))

let check = nothing  (* do something here *)

let plot() = ()

let all () = select make run check plot

end

(*****************************************************************************)
(** Comparison-sort benchmark *)

module ExpComparisonSort = struct

let name = "comparison_sort"

let prog = "./comparison_sort.opt"

let make() =
  build "." [prog] arg_virtual_build

let run() =
  Mk_runs.(call (run_modes @ [
    Output (file_results name);
    Timeout 400;
    Args (
      mk_prog prog
    & (mk_sequence_inputs "comparison_sort")
    & mk_lib_types)]))

let check = nothing  (* do something here *)

let plot() =
  Mk_bar_plot.(call ([
      Bar_plot_opt Bar_plot.([
         X_titles_dir Vertical;
         Y_axis [Axis.Lower (Some 0.)] ]);
      Formatter default_formatter;
      Charts mk_unit;
      Series mk_lib_types;
      X (mk_sequence_inputs "comparison_sort");
      Input (file_results name);
      Output (file_plots name);
      Y_label "exectime";
      Y eval_exectime;
  ]))

let all () = select make run check plot

end

(*****************************************************************************)
(** Main *)

let _ =
  let arg_actions = XCmd.get_others() in
  let bindings = [
    "generate", ExpGenerate.all;
    "comparison_sort", ExpComparisonSort.all;
  ]
  in
  system("mkdir -p _data") arg_virtual_run;
  Pbench.execute_from_only_skip arg_actions [] bindings;
  ()
