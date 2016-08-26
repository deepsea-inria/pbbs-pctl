split=(${1//./ })

name=${split[0]}
ext=${split[1]}

cmdline="g++ -std=gnu++11 -O3 -I ~/pctl/include -I ~/chunkedseq/include -I ~/pbbs-pctl/include -I ~/quickcheck/quickcheck -I ~/cmdline/include -I ~/pbbs-pctl/bench/include -I ~/pbbs-pctl/bench/include/generators -I ~/pbbs-include/"
#cmdline="g++ -std=gnu++11 -O0 -finline-limit=100000000 -findirect-inlining -fpartial-inlining -finline-functions -finline-functions-called-once -fmerge-constants -falign-functions -fthread-jumps -fdelete-null-pointer-checks -finline-small-functions -foptimize-sibling-calls -freorder-functions  -I ~/pctl/include -I ~/chunkedseq/include -I ~/pbbs-pctl/include -I ~/quickcheck/quickcheck -I ~/cmdline/include -I ~/pbbs-pctl/bench/include -I ~/pbbs-pctl/bench/include/generators -I ~/pbbs-include/"
#-g

if [[ $ext == "unkh" ]];
then
#  cmdline="${cmdline} -DESTIMATOR_LOGGING -DHONEST -DPCTL_CILK_PLUS -fcilkplus"
#  cmdline="${cmdline} -DESTIMATOR_LOGGING -DHONEST -DTIMING -DPCTL_CILK_PLUS -fcilkplus"
  cmdline="${cmdline} -DTIMING -DHONEST -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
# -nostdlibs -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unko" ]];
then
#  cmdline="${cmdline} -DESTIMATOR_LOGGING -DTIMING -DOPTIMISTIC -DPCTL_CILK_PLUS -fcilkplus"
#  cmdline="${cmdline} -DOPTIMISTIC -DTIMING -DPCTL_CILK_PLUS -fcilkplus"
  cmdline="${cmdline} -DTIMING -DTHREADS -DOPTIMISTIC -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unke" ]];
then
  cmdline="${cmdline} -DTIMING -DTHREADS -DEASYOPTIMISTIC -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unkp" ]];
then
  cmdline="${cmdline} -DPRUNING -DTIMING -DTHREADS -DEASYOPTIMISTIC -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unkt" ]];
then
  cmdline="${cmdline} -DPRUNING -DTIMING -DTHREADS -DEASYOPTIMISTIC -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "norm" ]];
then
#  cmdline="${cmdline} -DESTIMATOR_LOGGING -DPCTL_CILK_PLUS -fcilkplus"
# -DMANUAL_ALLOCATION -DMANUAL_CONTROL
  cmdline="${cmdline} -DMANUAL_ALLOCATION -DTHREADS -DTIMING -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
# -DTIME_MEASURE
# -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
#  cmdline="${cmdline} -DTIMING -DPCTL_CILK_PLUS -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
#  cmdline="${cmdline} -DESTIMATOR_LOGGING -DPCTL_CILK_PLUS -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "manb" ]];
then
  cmdline="${cmdline} -DTIME_MEASURE -DTIMING -DMANUAL_ALLOCATION -DPBBS_SEQUENCE -DMANUAL_CONTROL -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "manc" ]];
then
  cmdline="${cmdline} -DPBBS_SEQUENCE -DMANUAL_CONTROL -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "log" ]];
then
  cmdline="${cmdline} -DTIMING -DUSE_CILK_PLUS_RUNTIME -DMANUAL_CONTROL -fcilkplus -DPLOGGING -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "norml" ]];
then
  cmdline="${cmdline} -DTIMING -DTIME_MEASURE -DPLOGGING -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unkol" ]];
then
  cmdline="${cmdline} -DTIMING -DOPTIMISTIC -DTIME_MEASURE -DPLOGGING -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "par" ]];
then
  cmdline="${cmdline} -DPCTL_CILK_PLUS -fcilkplus -DPCTL_PARALLEL_ELISION"
fi

if [[ $ext == "seqel" ]];
then
  cmdline="${cmdline} -DPCTL_SEQUENTIAL_ELISION"
fi

if [[ $ext == "seq" ]];
then
  cmdline="${cmdline} -DPCTL_SEQUENTIAL_BASELINE"
fi

rm ${name}.${ext}
eval ${cmdline} ${name}.cpp -o ${name}.${ext}
