TARGETS = hello-world

all: $(TARGETS)

hello-world: hello-world.cc
	g++ $< -o $@

pack:
	rm -f submit-hw0.zip
	zip -r submit-hw0.zip hello-world.cc README Makefile

clean::
	rm -fv $(TARGETS) submit-hw0.zip
