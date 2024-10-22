#define _USE_MATH_DEFINES
#include <cmath>

#include "geo.h"

namespace geo {

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    const double earth_radius = 6371000;
    if (from == to) {
        return 0;
    }
    static const double dr = 3.1415926535 / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
        + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
        * earth_radius;
}

}  // namespace geo