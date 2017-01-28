TARGETS = makeinput mysort
TESTS_DIR = tests
TESTS = test_sort test_divide test_fork test_merge
OBJS = mysort.o common.o

CC = g++
CFLAGS = -lpthread -std=c++14 -I. -DDEBUG -Wall

all: $(TARGETS)

test: $(TESTS)

makeinput: makeinput.cc
	$(CC) $< -o $@

mysort: $(OBJS)
	$(CC) $^ -o $@

%.o: %.cc
	$(CC) $(CFLAGS) $< -c -o $@


test_% : tests/test_%.cc
	$(CC) $(CFLAGS) $< -o $@

pack:
	rm -f submit-hw1.zip
	zip -r submit-hw1.zip *.cc README Makefile graph*.pdf description*.txt graph*.png

clean::
	rm -fv $(TARGETS) $(TESTS) $(OBJS)
	rm -fv hw1_*.txt

#mysort: mysort.cc
	#$(CC) $(CFLAGS) $< -o $@
