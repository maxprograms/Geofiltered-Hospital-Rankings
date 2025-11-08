#include "../include/geohash_geofilter.h"
#include "../include/hospital.h"
#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <map>
#include <array>

constexpr double EARTH_RADIUS_KM = 6371.0;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

static const char* GEOHASH_BASE32 = "0123456789bcdefghjkmnpqrstuvwxyz";
static const std::string BASE32_ALPHABET = GEOHASH_BASE32;

std::string encodeGeohash(double latitude, double longitude, int precision) {
    double latInterval[2] = {-90.0, 90.0};
    double lonInterval[2] = {-180.0, 180.0};

    bool isEven = true;
    int bit = 0, ch = 0;
    std::string geohash;
    geohash.reserve(precision);

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

        if (bit < 4) {
            bit++;
        } else {
            geohash += GEOHASH_BASE32[ch];
            bit = 0;
            ch = 0;
        }
    }

    return geohash;
}

static const std::map<std::string, std::array<std::string, 2>> BORDERS = {
    {"N", {"prxz", "bcfguvyz"}},
    {"S", {"028b", "0145hjnp"}},
    {"E", {"bcfguvyz", "prxz"}},
    {"W", {"0145hjnp", "028b"}}
};

static const std::map<std::string, std::array<std::string, 2>> NEIGHBORS = {
    {"N", {"bcdefghjklmnpqrstuvwxyz0123456789", "fg45238967debc01yzhrstjnpqwvxy"}},
    {"S", {"0145hjnpqrstuvwxyz2389czfbpqrhjk", "0145hjnpqrstuvwxyz2389czfbpqrhjk"}},
    {"E", {"bcdefghjklmnpqrstuvwxyz0123456789", "fg45238967debc01yzhrstjnpqwvxy"}},
    {"W", {"0145hjnpqrstuvwxyz2389czfbpqrhjk", "0145hjnpqrstuvwxyz2389czfbpqrhjk"}},
};


std::string getGeohashNeighbor(const std::string& hash, const std::string& direction) {
    if (hash.empty() || (direction.length() != 1)) return "";

    char lastChar = hash.back();
    std::string parentHash = hash.substr(0, hash.length() - 1);

    int parity = (hash.length() % 2 == 0) ? 1 : 0;

    if (BORDERS.at(direction)[parity].find(lastChar) != std::string::npos) {
        parentHash = getGeohashNeighbor(parentHash, direction);
    }

    const std::string& lookupSet = NEIGHBORS.at(direction)[parity];
    size_t newCharIndex = lookupSet.find(lastChar);

    if (newCharIndex == std::string::npos) return "";

    char newChar;
    if (direction == "N" || direction == "E") {
        newChar = lookupSet[(newCharIndex + 1) % lookupSet.length()];
    } else { // S or W
        newChar = lookupSet[(newCharIndex - 1 + lookupSet.length()) % lookupSet.length()];
    }

    return parentHash + newChar;
}

std::unordered_set<std::string> getGeohashSearchArea(double latitude, double longitude, int precision) {
    std::unordered_set<std::string> geohashes;

    std::string centerHash = encodeGeohash(latitude, longitude, precision);
    geohashes.insert(centerHash);

    std::string n = getGeohashNeighbor(centerHash, "N");
    std::string e = getGeohashNeighbor(centerHash, "E");
    std::string s = getGeohashNeighbor(centerHash, "S");
    std::string w = getGeohashNeighbor(centerHash, "W");

    geohashes.insert(n);
    geohashes.insert(e);
    geohashes.insert(s);
    geohashes.insert(w);

    if (!n.empty()) geohashes.insert(getGeohashNeighbor(n, "E"));
    if (!n.empty()) geohashes.insert(getGeohashNeighbor(n, "W"));
    if (!s.empty()) geohashes.insert(getGeohashNeighbor(s, "E"));
    if (!s.empty()) geohashes.insert(getGeohashNeighbor(s, "W"));

    return geohashes;
}

}