#include "../include/find_closest.h"
#include "../include/geohash_geofilter.h"
#include "../include/quadtree_geofilter.h"
#include "../include/scoring.h"
#include "../include/user_prefs.h"
#include "../include/globals.h"
#include "chrono"
using namespace std;

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

vector<HospitalResult> findClosestHospitals(
    const vector<Hospital>& allHospitals,
    double uLat,
    double uLon,
    double maxDistanceMiles,
    const vector<int>& weights,
    int DSindicator
) {
    auto startTime = std::chrono::high_resolution_clock::now();
    vector<HospitalResult> results;
    UserPreferences prefs = makeUserPreferences(maxDistanceMiles, weights);
    double radiusKm = maxDistanceMiles * 1.60934;
    vector<Hospital> filteredHospitals;

    if (DSindicator == 1) {
        filteredHospitals = GeoFilter::filterByDistance(allHospitals, uLat, uLon, radiusKm);
    } else if (DSindicator == 2) {
        QBox usaBounds{-90, 90, -180, 180};
        QuadTree qtree(usaBounds);
        for (const auto& h : allHospitals) qtree.insert(h);
        filteredHospitals = QuadFilter::filterByDistance(qtree, uLat, uLon, radiusKm);
    } else {
        filteredHospitals = allHospitals;
    }

    auto scored = compute(const_cast<vector<Hospital>&>(filteredHospitals), uLat, uLon, prefs);

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
