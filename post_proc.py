"""
This is the post processing file
for the püarallalized game of life
software
"""
import numpy as np
from os import listdir
from os.path import isfile, join
import matplotlib.pyplot as plt

# get a list of all files avaiable for post processing
file_list = [f for f in listdir("data/") if isfile(join("data/", f))]

# list which length is the amount of cols of processors in the global domain
# and a each number tells the amount of processors in each col
arrangement = np.genfromtxt('data/arrangement.dat',\
                                dtype=int, delimiter=' ', skip_header=2)
# get the dimensions of the entire domain
dims = np.genfromtxt('data/arrangement.dat',\
                                dtype=int, delimiter=' ', skip_footer=2)

# amount of timesteps
if arrangement.shape == ():
    steps = int((len(file_list) - 2) / arrangement)
else:
    steps = int((len(file_list) - 2) / sum(arrangement))

# initiate array to store all values of one timestep
global_values = np.zeros((dims[0], dims[1]))

# passing values in case only one column of processors
if arrangement.shape == ():
    # lopping overall timesteps
    for i in range(steps):
        # set initial indices for slicing
        beg_0 = 0
        end_0 = 0
        beg_1 = 0
        #loop over all processors
        for j in range(arrangement):
            # get data from file
            file_name = "data/proc_" + str(j) + "_" + str(i) + ".dat"
            file = np.genfromtxt(file_name, dtype=int, delimiter=" ")
            # update slicing indices
            end_1 = np.array(file).shape[1]
            end_0 += np.array(file).shape[0]
            # copy file values into global values array
            global_values[beg_0 : end_0, beg_1 : end_1] = file
            # update top left index after slicing for next iteration
            beg_0 += np.array(file).shape[0]
        # put global values array to figure and save it to file
        plt.figure(1)
        plt.imshow(global_values)
        fig_name = "figures/img" + str(i) + ".jpg"
        plt.savefig(fig_name)

        # write image to file
        file_name = "figures/out" + str(i) + ".dat"
        file_out = open(file_name, "w")

        for m in range(global_values.shape[0]):
            for n in range(global_values.shape[1]):
                file_out.write(str(int(global_values[m, n])))
                file_out.write(" ")
            file_out.write("\n")
            
#passing values in case of multiple processor columns
else:
    # here, too, looping over all timesteps
    for i in range(steps):
        # set initial indices for slicing
        beg_0 = 0
        end_0 = 0
        beg_1 = 0
        end_1 = 0
        # loop over all columns of processors
        for j in range(len(arrangement)):
            # loop over all processors in a column
            for k in range(arrangement[j]):
                # get data from file
                file_name = "data/proc_" + str(int(k * len(arrangement) + j)) \
                            + "_" + str(i) + ".dat"
                file = np.genfromtxt(file_name, dtype=int, delimiter=" ")
                # reset indices everytime iteration moves on to next column of processors
                if k == 0:
                    beg_0 = 0
                    end_0 = 0
                    end_1 += np.array(file).shape[1]
                # updat index at every iteration
                end_0 += np.array(file).shape[0]
                # copy file values into global values array
                global_values[beg_0 : end_0, beg_1 : end_1] = file
                # update this index after copying
                beg_0 += np.array(file).shape[0]
            # update this index only when moving on to next column of processors
            beg_1 += np.array(file).shape[1]
        # put global values array to figure and save it to file
        plt.figure(1)
        plt.imshow(global_values)
        fig_name = "figures/img" + str(i) + ".jpg"
        plt.savefig(fig_name)

        # write image to file
        file_name = "figures/out" + str(i) + ".dat"
        file_out = open(file_name, "w")

        for m in range(global_values.shape[0]):
            for n in range(global_values.shape[1]):
                file_out.write(str(int(global_values[m, n])))
                file_out.write(" ")
            file_out.write("\n")
