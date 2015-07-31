#ifndef _TREESTRUCTURE_H
#define _TREESTRUCTURE_H

/**
 * @file tree_structure.h defining structures for the root tree
 *
 * @author Lutz Foucar
 */

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

/** a vector of bools that show which eventcodes where associated with the event */
typedef std::vector<bool> eventStatus_t;

/** the 0d postprocessor structure a map of values identified by a string */
typedef std::map<std::string, double> ppstructure_t;

#endif
