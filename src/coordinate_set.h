#ifndef COORDINATE_SET_H
#define COORDINATE_SET_H

#include "coordinate.h"
#include "uthash.h"

typedef struct {
    Coordinate coord;  // Key
    UT_hash_handle hh; // Required hash handle
} CoordinateSetEntry;

#endif