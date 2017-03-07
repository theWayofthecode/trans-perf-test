import subprocess
import pandas as pd
from calendar import datetime as dt
import os
import re
import random
import logging
from threading import Thread

process_timeout = 1800 #in seconds

def run_peer(cmd, df, chunk_size):
    with open(cmd[0]+".err", "a") as ferr:
        try:
            out = subprocess.check_output(cmd, stderr=ferr, universal_newlines=True, timeout=process_timeout)
        except Exception as e:
            logging.exception(e)
            subprocess.call(["killall", "tpt_host", "micnativeloadex"]);
        else:
            df[chunk_size] = pd.Series(list(map(int, out.split())), index=df.index)
            logging.info('Success[%d]: '+cmd[0], chunk_size)

def get_timestamp():
    today = str(dt.datetime.today())
    return "TS_"+re.sub('[: .-]', '_', today)

if __name__ == "__main__":
    logging.basicConfig(format='\n%(levelname)s[%(asctime)s]: %(message)s',
                        level=logging.DEBUG,
                        filename='trans_perf_test.log')
    pd.set_option('display.max_colwidth', -1)
    random.seed()

#####Experiment parameters
    host_cmd_version = subprocess.check_output(["./tpt_host", "-v"], universal_newlines=True)
    mic_cmd_version = subprocess.check_output(["micnativeloadex", "tpt_mic", "-a", "-v"], universal_newlines=True)
    node_type = "scif"
    host_e = "recv_thr"
    mic_e = "send_thr"
    chunk_sizes = list(map(lambda x: 2**x, range(12, 27)))
    chunk_sizes = [128, 256]
    repetitions = 5
    total_size_limit = 2**30
    params = 'chunk_size: ' + str(chunk_sizes) +\
        ' trans_proto: ' + node_type +\
        ' host_e: ' + host_e +\
        ' mic_e: ' + mic_e +\
        ' repetitions: ' + str(repetitions) +\
        ' total_size_limit: ' + str(total_size_limit) +\
        '\n' + host_cmd_version + mic_cmd_version

    mic_df = pd.DataFrame(index=range(0, repetitions))
    host_df = pd.DataFrame(index=range(0, repetitions))

    logging.info('======= experiment parameters ======= \n'+\
                 'process_timeout = %d\n' + params +\
                 '======================================================= \n',\
                 process_timeout)

#####Run experiments
    for chunk_size in chunk_sizes:
        total_size = chunk_size * repetitions
        port = random.randint(2000, 6000)
        if total_size > total_size_limit:
            total_size = total_size_limit

        host_cmd = ["./tpt_host", \
                    "-t", node_type, \
                    "-p", str(port), \
                    "-s", str(total_size), \
                    "-c", str(chunk_size), \
                    "-u", str(repetitions), \
                    "-e", host_e]
        mic_cmd = ["micnativeloadex", "tpt_mic", \
                   "-d", "0", \
                   "-a"]
        mic_params = "\' -n 0 -t " + node_type + \
                   " -p " + str(port) + \
                   " -s " + str(total_size) + \
                   " -c " + str(chunk_size) + \
                   " -e " + mic_e + \
                   " -u " + str(repetitions) + "\'"
        mic_cmd.append(mic_params)
        # logging.info(str(host_cmd) + str(mic_cmd) + '\n')

        host_t = Thread(target=run_peer, args=(host_cmd, host_df, chunk_size, ))
        mic_t = Thread(target=run_peer, args=(mic_cmd, mic_df, chunk_size, ))
        host_t.start()
        mic_t.start()
        host_t.join()
        mic_t.join()

######Storing data permanently
    data_root = '../../data/'+os.uname().nodename+"/" + node_type + "/"
    if not os.path.exists(data_root):
        os.makedirs(data_root)
    TS = get_timestamp()
    mic_df.to_csv(data_root + "mic_" + TS + ".csv")
    host_df.to_csv(data_root + "host_" + TS + ".csv")

    with open(data_root+"METADATA.txt", "a") as f:
        f.write("\n################################\n")
        f.write(TS)
        f.write('tpt_host -v ' + host_cmd_version + '\n')
        f.write('tpt_mic -v ' + mic_cmd_version + '\n')
        f.write(params)
