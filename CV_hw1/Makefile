FILENAME=$(wildcard *.cpp)
OUTPUTNAME=cvhw1
CFLAGS=-g -O2 -std=c++11
COMP=clang++

main:
	$(COMP) $(CFLAGS) $(FILENAME) -o $(OUTPUTNAME) `pkg-config opencv --cflags --libs`

clean:
	rm -f $(OUTPUTNAME)
