# osm2bin
Parses OpenStreetMap .osm XML files, and extracts them to an efficient binary format using string tables. Developed for University of Toronto ECE297.

The file osm2bin parses an XML file, either straight text .osm, or bzip2-compressed .osm.bz2.
For now, it just parses and then shows some summary stats regarding the number of elements, the distinct tag keys found, and the a printout of some randomly-chosen node/way/rels.

The parseOSM routine can be customized

# TODO 

* Emit .bin file using Boost::serialize
* Import old code to create road network


