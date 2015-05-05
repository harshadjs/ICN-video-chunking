#!/usr/bin/python3

import csv
import os
import matplotlib.pyplot as plt
import numpy as np
from numpy import genfromtxt
from matplotlib.backends.backend_pdf import PdfPages

def get_stats(dest):
    print(dest)

    total_views = 0
    total_start_time = 0
    total_view_time = 0
    total_buffering_time = 0

    for filename in os.listdir(dest):
        if filename.startswith("client") and filename.endswith("logs.txt"):
            abspath = os.path.join(dest, filename)
            data = genfromtxt(abspath, delimiter = ",")
            total_views += data[0]
            total_start_time += data[1]
            total_view_time += data[2]
            total_buffering_time += data[3]

    print("total_views = " + str(total_views))
    print("total_start_time = " + str(total_start_time))
    print("total_view_time = " + str(total_view_time))
    print("total_buffering_time = " + str(total_buffering_time))
    return (total_view_time, total_start_time / (total_views), \
            total_buffering_time / total_view_time)


## Example:
# def plot(total_stats):
#     print(total_stats)
#     N = 5
#     menMeans = (20, 35, 30, 35, 27)
#     menStd =   (2, 3, 4, 1, 2)
#     ind = np.arange(N)  # the x locations for the groups
#     width = 0.35       # the width of the bars
#     fig, ax = plt.subplots()
#     rects1 = ax.bar(ind, menMeans, width, color='r', yerr=menStd)
#     womenMeans = (25, 32, 34, 20, 25)
#     womenStd =   (3, 5, 2, 3, 3)
#     rects2 = ax.bar(ind+width, womenMeans, width, color='y', yerr=womenStd)
#     # add some text for labels, title and axes ticks
#     ax.set_ylabel('Scores')
#     ax.set_title('Scores by group and gender')
#     ax.set_xticks(ind+width)
#     ax.set_xticklabels( ('G1', 'G2', 'G3', 'G4', 'G5') )
#     ax.legend((rects1[0], rects2[0]), ('Men', 'Women'))
#     return fig

def plot(total_stats, index, chunks, names):
    print(total_stats)
    N = 1

    ind = np.arange(N)  # the x locations for the groups
    width = 0.35       # the width of the bars

    rects = []
    fig, ax = plt.subplots()
    count = 0
    with open(names["title"], "w") as fobj:
        for stat in total_stats:
            chunk = chunks[count]
            rects.append(ax.bar(ind + width*count, stat[index], width, color = np.random.rand(1,3)))
            count = count + 1
            fobj.write(str(chunk) + "," +  str(stat[index]) + "\n")

    # add some text for labels, title and axes ticks
    ax.set_ylabel(names["y"])
    ax.set_title(names["title"])
    ax.set_xticks(ind + width)
    ax.set_xticklabels((names["xtick"]))

    ax.legend((rects), (chunks))

    return fig

def calculate_load_reduction(root):
    with open(root + "/server-0-logs.txt", "r") as fobj:
        bytes_served_by_server = int(fobj.read())
    print("bytes_served_by_server = " + str(bytes_served_by_server))
    reader = csv.reader(open(root + "/cs-trace-router.txt"), delimiter="\t")
    cache_misses = 0
    cache_hits = 0
    for row in reader:
        try:
            if row[3].isdigit():
                if row[2] == "CacheMisses":
                    cache_misses += int(row[3])
                else:
                    cache_hits += int(row[3])
        except IndexError as e:
            pass
    bytes_served_by_cache = cache_hits * 1000
    print("bytes_served_by_cache = " + str(bytes_served_by_cache))
    return [bytes_served_by_cache / (bytes_served_by_server + bytes_served_by_cache)]

def calculate_pit_size(root):
    reader = csv.reader(open(root + "/pit-log.txt"), delimiter=",")
    pit_size = 0
    total_time = 0
    for row in reader:
#        print(row)
        pit_size += float(row[1].lstrip().rstrip())
        total_time = float(row[0].lstrip().rstrip())

    return [pit_size/total_time]


def draw(root):
    for experiment in os.listdir(root):
        total_stats = []
        chunks = []
        server_load_reduction = []
        pit_size = []
        dirs = [x for x in os.listdir(os.path.join(root, experiment)) if x.isdigit()]

        for subexpt in sorted(dirs, key=int):
            subexpt_dir = os.path.join(root, experiment, subexpt)
            if os.path.isdir(subexpt_dir):
                total_stats.append(get_stats(subexpt_dir))
                chunks.append(subexpt)
                server_load_reduction.append(calculate_load_reduction(subexpt_dir))
                pit_size.append(calculate_pit_size(subexpt_dir))
        pp = PdfPages(experiment + ".pdf")

        print(total_stats)
        names = {}
        names["y"] = "us"
        names["title"] = "View Time"
        names["xtick"] = (chunks)
        figure = plot(total_stats, 0, chunks, names)
        pp.savefig(figure)

        names = {}
        names["y"] = "us"
        names["title"] = "Start Time"
        names["xtick"] = "Start Time"
        figure = plot(total_stats, 1, chunks, names)
        pp.savefig(figure)

        names["y"] = ""
        names["title"] = "Buffering Ratio"
        names["xtick"] = "Buffering Time / View Time"
        figure = plot(total_stats, 2, chunks, names)
        pp.savefig(figure)

        names["y"] = ""
        names["title"] = "PIT Size"
        names["xtick"] = "Buffering Time / View Time"
        figure = plot(pit_size, 0, chunks, names)
        pp.savefig(figure)

        names["y"] = ""
        names["title"] = "Server Load Reduction"
        names["xtick"] = ""
        figure = plot(server_load_reduction, 0, chunks, names)
        pp.savefig(figure)
        pp.close()

        # plot = draw_graphs_dir(subexpt_dir)
        # pp = PdfPages('foo.pdf')
        # pp.savefig(plot)
        # pp.close()


