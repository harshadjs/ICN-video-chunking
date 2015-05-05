#!/usr/bin/python3
import os
import sys
import errno
import shutil
import argparse
import graph

chunk_sizes = [128, 256, 512, 1024, 1940, 4096, 6144, 8192]
#chunk_sizes = [2000]

exclude_scenarios = ["scenario-1", "scenario-2", "simple-tree"]

def draw_graphs(results_path):
    graph.draw(results_path)

def log(string, end = "\n"):
    print(string, end = end, flush = True)

def mkdir(path):
    try:
        os.mkdir(path)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise e


def run_scenario(exp_result_path, scenario, chunk_size):
    chunk_exp_result_path = exp_result_path + "/" + str(chunk_size)

    log("Running [%s@%d] ... " % (scenario, chunk_size), end = "")
    with open(ndn_app_path + "/chunk.conf", "w") as fobj:
        fobj.write("%d" % chunk_size)
    fobj.close()

    cmd = "NS_LOG=icnVideoChunkingClient:icnVideoChunkingServer " + \
          ndn_path + "/ns-3/waf --run=" + scenario
    if verbose == 0:
        cmd = cmd + " > /tmp/waf.log 2>&1"

    os.system(cmd)
    mkdir(chunk_exp_result_path)
    log("\t[DONE]")
    os.system("mv /tmp/waf.log " + chunk_exp_result_path \
              + "/waf-%s-%s.log" % (scenario, chunk_size))

    os.system("mv " + ns3_dir + "/cs-trace-router*.txt " + chunk_exp_result_path)
    os.system("mv " + ns3_dir + "/client*logs.txt " + chunk_exp_result_path)
    os.system("mv " + ns3_dir + "/server*logs.txt " + chunk_exp_result_path)
    os.system("mv " + ns3_dir + "/pit-log.txt " + chunk_exp_result_path)


def run_experiment(num, alpha, num_videos, max_size):
    print("New experiment started with configuration ", end = "")
    print("[alpha = " + str(alpha) + ", num = " + str(num_videos) + \
          ", siz = " + str(max_size) + "]")
    os.system(ndn_app_path + "generate_config_files.py -a " \
              + str(alpha) + " -n " + str(num_videos) + " -s " + str(max_size) \
              + " -c 5")

    for filename in os.listdir(scenarios_path):
        if filename in exclude_scenarios:
            log("Skipping " + filename)
            continue

        exp_result_path = results_path + "/" + str(num)
        mkdir(exp_result_path)
        shutil.copy(ndn_app_path + "videos.conf", exp_result_path)
        with open(exp_result_path + "/experiment.conf", 'w') as fobj:
            fobj.write("alpha = " + str(alpha) + "\n")
            fobj.write("num_videos = " + str(num_videos) + "\n")
            fobj.write("max_size = " + str(max_size) + "\n")

        fobj.close()
        for chunk_size in chunk_sizes:
            run_scenario(exp_result_path, filename, chunk_size)

##
## main
##

parser = argparse.ArgumentParser(description="ICN Video chunking Testsuite.")
parser.add_argument("-v", "--verbose", help="Verbose mode ON",
                    action="store_true")
parser.add_argument("-f", "--flush", help="Flush all old results",
                    action="store_true")
parser.add_argument("-g", "--graph", help="Draw graphs only (Don't run tests)",
                    action="store_true")
args = parser.parse_args()

ndn_path = "/home/harshad/projects/ndn" 			## SET_THIS
repo_path = "/home/harshad/projects/icn-video-chunking/" 	## SET_THIS

ndn_app_path = repo_path + "apps/"
results_path = repo_path + "/results/"
ns3_dir = ndn_path + "/ns-3"
verbose = 0

if args.verbose:
    verbose = 1

if args.graph:
    draw_graphs(results_path)
    exit(0)

if args.flush:
    os.system("rm -rf " + repo_path + "/results/*")
    print("Flushed all old results.")

range_alpha = [2.0, 2.0, 0.1]
range_num = [100, 101, 1]
range_size = [30, 31, 1]

scenarios_path = repo_path + "/scenarios"

os.chdir(ns3_dir)

alpha = range_alpha[0]
experiment_num = 1
while alpha <= range_alpha[1]:
    ## For all alpha values
    for num_videos in range(range_num[0], range_num[1], range_num[2]):
        for max_size in range(range_size[0], range_size[1], range_size[2]):
            run_experiment(experiment_num, alpha, num_videos, max_size)
            experiment_num = experiment_num + 1

    alpha = alpha + range_alpha[2]
log("Drawing graphs...")
draw_graphs(results_path)



