#ifndef SCORING_H
#define SCORING_H
#include "hospital.h"
#include "user_prefs.h"
#include <vector>
#include <utility>

using namespace std;
vector<pair<double, const Hospital*>> compute(
    vector<Hospital>& hospitals,
    double userlat,
    double userlon,
    const UserPreferences& prefs
);
#endif