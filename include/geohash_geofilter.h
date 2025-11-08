#pragma once

#include "../include/hospital.h"
#include <vector>
#include <string>
#include <unordered_set>


namespace GeoFilter {
    double haversine(double lat1, double lon1, double lat2, double lon2);


    std::string encodeGeohash(double latitude, double longitude, int precision);

    std::string getGeohashNeighbor(const std::string& hash, const std::string& direction);

    std::unordered_set<std::string> getGeohashSearchArea(
        double latitude,
        double longitude,
        int precision
    );
}