// coordinate_set.h
#ifndef COORDINATE_SET_H
#define COORDINATE_SET_H

#include "coordinate.h"
#include "uthash.h"

typedef struct {
    Coordinate coord;  
    UT_hash_handle hh; 
} CoordinateSetEntry;

#endif