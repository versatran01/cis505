TARGETS = makeinput mysort
TESTS_DIR = tests
TESTS = test_main test_divide test_fork test_merge
OBJS = mysort.o common.o

CC = g++
CFLAGS = -pthread -std=c++14 -I. -Wall

all: $(TARGETS)

test: $(TESTS)

makeinput: makeinput.cc
	$(CC) $< -o $@

mysort: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.cc
	$(CC) $(CFLAGS) $< -c -o $@


test_% : tests/test_%.cc
	$(CC) $(CFLAGS) $< -o $@

pack:
	rm -f submit-hw1.zip
	zip -r submit-hw1.zip *.cc README Makefile graph*.pdf description*.txt graph*.jpg *.py *.h *.hpp

clean::
	rm -fv $(TARGETS) $(TESTS) $(OBJS)
	rm -fv hw1_*.txt

#mysort: mysort.cc
	#$(CC) $(CFLAGS) $< -o $@
