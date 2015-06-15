all: osm2bin

%.o: %.cpp *.hpp
	g++ -Wall -std=c++11 -O3 -g -fPIC -c $<

osm2bin: XercesUtils.o osm2bin.o OSMElementHandler.o OSMTagHandler.o SAX2AttributeHandler.o SAX2ElementHandler.o OSMNode.o
	g++ -o $@ $^ -L/usr/lib/x86_64-linux-gnu -lxerces-c -lbz2 -lboost_iostreams -L/usr/local/lib/boost -lboost_serialization -lboost_timer -lboost_system

clean:
	rm -f *.o osm2bin log.txt
	
test_binvsosm: osm2bin
	time ./osm2bin hamilton_canada.osm.bz2 			> hamilton.bz2.out
	time ./osm2bin hamilton_canada.osm     		> hamilton.osm.out
	time ./osm2bin hamilton_canada.osm.bz2.out.bin > hamilton.bin.out
	
	diff hamilton.bz2.out hamilton.osm.out
	diff hamilton.osm.out hamilton.bin.out