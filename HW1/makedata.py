#!/usr/bin/env python3
import argparse as ap
from os import system

parser = ap.ArgumentParser()
parser.add_argument('num_files', nargs='?', default='4',
                    help='number of files')
parser.add_argument('--num_integers', nargs='?', default='1000',
                    help='number of integers')
args = parser.parse_args()

num_files = int(args.num_files)
num_integers = int(args.num_integers)

print("number of files: ", num_files)
print("number of integers: ", num_integers)

# run make first so that we have the binary makeinput
system('make')

for i_file in range(num_files):
    command = "./makeinput {} > hw1_input{}.txt".format(num_integers, i_file)
    print(command)
    system(command)
