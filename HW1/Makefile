TARGETS = makeinput mysort
TESTS_DIR = tests
TESTS = test_sort test_divide test_fork
CC = g++
CFLAGS = -lpthread -std=c++14 -I. -DDEBUG

all: $(TARGETS) $(TESTS)

makeinput: makeinput.cc
	$(CC) $< -o $@

mysort: mysort.cc
	$(CC) $(CFLAGS) $< -o $@

test_% : tests/test_%.cc
	$(CC) $(CFLAGS) $< -o $@

pack:
	rm -f submit-hw1.zip
	zip -r submit-hw1.zip *.cc README Makefile graph*.pdf description*.txt graph*.png

clean::
	rm -fv $(TARGETS) $(TESTS)
	rm -fv hw1_*.txt

