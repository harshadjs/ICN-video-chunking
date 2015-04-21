#!/usr/bin/python3
import os
import sys
import errno
import shutil

chunk_sizes = [100, 1000]

def log(string, end = "\n"):
    print(string, end = end)
    sys.stdout.flush()

def run_experiment(num, alpha, num_videos, max_size):
    print("New experiment started with configuration ", end = "")
    print("[alpha = " + str(alpha) + ", num = " + str(num_videos) + \
          ", siz = " + str(max_size) + "]")
    os.system(ndn_app_path + "generate_config_files.py " \
              + str(alpha) + " " + str(num_videos) + " " + str(max_size))

    for filename in os.listdir(scenarios_path):
        exp_result_path = results_path + "/" + str(num)
        try:
            os.mkdir(exp_result_path)
        except OSError as e:
            if e.errno != errno.EEXIST:
                raise e

        shutil.copy(ndn_app_path + "videos.conf", exp_result_path)
        with open(exp_result_path + "/experiment.conf", 'w') as fobj:
            fobj.write("alpha = " + str(alpha) + "\n")
            fobj.write("num_videos = " + str(num_videos) + "\n")
            fobj.write("max_size = " + str(max_size) + "\n")

        fobj.close()

        for chunk_size in chunk_sizes:
            log("Running [%s@%d] ... " % (filename, chunk_size), end = "")

            with open(ndn_app_path + "/chunk_size.conf", "w") as fobj:
                fobj.write("%d" % chunk_size)
            fobj.close()

            cmd = "NS_LOG=icnVideoChunkingClient:icnVideoChunkingServer " + \
                  ndn_path + "/ns-3/waf --run=" + filename
            if verbose == 0:
                cmd = cmd + " > /tmp/waf.log 2>&1"
            else:
                print("")

            os.system(cmd)

            log("\t[DONE]", end = "")

            try:
                shutil.move("/tmp/waf.log", exp_result_path \
                             + "/waf-%s-%s.log" % (filename, chunk_size))
                shutil.move(ns3_dir + "/cs-trace.txt", \
                            exp_result_path + "/cs-trace-%s-%s.txt" \
                            % (filename, chunk_size))
            except IOError as e:
                #if e.errno != errno.ENOENT:
                #    raise e
                #else:
                log("\t(!WARNING! This run did not produce cs-trace.txt)", end = "")
            log("")

##
## main
##


ndn_path = "/home/dayoon/ndnsim" 			## SET_THIS
repo_path = "/home/dayoon/15744/ICN-video-chunking" 	## SET_THIS
ndn_app_path = "/home/dayoon/15744/ICN-video-chunking/" + \
               "icn-video-chunking-ndn-apps/"
results_path = repo_path + "/results/"
ns3_dir = ndn_path + "/ns-3"
verbose = 0

range_alpha = [2.0, 2.1, 0.1]
range_num = [100, 102, 1]
range_size = [30720, 30721, 1]

scenarios_path = repo_path + "/scenarios"

if len(sys.argv) >= 2:
    if sys.argv[1] == "-v":
        verbose = 1

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



