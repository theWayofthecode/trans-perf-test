import argparse
import subprocess
import pandas as pd
from calendar import datetime as dt
import os
import re
import random
import logging

# parser = argparse.ArgumentParser()
# parser.add_argument('--total-size', type=string,  help='The total data size')
# parser.add_argument('--chunk-size', type=string,  help='The total data size is devided to chunks of sizes defined by this argument')
# args = parser.parse_args()
# print(args.total_size)


#print run sequence number
class PairProcessAbort(Exception):
    def __init__(self, sender_returncode, sender_stderr, receiver_returncode, receiver_stderr):
        self.sender_returncode = sender_returncode
        self.sender_stderr = sender_stderr
        self.receiver_returncode = receiver_returncode
        self.receiver_stderr = receiver_stderr

    def __str__(self):
        return ("\nSender: "+str(self.sender_returncode)+": "+self.sender_stderr+\
                "\nReceiver: "+str(self.receiver_returncode)+": "+self.receiver_stderr)

def run_trans_perf_test(mic_cmd, host_cmd):
    with subprocess.Popen(host_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True) as p_host,\
            subprocess.Popen(mic_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True) as p_mic:
        try:
            (p_mic_stdout, p_mic_stderr) = p_mic.communicate(timeout=process_timeout);
            if p_mic.returncode != 0:
                p_host.kill()
                raise PairProcessAbort(p_mic.returncode, p_mic_stderr, p_host.returncode, p_host.stderr.read())
            (p_host_stdout, p_host_stderr) = p_host.communicate(timeout=process_timeout);
            if p_host.returncode != 0:
                p_mic.kill()
                raise PairProcessAbort(p_mic.returncode, p_mic_stderr, p_host.returncode, p_host_stderr)
            mic_data = list(map(int, p_mic_stdout.split()))
            host_data = list(map(int, p_host_stdout.split()))
            return mic_data, host_data
        except subprocess.TimeoutExpired as te:
            logging.exception('Timeout')
            p_mic.kill()
            p_host.kill()
            raise PairProcessAbort(p_mic.returncode, p_mic.stderr.read(), p_host.returncode, p_host.stderr.read())

def get_timestamp():
    today = str(dt.datetime.today())
    return "TS_"+re.sub('[: .-]', '_', today)

def init_modules():
    logging.basicConfig(format='\n%(levelname)s[%(asctime)s]: %(message)s',
                        level=logging.DEBUG,
                        filename='trans_perf_test.log')
    pd.set_option('display.max_colwidth', -1)
    random.seed()

## MAIN ##
if __name__ == "__main__":
    init_modules()

    #Init parameters
    trans_proto = "trans4scif"
    process_timeout = 480 #in seconds
    chunk_sizes = list(map(lambda x: 2**x, range(20, 21)))
    num_of_transfers = 10
    total_size_limit = 2**30

    #Sender
    sender_df = pd.DataFrame(index=range(0, num_of_transfers))
    sender_cmd = pd.Series("Command")
    sender_err = pd.DataFrame(index=["return_code", "stderr"])

    #Receiver
    receiver_df = pd.DataFrame(index=range(0, num_of_transfers))
    receiver_cmd = pd.Series("Command")
    receiver_err = pd.DataFrame(index=["return_code", "stderr"])

    logging.info(' ======= experiment parameters ======= \n'+\
                 'trans_proto = %s\n'+\
                 'cnunk_sizes = %s\n'+\
                 'num_of_transfers = %d\n'+ \
                 'total_size_limit = %d\n'+ \
                 'process_timeout = %d\n',\
                 trans_proto, str(chunk_sizes), num_of_transfers, total_size_limit, process_timeout)

    for chunk_size in chunk_sizes:
        total_size = chunk_size * num_of_transfers
        port = random.randint(2000, 6000)

        if total_size > total_size_limit:
            total_size = total_size_limit

        # Cook the commands
        host_cmd = ["./tpt_host", \
                    "-t", trans_proto, \
                    "-p", str(port), \
                    "-s", str(total_size), \
                    "-c", str(chunk_size), \
                    "-u", str(num_of_transfers)]

        mic_cmd = ["micnativeloadex", "tpt_mic", \
                   "-a", \
                   " -t " + trans_proto + \
                   " -n 0" + \
                   " -p " + str(port) + \
                   " -s " + str(total_size) + \
                   " -c " + str(chunk_size) + \
                   " -u " + str(num_of_transfers)]

        sender_cmd[chunk_size] = str(mic_cmd)
        receiver_cmd[chunk_size] = str(host_cmd)
        
        # RUN
        try:
            mic_data, host_data = run_trans_perf_test(mic_cmd, host_cmd)
            logging.info('Success[%d]', chunk_size)
        except PairProcessAbort as e:
            logging.exception('PairProcessAbort: %s :: %s', sender_cmd[chunk_size], receiver_cmd[chunk_size])
            sender_err[chunk_size] = [e.sender_returncode, e.sender_stderr]
            receiver_err[chunk_size] = [e.receiver_returncode, e.receiver_stderr]
            break;
        else:
            sender_df[chunk_size] = pd.Series(mic_data, index=sender_df.index)
            receiver_df[chunk_size] = pd.Series(host_data, index=receiver_df.index)

    host_cmd_version = subprocess.check_output(["./tpt_host", "-v"], universal_newlines=True)
    mic_cmd_version = subprocess.check_output(["micnativeloadex", "tpt_mic", "-a", "-v"], universal_newlines=True)

    hdfstore_root = os.uname().nodename+"/" \
                    +trans_proto+"/" \
                    +get_timestamp()+"/"

    if sender_err.empty and receiver_err.empty:
        with pd.HDFStore('../../data/trans_perf_test.h5') as store:
            #Sender
            store[hdfstore_root+'sender'] = sender_df
            store.get_storer(hdfstore_root+'sender').attrs.cmds = sender_cmd
            store.get_storer(hdfstore_root+'sender').attrs.errors = sender_err
            store.get_storer(hdfstore_root+'sender').attrs.version = mic_cmd_version
            #Receiver
            store[hdfstore_root+'receiver'] = receiver_df
            store.get_storer(hdfstore_root+'receiver').attrs.cmds = receiver_cmd
            store.get_storer(hdfstore_root+'receiver').attrs.errors = receiver_err
            store.get_storer(hdfstore_root+'receiver').attrs.version = host_cmd_version