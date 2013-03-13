CFLAGS = $(shell cppunit-config --cflags)
LIBS = $(shell cppunit-config --libs)

.PHONY: test

test: test-bi
	./test-bi

test-bi: *.cc
	g++ -o $@ $(CFLAGS) *.cc $(LIBS)
