#!/usr/bin/python3
import os, sys
import numpy
import csv
import argparse
import random

class video:
    def __init__(self, index, size, popularity):
        self.index = index
        self.size = size
        self.popularity = popularity

    def dump(self):
        print("popularity = " + str(self.popularity) + " access = " +
              str(self.access_pattern) + " size = " + str(self.size))

    def write_to_file(self, fd):
        fd.write(str(self.index) + "," + str(self.size) + "\n")

def generate_videos(filename, video_max_size, num_videos):
    pop_array = numpy.random.zipf(alpha, num_videos)
    pop_array_sorted = sorted(pop_array, key = int, reverse = True)
    video_list = []
    total_accesses = 0
    total_video_size = 0

    for count in range(0, num_videos):
        # my_video = video(count, \
        #                  int(numpy.random.uniform(0.0, video_max_size)), \
        #                  pop_array[count])
        video_size = int(numpy.random.uniform(0.0, video_max_size) * 1024 * 1024)
        my_video = (count, video_size, pop_array_sorted[count])
        video_list.append(my_video)
        total_accesses = total_accesses + pop_array_sorted[count]
        total_video_size = total_video_size + video_size

    print("total video size = " + str(total_video_size / (1024 * 1024)) + "MB")
    fobj = open(filename, "w")
    wr = csv.writer(fobj)
    wr.writerows(video_list)
    fobj.close()

    return (total_accesses, video_list)


def pick(total_accesses, video_list):
    choice = random.uniform(0, total_accesses)
    passed = 0
    for video in video_list:
        passed = passed + video[2]
        if(choice < passed):
            return video_list.index(video)

def generate_accesses(total_accesses, video_list):
    accesses = []
    for i in range(0, 500):
        accesses.append([pick(total_accesses, video_list), random.uniform(0, 1)])
    return accesses

def generate_accesses_clients(total_accesses, video_list, num_clients):
    for client in range(0, num_clients):
        accesses = generate_accesses(total_accesses, video_list)
        with open("client-" + str(client) + ".trace", "w") as fobj:
            wr = csv.writer(fobj)
            wr.writerows(accesses)
            # for access in accesses:
            #     fobj.write(str(access) + "\n")

##
## main ##
##

parser = argparse.ArgumentParser(description="ICN Video chunking: Config file generator.")
parser.add_argument("-a", "--alpha", help="alpha",
                    default = "1.5")
parser.add_argument("-s", "--max-video-size", help="Max video size",
                    default = "30")
parser.add_argument("-n", "--num-videos", help="Total number of videos",
                    default = "100")
parser.add_argument("-c", "--num-clients", help="Total number of clients",
                    default = "5")
args = parser.parse_args()

filename = "/home/harshad/projects/icn-video-chunking/apps/videos.conf" ## SET_THIS

alpha = args.alpha
video_max_size = int(args.max_video_size)
num_videos = int(args.num_videos)
num_clients = int(args.num_clients)
(total_accesses, video_list) = generate_videos(filename, video_max_size, num_videos)
generate_accesses_clients(total_accesses, video_list, num_clients)
