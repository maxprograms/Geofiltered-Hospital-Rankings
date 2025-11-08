#pragma once
#include <string>
#include <unordered_map>
#include <utility>

bool findCityCoords(
    const std::string& cityName,
    const std::string& stateName,
    const std::unordered_map<std::string, std::pair<double, double>>& coords,
    double& lat,
    double& lon,
    std::string& foundCityState
);