#include "find_city_coords.h"
#include <algorithm>
#include <cctype>

using namespace std;

bool findCityCoords(
    const string& cityName,
    const string& stateName,
    const unordered_map<string, pair<double, double>>& coords,
    double& lat,
    double& lon,
    string& foundCityState
) {
    string standardizedCity = cityName;
    string standardizedState = stateName;
    transform(standardizedCity.begin(), standardizedCity.end(), standardizedCity.begin(), ::tolower);
    transform(standardizedState.begin(), standardizedState.end(), standardizedState.begin(), ::tolower);

    for (const auto& entry : coords) {
        const string& key = entry.first;
        size_t commaPos = key.find(',');
        if (commaPos != string::npos) {
            string keyCity = key.substr(0, commaPos);
            string keyState = key.substr(commaPos + 1);
            keyCity.erase(remove_if(keyCity.begin(), keyCity.end(), ::isspace), keyCity.end());
            keyState.erase(remove_if(keyState.begin(), keyState.end(), ::isspace), keyState.end());
            string lowerKeyCity = keyCity, lowerKeyState = keyState;
            transform(lowerKeyCity.begin(), lowerKeyCity.end(), lowerKeyCity.begin(), ::tolower);
            transform(lowerKeyState.begin(), lowerKeyState.end(), lowerKeyState.begin(), ::tolower);
            if (lowerKeyCity == standardizedCity && lowerKeyState == standardizedState) {
                lat = entry.second.first;
                lon = entry.second.second;
                foundCityState = key;
                return true;
            }
        }
    }
    return false;
}