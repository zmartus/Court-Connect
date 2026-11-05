// OWNER: Zachery Lowry
#include "court_connect.h"
#include <algorithm>
#include <cmath>

// Straight-line (great-circle) distance in miles, not driving distance.
double haversine_miles(double from_latitude, double from_longitude,
                       double to_latitude, double to_longitude) {
    constexpr double EARTH_RADIUS_MILES = 3958.7613;
    constexpr double PI = 3.14159265358979323846;
    const double lat1 = from_latitude * PI / 180.0;
    const double lat2 = to_latitude * PI / 180.0;
    const double delta_lat = (to_latitude - from_latitude) * PI / 180.0;
    const double delta_lon = (to_longitude - from_longitude) * PI / 180.0;

    const double a = std::pow(std::sin(delta_lat / 2.0), 2) +
                     std::cos(lat1) * std::cos(lat2) *
                     std::pow(std::sin(delta_lon / 2.0), 2);
    const double bounded_a = std::clamp(a, 0.0, 1.0);
    const double angle = 2.0 * std::asin(std::sqrt(bounded_a));
    return EARTH_RADIUS_MILES * angle;
}
