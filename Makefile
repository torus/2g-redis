GXX = g++-mp-4.8 -std=c++11

CFLAGS = $(shell $(HOME)/local/cppunit/bin/cppunit-config --cflags) -g
LIBS = $(shell $(HOME)/local/cppunit/bin/cppunit-config --libs)

CCFILES = $(wildcard *.cc)
HHFILES = $(wildcard *.hh)

.PHONY: test

test: test-bi
	./test-bi

test-bi: $(CCFILES) $(HHFILES)
	$(GXX) -o $@ $(CFLAGS) $(CCFILES) $(LIBS)
