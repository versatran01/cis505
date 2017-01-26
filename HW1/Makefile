TARGETS = makeinput mysort test_sort test_divide test_fork

all: $(TARGETS)

makeinput: makeinput.cc
	g++ $< -o $@

mysort: mysort.cc
	g++ -pthread $< -o $@ -DDEBUG -std=c++14

test_sort: test_sort.cc
	g++ -pthread $< -o $@

test_fork: test_fork.cc
	g++ -pthread $< -o $@

pack:
	rm -f submit-hw1.zip
	zip -r submit-hw1.zip *.cc README Makefile graph*.pdf description*.txt

clean::
	rm -fv $(TARGETS)
	rm -fv hw1_*.txt

