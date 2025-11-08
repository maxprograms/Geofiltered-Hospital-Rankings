#pragma once

#include "../include/hospital.h"
#include <vector>
#include <string>
#include <cmath>


namespace GeoFilter {

    double haversine(double lat1, double lon1, double lat2, double lon2);

    std::vector<Hospital> filterByDistance(
    const std::vector<Hospital>& hospitals,
    double userLat,
    double userLon,
    double radiusKm
    );

    std::string encodeGeohash(double latitude, double longitude, int precision);

}