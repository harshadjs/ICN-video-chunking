#!/usr/bin/python3
import sys
import os

class video:
    def __init__(self):
        self.name = ""
        self.popularity = ""

    def __EQ__()


class video_access_entry:
    def __init__(self):
        self.started = 0
        self.stopped = 0
        self.requester = ""
        self.video_name = ""
        self.start_offset = 0
        self.stop_offset = 0
        self.bytes_watched = 0
    def display(self, msg = ""):
        print(msg + ":" + self.requester + " watched " + self.video_name + "."
              + "Started at " + str(self.started) +
              " Stopped at " + str(self.stopped))


class video_access:
    def __init__(self):
        self.entries = []
        self.videos = []


    def search_entry(self, timestamp, requester, video_name):
        for item in self.entries:
            if item.requester != requester or item.video_name != video_name:
                continue
            if item.started > int(timestamp):
                continue
            if item.stopped == 0:
                return item
        return None


    def video_start(self, timestamp, requester, video_name, start_offset):
        ventry = video_access_entry()
        ventry.started = int(timestamp)
        ventry.requester = requester
        ventry.video_name = video_name
        ventry.start_offset = start_offset
#        ventry.display()
        self.entries.append(ventry)
        pass

    def video_stop(self, timestamp, requester, video_name, bytes_watched):
        item = self.search_entry(timestamp, requester, video_name)
        if item != None:
#            item.display("Stopped")
            item.stopped = timestamp
            item.bytes_watched = bytes_watched
        pass

    def analyze(self):
        for item in self.entries:
            if item.video_name in self.videos:
                pass
            else:
                print("++ " + item.video_name)
                self.videos.append(item.video_name)


    def display_all(self):
        for item in self.entries:
            item.display()
## Main ##

if len(sys.argv) < 2:
    print("Usage: " + sys.argv[0] + " <trace>")
    sys.exit(0)

if sys.argv[1] == "cornell-video-trace.txt":
    vd = video_access()
    with open(sys.argv[1]) as f:
        content = f.readlines()
        for line in content:
            words = line.split('\t')
            if words[4] == "GET":
                vd.video_start(words[0], words[3], words[5].split("\n")[0], words[2])
            if words[4] == "STOP":
                vd.video_stop(words[0], words[3], words[5].split("\n")[0], words[2])
    vd.analyze()
    vd.display_all()
