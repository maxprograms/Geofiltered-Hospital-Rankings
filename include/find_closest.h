#pragma once
#include <vector>
#include "hospital.h"
#include "user_prefs.h"

struct HospitalResult {
    Hospital hospital;
    double distanceKm;
    double score;
};

UserPreferences makeUserPreferences(int maxdist, const std::vector<int>& weights);

std::vector<HospitalResult> findClosestHospitals(
    const std::vector<Hospital>& allHospitals,
    double uLat,
    double uLon,
    double maxDistanceMiles,
    const std::vector<int>& weights,
    int DSindicator
);