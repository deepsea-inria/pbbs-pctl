split=(${1//./ })

name=${split[0]}
ext=${split[1]}

cmdline="g++ -std=gnu++11 -O2 -I ~/pctl/include -I ~/chunkedseq/include -I ~/pbbs-pctl/include -I ~/quickcheck/quickcheck -I ~/cmdline/include -I ~/pbbs-pctl/bench/include -I ~/pbbs-pctl/bench/include/generators -I ~/pbbs-include/"
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
  cmdline="${cmdline} -DTIMING -DOPTIMISTIC -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -DNDEBUG -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "norm" ]];
then
#  cmdline="${cmdline} -DESTIMATOR_LOGGING -DPCTL_CILK_PLUS -fcilkplus"
# -DMANUAL_ALLOCATION -DMANUAL_CONTROL
  cmdline="${cmdline} -DTIMING -DTIME_MEASURE -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
# -DTIME_MEASURE
# -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
#  cmdline="${cmdline} -DTIMING -DPCTL_CILK_PLUS -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
#  cmdline="${cmdline} -DESTIMATOR_LOGGING -DPCTL_CILK_PLUS -fcilkplus -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "manb" ]];
then
  cmdline="${cmdline} -DPBBS_SEQUENCE -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
fi

if [[ $ext == "manb" ]];
then
  cmdline="${cmdline} -DUSE_CILK_PLUS_RUNTIME -fcilkplus -DNDEBUG -ltcmalloc -L/home/rainey/Installs/gperftools/lib/"
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
