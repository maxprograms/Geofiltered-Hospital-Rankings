#include "hospital.h"
#include "user_prefs.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
using namespace std;

constexpr double EARTH_RADIUS_MI = 3958.8;

double degrees_to_radians(double degrees) {
  return degrees * M_PI / 180.0;
}
double haversine_distance(double lat1, double lon1, double lat2, double lon2) {
  double lat = degrees_to_radians(lat2 - lat1);
  double lon = degrees_to_radians(lon2 - lon1);
  double a = sin(lat/2) * sin(lat/2) + cos(degrees_to_radians(lat1)) * cos(degrees_to_radians(lat2)) * sin(lon/2) * sin(lon/2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return c * EARTH_RADIUS_MI;
}


// double normalize(const string &rating) {
//   if (rating == "Above") {
//     return 1.0;
//  }
//   if (rating == "Average") {
//     return 0.6;
//  }
//   if (rating == "Below") {
//     return 0.3;
//  }
//   return 0.0;
// }

vector<pair<double, const Hospital*>> compute(vector<Hospital> &hospitals, double userlat, double userlon, const UserPreferences &prefs) {
  vector<pair<double, const Hospital *>> scored;
  for (auto &h : hospitals) {
    if (h.latitude == 0.0 && h.longitude == 0.0) {
      continue;
    }
    double distance = haversine_distance(userlat, userlon, h.latitude, h.longitude);
    if (distance > prefs.maxdist) {
      continue;
    }
    double t = h.timeliness;
    double e = h.effectiveness;
    double x = h.experience;
    double em = h.safety;
    double p = h.readmission;
    double distscore = 1.0 - min(distance / prefs.maxdist, 1.0);

    double total = prefs.timeliness * t + prefs.effectiveness * e + prefs.experience * x + prefs.emergency * em + prefs.preventive * p + prefs.distance * distscore;
    scored.emplace_back(total, &h);
  }

  sort(scored.begin(), scored.end(), [](auto &a, auto &b) {
    return a.first > b.first;
  });

  return scored;
}
