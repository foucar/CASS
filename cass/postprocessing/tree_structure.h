#ifndef _TREESTRUCTURE_H
#define _TREESTRUCTURE_H

#include <map>
#include <vector>
#include <string>

/** a hit is just a map of hit values identified by a string */
typedef std::map<std::string, double> treehit_t;

/** lots of hits are a delayline detector */
typedef std::vector<treehit_t> treedetector_t;

/** a map will enable us to identify the right detector in the container */
typedef std::map<std::string, treedetector_t> treestructure_t;

/** the machine structure just a map of values identified by a string */
typedef std::map<std::string, double> machinestructure_t;

#endif
