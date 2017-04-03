split=(${1//./ })

name=${split[0]}
ext=${split[1]}

cmdline="g++ -std=gnu++11 -O2 -I ~/scratch/oracle/pctl/include -I ~/scratch/oracle/chunkedseq/include -I ~/scratch/oracle/pbbs-pctl/include -I ~/scratch/oracle/quickcheck/quickcheck -I ~/scratch/oracle/cmdline/include -I ~/scratch/oracle/pbbs-pctl/bench/include -I ~/scratch/oracle/pbbs-pctl/bench/include/generators -I ~/scratch/oracle/pbbs-include/"
#cmdline="g++ -std=gnu++11 -O0 -finline-limit=100000000 -findirect-inlining -fpartial-inlining -finline-functions -finline-functions-called-once -fmerge-constants -falign-functions -fthread-jumps -fdelete-null-pointer-checks -finline-small-functions -foptimize-sibling-calls -freorder-functions  -I ~/scratch/oracle/pctl/include -I ~/scratch/oracle/chunkedseq/include -I ~/scratch/oracle/pbbs-pctl/include -I ~/scratch/oracle/quickcheck/quickcheck -I ~/scratch/oracle/cmdline/include -I ~/scratch/oracle/pbbs-pctl/bench/include -I ~/scratch/oracle/pbbs-pctl/bench/include/generators -I ~/scratch/oracle/pbbs-include/"
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
  cmdline="${cmdline} -DTIMING -DTHREADS_CREATED -DOPTIMISTIC -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unke" ]];
then
  cmdline="${cmdline} -DTIMING -DTHREADS_CREATED -DEASYOPTIMISTIC -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unks" ]];
then
  cmdline="${cmdline} -DKAPPA300 -DSHARED -DEASYOPTIMISTIC -DTWO_MODES -DSMART_ESTIMATOR -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unkst100" ]];
then
  cmdline="${cmdline} -DEASYOPTIMISTIC -DSMART_ESTIMATOR -DKAPPA100 -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unkm" ]];
then
  cmdline="${cmdline} -DTIMING -DTHREADS_CREATED -DEASYOPTIMISTIC -DTWO_MODES -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unkm100" ]];
then
  cmdline="${cmdline} -DTIMING -DPBBS_SEQUENCE -DEASYOPTIMISTIC -DKAPPA100 -DTWO_MODES -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unkp" ]];
then
  cmdline="${cmdline} -DPRUNING -DTIMING -DTHREADS_CREATED -DEASYOPTIMISTIC -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "unkt" ]];
then
  cmdline="${cmdline} -DPRUNING -DTIMING -DTHREADS_CREATED -DEASYOPTIMISTIC -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "norm" ]];
then
#  cmdline="${cmdline} -DESTIMATOR_LOGGING -DPCTL_CILK_PLUS -fcilkplus"
# -DMANUAL_ALLOCATION -DMANUAL_CONTROL
#  cmdline="${cmdline} -DMANUAL_ALLOCATION -DTHREADS -DTIMING -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
  cmdline="${cmdline} -DMANUAL_ALLOCATION -DTHREADS_CREATED -DTIMING -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG"
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
  cmdline="${cmdline} -DTHREADS_CREATED -DMANUAL_ALLOCATION -DPBBS_SEQUENCE -DMANUAL_CONTROL -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "rep" ]];
then
  cmdline="${cmdline} -DREPORTS -DUSE_CILK_PLUS_RUNTIME -fcilkplus"
# -fsanitize=address
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

echo ${cmdline}
rm ${name}.${ext}
eval ${cmdline} ${name}.cpp -o ${name}.${ext}
