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

##################################################
## MAIN ##
##################################################

if __name__ == "__main__":
    init_modules()

    #Init parameters
    host_cmd_version = subprocess.check_output(["./tpt_host", "-v"], universal_newlines=True)
    mic_cmd_version = subprocess.check_output(["micnativeloadex", "tpt_mic", "-a", "-v"], universal_newlines=True)

    node_type = "scif"
    experiment = "mem_unreg"
    process_timeout = 20 #in seconds
    chunk_sizes = list(map(lambda x: 2**x, range(12, 27)))
    repetitions = 1000
    total_size_limit = 2**30
    #TODO: host-MIC vs send-recv
    mic_to_host = True
    params = 'chunk_size: ' + str(chunk_sizes) +\
        ' trans_proto: ' + node_type +\
        ' experiment: ' + experiment +\
        ' repetitions: ' + str(repetitions) +\
        ' total_size_limit: ' + str(total_size_limit) +\
        ' mic_to_host: ' + str(mic_to_host) +\
        '\n' + host_cmd_version + '\n' +\
        mic_cmd_version + '\n'

    sender_df = pd.DataFrame(index=range(0, repetitions))
    receiver_df = pd.DataFrame(index=range(0, repetitions))

    logging.info('======= experiment parameters ======= \n'+\
                 'process_timeout = %d\n' + params +\
                 '======================================================= \n',\
                 process_timeout)

    for chunk_size in chunk_sizes:
        total_size = chunk_size * repetitions
        port = random.randint(2000, 6000)

        if total_size > total_size_limit:
            total_size = total_size_limit

        # Cook the commands
        receiver_cmd = ["./tpt_host", \
                    "-t", node_type, \
                    "-p", str(port), \
                    "-s", str(total_size), \
                    "-c", str(chunk_size), \
                    "-u", str(repetitions), \
                    "-e", experiment]


        sender_cmd = ["micnativeloadex", "tpt_mic", \
                   "-d", "0", \
                   "-a"]
        sender_pars = "\' -n 0 -t " + node_type + \
                   " -p " + str(port) + \
                   " -s " + str(total_size) + \
                   " -c " + str(chunk_size) + \
                   " -e " + experiment + \
                   " -u " + str(repetitions) + "\'"
        sender_cmd.append(sender_pars)

        logging.info(str(sender_cmd) + str(receiver_cmd) + '\n')
        
        # RUN
        try:
            mic_data, host_data = run_trans_perf_test(sender_cmd, receiver_cmd)
            logging.info('Success[%d]', chunk_size)
        except PairProcessAbort as e:
            logging.exception('PairProcessAbort: %s :: %s', sender_cmd[chunk_size], receiver_cmd[chunk_size])
            break;
        else:
            sender_df[chunk_size] = pd.Series(mic_data, index=sender_df.index)
            receiver_df[chunk_size] = pd.Series(host_data, index=receiver_df.index)



### Storing permanently data ###

    data_root = '../../data/'+os.uname().nodename+"/" + node_type + "/"
    TS = get_timestamp()

    # TODO: change host/mic to sender/receiver
    sender_df.to_csv(data_root+"sender_"+TS+".csv")
    receiver_df.to_csv(data_root+"receiver_"+TS+".csv")

    # TODO: file not found
    with open(data_root+"METADATA.txt", "a") as f:
        f.write("\n################################\n")
        f.write(TS)
        f.write('tpt_host -v ' + host_cmd_version + '\n')
        f.write('tpt_mic -v ' + mic_cmd_version + '\n')
        f.write(params)
