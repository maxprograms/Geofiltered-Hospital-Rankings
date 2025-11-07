#pragma once
#include <string>
#include <map>
struct Hospital {
    std::string name;
    std::string city;
    std::string state;
    std::string type;
    Hospital(
            const std::string& name,
            const std::string& city,
            const std::string& state,
            const std::string& type,
            const std::string& ratingMortality = "",
            const std::string& ratingSafety = "",
            const std::string& ratingReadmission = "",
            const std::string& ratingExperience = "",
            const std::string& ratingEffectiveness = "",
            const std::string& ratingTimeliness = "",
            const std::string& ratingImaging = "",
            const std::map<std::string, double>& procedureScores = {},
            double latitude = 0.0,
            double longitude = 0.0
        )
            : name(name), city(city), state(state), type(type),
              mortality(ratingMortality),
              safety(ratingSafety),
              readmission(ratingReadmission),
              experience(ratingExperience),
              effectiveness(ratingEffectiveness),
              timeliness(ratingTimeliness),
              imaging(ratingImaging),
              latitude(latitude),
              longitude(longitude)
    {}

    // Default constructor for empty hospitals
    Hospital() :  latitude(0.0), longitude(0.0) {}
    int overall_rating;
    std::string mortality;
    std::string safety;
    std::string readmission;
    std::string experience;
    std::string effectiveness;
    std::string timeliness;
    std::string imaging;

    double heart_attack_cost;
    std::string heart_attack_quality;
    std::string heart_attack_value;

    double heart_failure_cost;
    std::string heart_failure_quality;
    std::string heart_failure_value;

    double pneumonia_cost;
    std::string pneumonia_quality;
    std::string pneumonia_value;

    double hip_knee_cost;
    std::string hip_knee_quality;
    std::string hip_knee_value;

    double latitude = 0.0;
    double longitude = 0.0;
};