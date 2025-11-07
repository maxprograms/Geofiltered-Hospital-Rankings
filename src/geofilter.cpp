#include "../include/geofilter.h"
#include <cmath>
#include <sstream>

// Earth radius in kilometers
constexpr double EARTH_RADIUS_KM = 6371.0;

namespace GeoFilter {

double haversine(double lat1, double lon1, double lat2, double lon2) {
    const double toRad = M_PI / 180.0;
    double dLat = (lat2 - lat1) * toRad;
    double dLon = (lon2 - lon1) * toRad;

    lat1 *= toRad;
    lat2 *= toRad;

    double a = std::sin(dLat/2) * std::sin(dLat/2) +
               std::cos(lat1) * std::cos(lat2) *
               std::sin(dLon/2) * std::sin(dLon/2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return EARTH_RADIUS_KM * c;
}

std::vector<Hospital> filterByDistance(
    const std::vector<Hospital>& hospitals,
    double userLat,
    double userLon,
    double radiusKm
) {
    std::vector<Hospital> result;
    for (const auto& h : hospitals) {
        if (h.latitude == 0.0 && h.longitude == 0.0) continue; // skip if coords missing
        double distance = haversine(userLat, userLon, h.latitude, h.longitude);
        if (distance <= radiusKm)
            result.push_back(h);
    }
    return result;
}

// Simple base32 alphabet for geohash encoding
static const char* GEOHASH_BASE32 = "0123456789bcdefghjkmnpqrstuvwxyz";

std::string encodeGeohash(double latitude, double longitude, int precision) {
    double latInterval[2] = {-90.0, 90.0};
    double lonInterval[2] = {-180.0, 180.0};

    bool isEven = true;
    int bit = 0, ch = 0;
    std::string geohash;

    while ((int)geohash.size() < precision) {
        double mid;
        if (isEven) {
            mid = (lonInterval[0] + lonInterval[1]) / 2;
            if (longitude > mid) {
                ch |= 1 << (4 - bit);
                lonInterval[0] = mid;
            } else {
                lonInterval[1] = mid;
            }
        } else {
            mid = (latInterval[0] + latInterval[1]) / 2;
            if (latitude > mid) {
                ch |= 1 << (4 - bit);
                latInterval[0] = mid;
            } else {
                latInterval[1] = mid;
            }
        }

        isEven = !isEven;
        if (bit < 4)
            bit++;
        else {
            geohash += GEOHASH_BASE32[ch];
            bit = 0;
            ch = 0;
        }
    }

    return geohash;
}

}
