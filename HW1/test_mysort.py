#!/usr/bin/env python3
from glob import glob
import argparse as ap
from os import system
import os

input_prefix = "hw1_input"
output_file = "hw1_output.txt"
expected_file = "hw1_expected.txt"
diff_file = "hw1_diff.txt"
time_cmd = 'time -f "%e"'
timing_file = "timing.txt"


def make_input(num_files, num_data):
    for i_file in range(num_files):
        command = "./makeinput {} > {}{}.txt".format(num_data, input_prefix,
                                                     i_file)
        print(command)
        system(command)


def get_sort_cmd(files, num_processes, use_threads):
    if use_threads:
        sort_cmd = "./mysort -n {} -t".format(num_processes)
    else:
        sort_cmd = "./mysort -n {}".format(num_processes)

    command = "{} {}".format(sort_cmd, files)
    return command


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

num_files = int(args.num_files)
num_data = int(args.num_data)

# run make first so that we have the binary makeinput
system('make clean && make')
make_input(num_files, num_data)
files = glob("{}*.txt".format(input_prefix))
files = " ".join(files)

print("+++ Number of files:", num_files)
print("+++ Number of data:", num_data)
print("+++ Use threads:", args.use_threads)
print("+++ Files:", files)

if args.evaluate:
    print("*** Evaluate mysort with various options")

    for u_t in range(2):
        for n_p in range(1, 21):
            desc = "num_data: {}, num_processes: {}, use_threads: {}".format(num_data, n_p, u_t)
            print(desc)
            system('echo "{}" >> {}'.format(desc, timing_file))

            sort_cmd = get_sort_cmd(files, n_p, u_t)
            command = '{} {} > /dev/null'.format(time_cmd, sort_cmd)
            command = '{{ {} ; }} 2>> {}'.format(command, timing_file)
            print(command)
            for i in range(10):
                system(command)
    exit()


if args.run:
    print("*** Run mysort on input files and check result")

    sort_cmd = get_sort_cmd(files, args.num_processes, args.use_threads)

    # Redirect mysort result to output file
    command = "{} > {}".format(sort_cmd, output_file)
    print(command)
    system(command)

    # Use system sort on input files
    command = "cat {} | sort -n > {}".format(files, expected_file)
    print(command)
    system(command)

    # Redirect diff of output and system sort to diff file
    command = "diff {} {} > {}".format(output_file, expected_file, diff_file)
    print(command)
    system(command)

    # Check correctness
    if (os.path.getsize(diff_file)):
        print("*** Diff is not empty, somethings wrong")
    else:
        print("*** Diff is empty, everything's good")
    exit()

