#!/usr/bin/env python3
from glob import glob
import argparse as ap
from os import system


parser = ap.ArgumentParser()
parser.add_argument('--num_files', '-nf', nargs='?', default='4',
                    help='number of files')
parser.add_argument('--num_integers', '-n1', nargs='?', default='1000',
                    help='number of integers')
parser.add_argument('--run', '-r', action='store_true', help="run mysort")
parser.add_argument('--num_processes', '-np', nargs='?', default='4',
                    help='number of processes')
args = parser.parse_args()

if args.run:
    print("Run mysort on input files")
    files = glob("hw1_input*.txt")
    files_with_space = " ".join(files)
    command = "./mysort -n {} {}".format(args.num_processes, files_with_space)
    print(command)
    system(command)
    exit()

num_files = int(args.num_files)
num_integers = int(args.num_integers)

print("number of files: ", num_files)
print("number of integers: ", num_integers)

# run make first so that we have the binary makeinput
system('make clean && make')

# Generate files
for i_file in range(num_files):
    command = "./makeinput {} > hw1_input{}.txt".format(num_integers, i_file)
    print(command)
    system(command)


