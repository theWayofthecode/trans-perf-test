import argparse
import subprocess
import pandas as pd
from calendar import datetime as dt
import os
import re

# parser = argparse.ArgumentParser()
# parser.add_argument('--total-size', type=string,  help='The total data size')
# parser.add_argument('--chunk-size', type=string,  help='The total data size is devided to chunks of sizes defined by this argument')
# args = parser.parse_args()
# print(args.total_size)

PROCESS_TIMEOUT = 20 #in seconds
PORT = '5665'
CHUNK_SIZES = [128, 256]
NUM_TRANSFERS = 8


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
            (p_mic_stdout, p_mic_stderr) = p_mic.communicate(timeout=PROCESS_TIMEOUT);
            if p_mic.returncode != 0:
                p_host.kill()
                raise PairProcessAbort(p_mic.returncode, p_mic_stderr, p_host.returncode, p_host.stderr.read())
            (p_host_stdout, p_host_stderr) = p_host.communicate(timeout=PROCESS_TIMEOUT);
            if p_host.returncode != 0:
                p_mic.kill()
                raise PairProcessAbort(p_mic.returncode, p_mic_stderr, p_host.returncode, p_host_stderr)
            mic_data = list(map(int, p_mic_stdout.split()))
            host_data = list(map(int, p_host_stdout.split()))
            return mic_data, host_data
        except subprocess.TimeoutExpired as te:
            p_mic.kill()
            p_host.kill()
            raise PairProcessAbort(p_mic.returncode, p_mic.stderr.read(), p_host.returncode, p_host.stderr.read())

def get_timestamp():
    today = str(dt.datetime.today())
    return "TS_"+re.sub('[: .-]', '_', today)

if __name__ == "__main__":
    # Pandas options
    pd.set_option('display.max_colwidth', -1)
    #Sender
    sender_df = pd.DataFrame(index=range(0, NUM_TRANSFERS))
    sender_cmd = pd.Series("Command")
    sender_err = pd.DataFrame(index=["return_code", "stderr"])

    #Receiver
    receiver_df = pd.DataFrame(index=range(0, NUM_TRANSFERS))
    receiver_cmd = pd.Series("Command")
    receiver_err = pd.DataFrame(index=["return_code", "stderr"])

    trans_proto = "scif"

    for chunk_size in CHUNK_SIZES:
        total_size = chunk_size*NUM_TRANSFERS
        if total_size > 2**9:
            total_size = 2**9
        # Cook the commands
        host_cmd = ["./tpt", \
                    "-t", trans_proto, \
                    "-p", PORT, \
                    "-s", str(total_size), \
                    "-c", str(chunk_size), \
                    "-u", str(NUM_TRANSFERS)]

        mic_cmd = ["micnativeloadex", "tpt_mic", \
                   "-a", \
                   " -t "+trans_proto+ \
                   " -n 0"+ \
                   " -p " + PORT + \
                   " -s " + str(total_size) + \
                   " -c " + str(chunk_size) + \
                   " -u " + str(NUM_TRANSFERS)]

        sender_cmd[chunk_size] = str(mic_cmd)
        receiver_cmd[chunk_size] = str(host_cmd)
        
        # RUN
        try:
            mic_data, host_data = run_trans_perf_test(mic_cmd, host_cmd)
        except PairProcessAbort as e:
            print(e)
            sender_err[chunk_size] = [e.sender_returncode, e.sender_stderr]
            receiver_err[chunk_size] = [e.receiver_returncode, e.receiver_stderr]
        else:
            sender_df[chunk_size] = pd.Series(mic_data, index=sender_df.index)
            receiver_df[chunk_size] = pd.Series(host_data, index=receiver_df.index)

    host_cmd_version = subprocess.check_output(["./tpt", "-v"], universal_newlines=True)
    mic_cmd_version = subprocess.check_output(["micnativeloadex", "tpt_mic", "-a", "-v"], universal_newlines=True)

    hdfstore_root = os.uname().nodename+"/" \
                    +trans_proto+"/" \
                    +get_timestamp()+"/"

    with pd.HDFStore('trans_perf_test.h5') as store:
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

        # print("SENDER\n")
        # print(store['oplaboum2/scif/sender'])
        # print(store.get_storer('oplaboum2/scif/sender').attrs.cmds)
        # print(store.get_storer('oplaboum2/scif/sender').attrs.errors)
        # print("RECEIVER\n")
        # print(store['oplaboum2/scif/receiver'])
        # print(store.get_storer('oplaboum2/scif/receiver').attrs.errors)
        # print(store.get_storer('oplaboum2/scif/receiver').attrs.cmds)