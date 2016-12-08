#!/usr/bin/env bash

set -e

function run {
    touch trans_perf_test.log
    python3 trans_perf_test.py
    #tail -f --pid=$! trans_perf_test.log
}

function rebuild_t4s {
    cd ../../../Trans4SCIF/build
    rm -rf k1om
    rm -rf x86_64
    T4S_RECV_BUF_SIZE=$1 sh build_host_mic.sh
    cd ../../trans-perf-test/build/bin
}

if [ ! -d "../x86_64" ]; then
  cd ../
  mkdir x86_64
  cd x86_64
  cmake ../../ -DCMAKE_INSTALL_PREFIX=${PWD}
  cd ../bin
fi

if [ ! -d "../k1om" ]; then
  cd ../
  mkdir k1om
  cd k1om
  cmake ../../ -DCMAKE_INSTALL_PREFIX=${PWD}  -DCMAKE_CXX_COMPILER=/usr/linux-k1om-4.7/bin/x86_64-k1om-linux-g++ -DCMAKE_C_COMPILER=/usr/linux-k1om-4.7/bin/x86_64-k1om-linux-gcc
  cd ../bin
fi

cd ../x86_64
make install
cd ../bin
mv tpt tpt_host

cd ../k1om
make install
cd ../bin
mv tpt tpt_mic

#cp ../../scripts/build_run_monitor.sh ./
cp ../../scripts/trans_perf_test.py ./
cp ../../scripts/h5file.py ../../data/

#for i in `seq 5 15`;
#do
#    rebuild_t4s $((2**i))
#    run
#done

