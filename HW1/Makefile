TARGETS = makeinput mysort

all: $(TARGETS)

makeinput: makeinput.cc
	g++ $< -o $@

mysort: mysort.cc
	g++ -pthread $< -o $@ -DDEBUG

pack:
	rm -f submit-hw1.zip
	zip -r submit-hw1.zip *.cc README Makefile graph*.pdf description*.txt

clean::
	rm -fv $(TARGETS)
	rm -fv hw1_input*.txt
