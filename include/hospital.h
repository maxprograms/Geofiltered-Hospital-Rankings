#pragma once
#include <string>

struct Hospital {
    std::string name;
    std::string city;
    std::string state;
    std::string type;

    double overall_rating = 0.0;
    std::string timeliness;
    std::string safety;
    std::string experience;
    std::string effectiveness;
    std::string readmission;


    double latitude = 0.0;
    double longitude = 0.0;
};