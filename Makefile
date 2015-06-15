all: osm2bin

%.o: %.cpp *.hpp
	g++ -Wall -std=c++11 -O3 -g -fPIC -c $<

osm2bin: XercesUtils.o osm2bin.o OSMElementHandler.o OSMTagHandler.o SAX2AttributeHandler.o SAX2ElementHandler.o
	g++ -o $@ $^ -L/usr/lib/x86_64-linux-gnu -lxerces-c -lbz2 -lboost_iostreams -L/usr/local/lib/boost

clean:
	rm -f *.o osm2bin log.txt