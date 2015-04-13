#!/usr/bin/python3
import os, sys
import numpy

class video:
    def __init__(self, video_name, popularity_index, access_pattern, size):
        self.popularity = popularity_index
        self.access_pattern = access_pattern
        self.size = size
        self.video_name = video_name

    def dump(self):
        print("popularity = " + str(self.popularity) + " access = " +
              str(self.access_pattern) + " size = " + str(self.size))

    def write_to_file(self, fd):
        fd.write(str(self.popularity) + "," + str(self.access_pattern)
                 + "," + str(self.size) + "\n")

def generate_videos():
    pop_array = numpy.random.zipf(alpha, num_videos)
    pop_array_sorted = sorted(pop_array, key = int, reverse = True)

    video_list = []
    count = 1
    for pop in pop_array_sorted:
        video_name = ("video_%d" % count)
        my_video = video(video_name, pop, 1, 100)
        video_list.append(my_video)
        count = count + 1

    fd = open(filename, "w")
    fd.truncate()
    for my_video in video_list:
        my_video.write_to_file(fd)
    fd.close()

##
## main ##
##


num_videos = 100
filename = "videos.conf"
alpha = 1.5

generate_videos()
