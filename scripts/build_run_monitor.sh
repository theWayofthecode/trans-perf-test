#!/usr/bin/env bash
cd ../
make install
cd bin/
touch trans_perf_test.log
python3 trans_perf_test.py &
tail -f --pid=$! trans_perf_test.log