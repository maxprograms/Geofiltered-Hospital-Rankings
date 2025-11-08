#include "hospital.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <cctype>

double convertRatingToDouble(const std::string& rating) {
    std::string lower_rating = rating;
    // Convert to lowercase for case-insensitive comparison
    std::transform(lower_rating.begin(), lower_rating.end(), lower_rating.begin(), ::tolower);

    if (lower_rating == "above" || lower_rating == "higher") {
        return 5.0; // Above Average
    } else if (lower_rating == "average" || lower_rating == "same") {
        return 2.5; // Average
    } else if (lower_rating == "below" || lower_rating == "worse") {
        return 1.0; // Below Average
    }
    return 0.0; // Default for unrated or unknown
}

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

        // Split line by tab
        while (std::getline(ss, field, ',')) {
            // Remove potential trailing carriage return (\r)
            if (!field.empty() && field.back() == '\r') {
                field.pop_back();
            }
            // Simple removal of surrounding quotes if present
            if (field.size() > 1 && field.front() == '"' && field.back() == '"') {
                field = field.substr(1, field.size() - 2);
            }
            fields.push_back(field);
        }

        // Skip header row
        if (isHeader) {
            isHeader = false;
            continue;
        }

        // Must still check for 24 columns to ensure the row is complete,
        // even though we only store the first 12 fields.
        if (fields.size() < 24) {
            std::cerr << "Skipping malformed row with " << fields.size() << " columns.\n";
            continue;
        }

        Hospital h;
        h.name = fields[0];
        h.city = fields[1];
        h.state = fields[2];
        h.type = fields[3];

        // Convert overall rating safely
        try {
             // Handle empty string and convert to double
             h.overall_rating = std::stod(fields[4].empty() ? "0.0" : fields[4]);
        } catch (...) {
            h.overall_rating = 0.0;
        }

        // Convert string ratings to double scores (1.0 to 5.0)

        h.safety = convertRatingToDouble(fields[6]);
        h.readmission = convertRatingToDouble(fields[7]);
        h.experience = convertRatingToDouble(fields[8]);
        h.effectiveness = convertRatingToDouble(fields[9]);
        h.timeliness = convertRatingToDouble(fields[10]);


        // The remaining 12 fields (fields[12] through fields[23]) are ignored.

        hospitals.push_back(h);
    }

    std::cout << "Successfully parsed " << hospitals.size() << " data rows.\n";
    return hospitals;
}