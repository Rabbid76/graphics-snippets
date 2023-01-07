#ifndef __MESH_INTERSECTION_TEST_HELPER__H__
#define __MESH_INTERSECTION_TEST_HELPER__H__

#include "mesh_utility/meshMath.h"

namespace mesh {
    inline bool operator ==(const Coordinates3 &a, const Coordinates3 &b) {
        return
                std::fabs(a[0] - b[0]) < 0.001f &&
                std::fabs(a[1] - b[1]) < 0.001f &&
                std::fabs(a[2] - b[2]) < 0.001f;
    }
}

#endif