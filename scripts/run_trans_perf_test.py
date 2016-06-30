import argparse
import subprocess
import pandas

# parser = argparse.ArgumentParser()
# parser.add_argument('--total-size', type=string,  help='The total data size')
# parser.add_argument('--chunk-size', type=string,  help='The total data size is devided to chunks of sizes defined by this argument')
# args = parser.parse_args()
# print(args.total_size)

PROCESS_TIMEOUT = 30 #in seconds
PORT = '5665'
TOTAL_SIZE = '1024'
CHUNK_SIZE = '128'
NUM_TRANSFERS = '8'
HOST_CMD = ["./tpt", "-t", "scif", "-p", PORT, "-s", TOTAL_SIZE, "-c", CHUNK_SIZE, "-u", NUM_TRANSFERS]
MIC_CMD = ["micnativeloadex", "tpt_mic", "-a", '\' -t scif -n 0 -p ' + PORT + ' -s ' + TOTAL_SIZE + ' -c ' + CHUNK_SIZE + ' -u ' + NUM_TRANSFERS + '\'']

#print run sequence number

def term_proc(proc):
    proc.kill()
    proc.stdout.read() #discard
    return proc.stderr.read()

def clean_term(proc_mic, proc_host):
    err_mic = term_proc(proc_mic)
    err_host = term_proc(proc_host)
    print("MIC stderror: \n"+str(err_mic))
    print("HOST stderror: \n"+str(err_host))


if __name__ == "__main__":
    with subprocess.Popen(HOST_CMD, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as p_host, \
            subprocess.Popen(MIC_CMD, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as p_mic:
        try:
            rc_mic = p_mic.wait(timeout=PROCESS_TIMEOUT)
            rc_host = p_host.wait(timeout=PROCESS_TIMEOUT)

            if rc_mic != 0 or rc_host != 0: #ERROR
                print("MIC: "+str(rc_mic))
                print("HOST: "+str(rc_host))
                clean_term(p_mic, p_host)
            else: #SUCCESS
                print("================- MIC -================")
                print(str(p_mic.stdout.read()))
                print("================- HOST -================")
                print(str(p_host.stdout.read()))

        except subprocess.TimeoutExpired as exc:
            print("command '" + str(exc.cmd[0]) + "' timed out after " + str(exc.timeout) + " seconds.")
            clean_term(p_mic, p_host)




