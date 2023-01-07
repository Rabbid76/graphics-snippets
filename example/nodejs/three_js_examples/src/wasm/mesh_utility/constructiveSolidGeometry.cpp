#include "mesh_utility/constructiveSolidGeometry.h"

using namespace csg;

float Plane::epsilon = PLANE_EPSILON;

Epsilon::Epsilon(float scale) {
    Plane::epsilon = PLANE_EPSILON * scale;
}

Epsilon::~Epsilon() {
    Plane::epsilon = PLANE_EPSILON;
}