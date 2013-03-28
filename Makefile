GXX = g++-mp-4.8 -std=c++11

CFLAGS = $(shell $(HOME)/local/cppunit/bin/cppunit-config --cflags) -g
LIBS = $(shell $(HOME)/local/cppunit/bin/cppunit-config --libs)

CCFILES = $(wildcard *.cc)

.PHONY: test

test: test-bi
	./test-bi

test-bi: $(CCFILES)
	$(GXX) -o $@ $(CFLAGS) $(CCFILES) $(LIBS)