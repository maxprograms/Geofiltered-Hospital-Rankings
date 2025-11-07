#pragma once
#include "hospital.h"
#include <vector>
#include <string>
#include <cmath>

namespace GeoFilter {

    // --- Distance calculation (Haversine formula) ---
    double haversine(double lat1, double lon1, double lat2, double lon2);

    // --- Filter hospitals within a given radius ---
    std::vector<Hospital> filterByDistance(
        const std::vector<Hospital>& hospitals,
        double userLat,
        double userLon,
        double radiusKm
    );

    // --- Optional geohash encoding ---
    std::string encodeGeohash(double latitude, double longitude, int precision = 6);

}
