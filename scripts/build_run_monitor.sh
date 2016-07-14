#!/usr/bin/env bash

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

touch trans_perf_test.log
python3 trans_perf_test.py &
tail -f --pid=$! trans_perf_test.log