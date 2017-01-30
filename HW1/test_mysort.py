#!/usr/bin/env python3
from glob import glob
import argparse as ap
from os import system
import os

input_prefix = "hw1_input"
output_file = "hw1_output.txt"
expected_file = "hw1_expected.txt"
diff_file = "hw1_diff.txt"


def make_input(num_files, num_data):
    for i_file in range(num_files):
        command = "./makeinput {} > {}{}.txt".format(num_data, input_prefix,
                                                     i_file)
        print(command)
        system(command)

parser = ap.ArgumentParser()
parser.add_argument('--num_files', '-f', nargs='?', default='1',
                    help='number of files')
parser.add_argument('--num_data', '-d', nargs='?', default='1000',
                    help='number of data')
parser.add_argument('--run', '-r', action='store_true', help="run mysort")
parser.add_argument('--num_processes', '-n', nargs='?', default='4',
                    help='number of processes')
parser.add_argument('--use_threads', '-t', action='store_true',
                    help="Use threads instead of processes")
parser.add_argument('--evaluate', '-e', action='store_true', help="evaluate")
parser.add_argument('--baseline', '-b', action='store_true',
                    help="find the baseline size")
args = parser.parse_args()

# run make first so that we have the binary makeinput
system('make clean && make')

num_files = int(args.num_files)
num_data = int(args.num_data)

print("Number of files: ", num_files)
print("Number of data: ", num_data)
print("Use threads: ", args.use_threads)

if args.evaluate:
    print("Evaluation")
    files = glob("hw1_input*.txt")
    files_with_space = " ".join(files)
    system("rm {}".format(output_file))
    time_command = 'time -f "%U user, %e real"'
    command = '{} ./mysort -n 2 {} > /dev/null'.format(time_command, files_with_space)
    command = '{{ {} ; }} 2>> {}'.format(command, output_file)
    system(command)
    exit()

if args.run:
    make_input(num_files, num_data)
    print("*** Run mysort on input files and check result")
    files = glob("{}*.txt".format(input_prefix))
    files_with_space = " ".join(files)
    print("Files: {}".format(files_with_space))

    if args.use_threads:
        sort_cmd = "./mysort -n {} -t".format(args.num_processes)
    else:
        sort_cmd = "./mysort -n {}".format(args.num_processes)

    command = "{} {} > {}".format(sort_cmd, files_with_space, output_file)
    print(command)
    system(command)

    command = "cat {} | sort -n > {}".format(files_with_space, expected_file)
    print(command)
    system(command)

    command = "diff {} {} > {}".format(output_file, expected_file, diff_file)
    print(command)
    system(command)

    if (os.path.getsize(diff_file)):
        print("*** Diff is not empty, somethings wrong")
    else:
        print("*** Diff is empty, everything's good")
    exit()






