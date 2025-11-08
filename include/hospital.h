#pragma once
#include <string>

struct Hospital {
    std::string name;
    std::string city;
    std::string state;
    std::string type;

    double overall_rating = 0.0;
    double timeliness;
    double safety;
    double experience;
    double effectiveness;
    double readmission;

    double latitude = 0.0;
    double longitude = 0.0;
};