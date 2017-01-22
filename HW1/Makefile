TARGETS = makeinput mysort test_sort

all: $(TARGETS)

makeinput: makeinput.cc
	g++ $< -o $@

mysort: mysort.cc
	g++ -pthread $< -o $@ -DDEBUG -std=c++14

pack:
	rm -f submit-hw1.zip
	zip -r submit-hw1.zip *.cc README Makefile graph*.pdf description*.txt

clean::
	rm -fv $(TARGETS)
	rm -fv hw1_*.txt

test_sort: test_sort.cc
	g++ -pthread $< -o $@ -DDEBUG -std=c++14
