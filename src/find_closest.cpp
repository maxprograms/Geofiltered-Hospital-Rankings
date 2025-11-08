#include "../include/find_closest.h"
#include "../include/geohash_geofilter.h"
#include "../include/quadtree_geofilter.h"
#include "../include/scoring.h"
#include "../include/user_prefs.h"
#include "../include/globals.h"
#include "chrono"
#include <unordered_set>
using namespace std;

// Makes the userpreferences object given the weights and max distance.
UserPreferences makeUserPreferences(int maxdist, const vector<int>& weights) {
    UserPreferences prefs;
    prefs.timeliness = weights[0];
    prefs.effectiveness = weights[1];
    prefs.experience = weights[2];
    prefs.distance = weights[3];
    prefs.preventive = weights[4];
    prefs.emergency = weights[5];
    prefs.maxdist = maxdist;
    return prefs;
}

// Finds hospitals according to specified data structure (given by DSIndicator global variable).
vector<HospitalResult> findClosestHospitals(
    const vector<Hospital>& allHospitals,
    double uLat,
    double uLon,
    double maxDistanceMiles,
    const vector<int>& weights,
    int DSindicator
) {
    // Begins runtime timer for data structure comparison.
    auto startTime = std::chrono::high_resolution_clock::now();
    vector<HospitalResult> results;
    UserPreferences prefs = makeUserPreferences(maxDistanceMiles, weights);
    double radiusKm = maxDistanceMiles * 1.60934;
    vector<Hospital> filteredHospitals;

    // Filters hospitals based on data structure
    if (DSindicator == 1) {
        int geohashPrecision = 5;
        unordered_set<string> nearbyHashes = GeoFilter::getGeohashSearchArea(uLat, uLon, geohashPrecision);

        for (const auto& h : allHospitals) {
            if (h.latitude == 0.0 && h.longitude == 0.0) continue;

            string hGeohash = GeoFilter::encodeGeohash(h.latitude, h.longitude, geohashPrecision);
            if (nearbyHashes.find(hGeohash) != nearbyHashes.end()) {
                double dist = GeoFilter::haversine(uLat, uLon, h.latitude, h.longitude);
                if (dist <= radiusKm) {
                    filteredHospitals.push_back(h);
                }
            }
        }

    } else if (DSindicator == 2) {
        QBox usaBounds{-90, 90, -180, 180};
        QuadTree qtree(usaBounds);
        for (const auto& h : allHospitals) qtree.insert(h);
        filteredHospitals = QuadFilter::filterByDistance(qtree, uLat, uLon, radiusKm);
    } else {
        // No filtering (redundant)
        filteredHospitals = allHospitals;
    }
    //Computes the scores of the filtered hospitals
    auto scored = compute(const_cast<vector<Hospital>&>(filteredHospitals), uLat, uLon, prefs);

    // Places the top five hospitals into results
    for (auto &entry : scored) {
        HospitalResult hr;
        hr.hospital = *(entry.second);
        hr.distanceKm = GeoFilter::haversine(uLat, uLon, hr.hospital.latitude, hr.hospital.longitude) * 1.60934;
        hr.score = entry.first;
        results.push_back(hr);
        if (results.size() >= 5) break;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    runtime = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    return results;
}
