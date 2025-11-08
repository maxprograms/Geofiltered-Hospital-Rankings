#include "hospital.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cctype>

// Function to normalize the english language ratings and turn them into a double 1-5
double convertRatingToDouble(const std::string& rating) {
    std::string lower_rating = rating;
    std::transform(lower_rating.begin(), lower_rating.end(), lower_rating.begin(), ::tolower);

    if (lower_rating == "above" || lower_rating == "higher") {
        return 5.0;
    } else if (lower_rating == "average" || lower_rating == "same") {
        return 2.5;
    } else if (lower_rating == "below" || lower_rating == "worse") {
        return 1.0;
    }
    return 0.0;
}

// Parses the hospitals from the CSV into a vector
std::vector<Hospital> parseHospitalCSV(const std::string& filename) {
    std::vector<Hospital> hospitals;

    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << ". Please check the file path." << std::endl;
        return hospitals;
    }

    std::string line;
    bool isHeader = true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> fields;

        while (std::getline(ss, field, ',')) {
            if (!field.empty() && field.back() == '\r') {
                field.pop_back();
            }
            if (field.size() > 1 && field.front() == '"' && field.back() == '"') {
                field = field.substr(1, field.size() - 2);
            }
            fields.push_back(field);
        }

        if (isHeader) {
            isHeader = false;
            continue;
        }

        if (fields.size() < 24) {
            std::cerr << "Skipping malformed row with " << fields.size() << " columns.\n";
            continue;
        }

        Hospital h;
        h.name = fields[0];
        h.city = fields[1];
        h.state = fields[2];
        h.type = fields[3];

        try {
             h.overall_rating = std::stod(fields[4].empty() ? "0.0" : fields[4]);
        } catch (...) {
            h.overall_rating = 0.0;
        }

        h.safety = convertRatingToDouble(fields[6]);
        h.readmission = convertRatingToDouble(fields[7]);
        h.experience = convertRatingToDouble(fields[8]);
        h.effectiveness = convertRatingToDouble(fields[9]);
        h.timeliness = convertRatingToDouble(fields[10]);


        hospitals.push_back(h);
    }

    std::cout << "Successfully parsed " << hospitals.size() << " data rows.\n";
    return hospitals;
}