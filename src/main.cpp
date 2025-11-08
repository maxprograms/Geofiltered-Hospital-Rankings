#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <cmath> // For std::stod
#include "geofilter.h"
#include "hospital.h"
#include "scoring.h"
#include "csv_parser.h"
#include "user_prefs.h"
#include "assign_coords.h"
#include "city_coords.h" // Includes loadCityCoords

using namespace std;

// Global variables for user location and filtered results
double userLat = 0.0;
double userLon = 0.0;
string userCityState = ""; // Stores the found key, e.g., "Dallas,TX"

// Structure to hold the result of the scoring process
struct HospitalResult {
    Hospital hospital;
    double distanceKm; // Distance from user in Kilometers
    double score;      // Calculated score based on weights
};

// Global storage for the top 10 recommended hospitals
std::vector<HospitalResult> topHospitals;

// Conversion factor: miles to kilometers
constexpr double MILE_TO_KM = 1.60934;

// Helper function to find coordinates for a city name (simple search)
// NOTE: This assumes the input `cityName` is in Title Case and attempts a partial match.
bool findCityCoords(const std::string& cityName, const std::unordered_map<std::string, std::pair<double, double>>& coords, double& lat, double& lon, std::string& foundCityState) {
    // Standardize input (e.g., "Austin")
    std::string standardizedCity = cityName;

    // Loop through all city coordinates loaded from the CSV
    for (const auto& pair : coords) {
        // Key is in format "City,ST" (e.g., "AUSTIN,TX")
        size_t commaPos = pair.first.find(',');
        if (commaPos != std::string::npos) {
            std::string keyCity = pair.first.substr(0, commaPos);

            // Simple string comparison for a match
            if (keyCity == standardizedCity) {
                lat = pair.second.first;
                lon = pair.second.second;
                foundCityState = pair.first;
                return true; // Found the first matching city
            }
        }
    }
    return false;
}
UserPreferences makeUserPreferences(int maxdist, const vector<int>& weights) {
    UserPreferences prefs;
    prefs.timeliness = weights[0];
    prefs.effectiveness = weights[1];
    prefs.experience = weights[2];
    prefs.distance = weights[3];
    prefs.preventive = weights[4];
    prefs.emergency = weights[5];
    prefs.maxdist = maxdist;
    return prefs;
}

vector<HospitalResult> findClosestHospitals(
    const vector<Hospital>& allHospitals,
    double uLat,
    double uLon,
    double maxDistanceMiles,
    const vector<int>& weights
) {
    vector<HospitalResult> results;

    // Create UserPreferences
    UserPreferences prefs = makeUserPreferences(maxDistanceMiles, weights);

    // Compute scores
    auto scored = compute(const_cast<vector<Hospital>&>(allHospitals), uLat, uLon, prefs);

    for (auto &entry : scored) {
        HospitalResult hr;
        hr.hospital = *(entry.second);
        hr.distanceKm = GeoFilter::haversine(uLat, uLon, hr.hospital.latitude, hr.hospital.longitude) * 1.60934; // km
        hr.score = entry.first;
        results.push_back(hr);

        if (results.size() >= 10) break; // top 10
    }

    return results;
}



int main() {

    std::vector<Hospital> hospitals = parseHospitalCSV("../data/hospitals.csv");
    std::unordered_map<std::string, std::pair<double, double>> coords = loadCityCoords("../data/uscities.csv");
    assignCoordinates(hospitals, coords);

    // Create window
    sf::RenderWindow window(sf::VideoMode(1200, 800), "MedMetrics");
    window.setFramerateLimit(60);

    // Load font
    sf::Font font;
    if (!font.loadFromFile("../include/arial.ttf")) {
        return -1;
    }

    // Title
    sf::Text titleText("Welcome to MedMetrics", font, 40);
    titleText.setFillColor(sf::Color::Blue);
    titleText.setStyle(sf::Text::Bold);
    sf::FloatRect titleBound = titleText.getLocalBounds();
    titleText.setPosition((1200 - titleBound.width) / 2, 80);

    sf::Text subtitleText("Geofiltered Hospital Rankings Program", font, 30);
    subtitleText.setFillColor(sf::Color::Blue);
    sf::FloatRect subtitleBound = subtitleText.getLocalBounds();
    subtitleText.setPosition((1200 - subtitleBound.width) / 2, 130);

    // Underlines separator
    sf::RectangleShape separator1(sf::Vector2f(1100, 2));
    separator1.setFillColor(sf::Color::Black);
    separator1.setPosition(50, 180);

    sf::RectangleShape separator2(sf::Vector2f(1100, 2));
    separator2.setFillColor(sf::Color::Black);
    separator2.setPosition(50, 200);

    // Menu
    sf::Text menuText("1. Set Location\n"
                       "2. Customize Hospital Priorities\n"
                       "3. View Recommended Hospitals\n"
                       "4. Display Hospital Details\n"
                       "5. Export Results\n"
                       "6. Choose Data Structure\n"
                       "7. About\n"
                       "8. Exit", font, 33);
    menuText.setFillColor(sf::Color::Black);
    menuText.setLineSpacing(1.2f);
    sf::FloatRect menuBound = menuText.getLocalBounds();
    menuText.setPosition((1200 - menuBound.width) / 2, 280);

    sf::Text promptText("Enter choice [1-8]", font, 30);
    promptText.setFillColor(sf::Color::Green);
    sf::FloatRect promptBound = promptText.getLocalBounds();
    promptText.setPosition(((1200 - promptBound.width) / 2)-50, 700);

    //variables and things that will be used in the different options
    int choice;
    vector<int> weights = {3, 3, 3, 3, 3, 3};
    int selectedHospitalInd = -1;
    int DSindicator = 1;
    string city = "";
    int userDistanceMiles = 0; // Store distance in miles as input

    bool citySelected = false; // Changed to false for final tests

    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            // User input selection
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) {
                    choice = event.key.code - sf::Keyboard::Num1 + 1;

                    //CHOICE 1 - Set Location - Now correctly finds coords and filters hospitals
                    if (choice == 1) {
                        sf::Event clearEvent;
                        while (window.pollEvent(clearEvent)) {}

                        string inputName = "";
                        string inputDistance = "";
                        string cityNameDisplay = "";
                        bool Option1 = true;
                        bool nameFound = false;
                        bool distanceEnter = false;
                        bool nameEnteredOnce = false;

                        // Event processing for option 1
                        while (Option1 && window.isOpen()) {
                            sf::Event Opt1Event;
                            while (window.pollEvent(Opt1Event)) {
                                if (Opt1Event.type == sf::Event::Closed) {
                                    window.close();
                                }

                                if (Opt1Event.type == sf::Event::KeyPressed && Opt1Event.key.code == sf::Keyboard::Escape) {
                                    Option1 = false;
                                }

                                if (Opt1Event.type == sf::Event::KeyPressed && Opt1Event.key.code == sf::Keyboard::Enter) {
                                    // City search
                                    if (!inputName.empty() && !nameFound) {
                                        nameEnteredOnce = true;
                                        // Attempt to find coordinates for the city
                                        if (findCityCoords(inputName, coords, userLat, userLon, userCityState)) {
                                            nameFound = true;
                                            cityNameDisplay = inputName;
                                            city = inputName;
                                            cityNameDisplay[0] = toupper(cityNameDisplay[0]);
                                        }
                                        else {
                                            inputName = ""; // Clear input for re-entry
                                        }
                                    }

                                    // Distance input
                                    else if (nameFound && !inputDistance.empty() && !distanceEnter) {
                                        try {
                                            int distanceValue = std::stoi(inputDistance);
                                            if (distanceValue > 0 && distanceValue < 1000) { // Max distance check
                                                distanceEnter = true;
                                                userDistanceMiles = distanceValue;
                                            } else {
                                                inputDistance = "";
                                            }
                                        } catch (...) {
                                            inputDistance = "";
                                        }
                                    }
                                }

                                // City name input
                                if (!nameFound && Opt1Event.type == sf::Event::TextEntered) {
                                    if (Opt1Event.text.unicode == '\b' && !inputName.empty()) {
                                        inputName.pop_back();
                                    }
                                    else if (inputName.length() < 25 &&
                                            ((Opt1Event.text.unicode >= 'a' && Opt1Event.text.unicode <= 'z') ||
                                             (Opt1Event.text.unicode >= 'A' && Opt1Event.text.unicode <= 'Z') ||
                                             (Opt1Event.text.unicode == ' '))) {

                                        // Simple title case for display, but store as-is for search
                                        char c = static_cast<char>(Opt1Event.text.unicode);
                                        if (inputName.empty() || inputName.back() == ' ') {
                                            inputName += toupper(c);
                                        }
                                        else {
                                            inputName += tolower(c);
                                        }
                                    }
                                }

                                // Handle distance input - only allow digits 0-9
                                if (nameFound && !distanceEnter && Opt1Event.type == sf::Event::TextEntered) {
                                    if (Opt1Event.text.unicode == '\b' && !inputDistance.empty()) {
                                        inputDistance.pop_back();
                                    }
                                    else if (inputDistance.length() < 3 &&
                                            (Opt1Event.text.unicode >= '0' && Opt1Event.text.unicode <= '9')) {
                                        inputDistance += static_cast<char>(Opt1Event.text.unicode);
                                    }
                                }
                            }

                            // Drawing interface
                            window.clear(sf::Color::White);

                            sf::Text title("Set Location and Search Radius", font, 40);
                            title.setFillColor(sf::Color::Blue);
                            title.setStyle(sf::Text::Bold);
                            sf::FloatRect titleBound = title.getLocalBounds();
                            title.setPosition((1200 - titleBound.width) / 2, 50);
                            window.draw(title);

                            if (!nameFound  && !nameEnteredOnce) {
                                // City input screen
                                sf::Text prompt1("Enter your city name:", font, 30);
                                prompt1.setFillColor(sf::Color::Black);
                                sf::FloatRect prompt1Bounds = prompt1.getLocalBounds();
                                prompt1.setPosition((1200 - prompt1Bounds.width) / 2, 150);
                                window.draw(prompt1);

                                // Input box
                                sf::RectangleShape inputBox(sf::Vector2f(600, 50));
                                inputBox.setFillColor(sf::Color(240, 240, 240));
                                inputBox.setOutlineColor(sf::Color::Blue);
                                inputBox.setOutlineThickness(2);
                                inputBox.setPosition(300, 220);
                                window.draw(inputBox);

                                // Input text
                                sf::Text inputDisplay(inputName.empty() ? "Type Here (e.g., Dallas)" : inputName, font, 28);
                                inputDisplay.setFillColor(inputName.empty() ? sf::Color(150, 150, 150) : sf::Color::Black);
                                inputDisplay.setPosition(310, 225);
                                window.draw(inputDisplay);

                                sf::Text instruction("Press ENTER to search", font, 20);
                                instruction.setFillColor(sf::Color::Green);
                                instruction.setPosition(500, 300);
                                window.draw(instruction);

                            } else if (!nameFound && nameEnteredOnce) {
                                // City was not found
                                sf::Text error("City not found. Try again or check spelling.", font, 30);
                                error.setFillColor(sf::Color::Red);
                                sf::FloatRect errorBounds = error.getLocalBounds();
                                error.setPosition((1200 - errorBounds.width) / 2, 200);
                                window.draw(error);

                                sf::Text errorInstruction("Press ESC to return to main menu", font, 20);
                                errorInstruction.setFillColor(sf::Color::Green);
                                errorInstruction.setPosition(450, 250);
                                window.draw(errorInstruction);
                                citySelected = false;
                            }
                            else if (nameFound && !distanceEnter) {
                                // Distance input screen
                                sf::Text prompt2("Enter maximum distance (miles) from " + cityNameDisplay + ":", font, 30);
                                prompt2.setFillColor(sf::Color::Black);
                                sf::FloatRect prompt2Bounds = prompt2.getLocalBounds();
                                prompt2.setPosition((1200 - prompt2Bounds.width) / 2, 150);
                                window.draw(prompt2);

                                // Distance input box
                                sf::RectangleShape distanceBox(sf::Vector2f(200, 50));
                                distanceBox.setFillColor(sf::Color(240, 240, 240));
                                distanceBox.setOutlineColor(sf::Color::Blue);
                                distanceBox.setOutlineThickness(2);
                                distanceBox.setPosition(500, 220);
                                window.draw(distanceBox);

                                // Distance input text
                                sf::Text distanceDisplay(inputDistance.empty() ? "Type Here" : inputDistance, font, 28);
                                distanceDisplay.setFillColor(inputDistance.empty() ? sf::Color(150, 150, 150) : sf::Color::Black);
                                distanceDisplay.setPosition(510, 225);
                                window.draw(distanceDisplay);

                                sf::Text instruction("Press ENTER to confirm", font, 20);
                                instruction.setFillColor(sf::Color::Green);
                                instruction.setPosition(500, 300);
                                window.draw(instruction);
                            }

                            //Everything was successful
                            else if (distanceEnter) {
                                double radiusKm = userDistanceMiles * MILE_TO_KM;
                                topHospitals = findClosestHospitals(hospitals, userLat, userLon, radiusKm, weights);

                                sf::Text success("Location and radius set successfully!", font, 36);
                                success.setFillColor(sf::Color::Green);
                                success.setStyle(sf::Text::Bold);
                                sf::FloatRect successBounds= success.getLocalBounds();
                                success.setPosition((1200 -successBounds.width) / 2, 200);
                                window.draw(success);

                                selectedHospitalInd= -1;
                                citySelected=true;
                            }

                            sf::Text escText("Press ESC to return to main menu", font, 18);
                            escText.setFillColor(sf::Color::Green);
                            sf::FloatRect escBound=escText.getLocalBounds();
                            escText.setPosition((1200-escBound.width) / 2, 700);
                            window.draw(escText);

                            window.display();
                        }
                    }

                    //CHOICE 2 - Weight Selection (contains the vector <int> weights)
                    if (choice == 2) {
                        int selected= 0;
                        string inputString = "";

                        bool Option2 = true;

                        vector<string> factors = {
                            "Timeliness of care",
                            "Effectiveness of treatment",
                            "Patient experience",
                            "Distance",
                            "Preventive care",
                            "Emergency department safety quality"
                        };

                        // Store the location for each weight for click detection in a vector / Used for red box selection
                        vector<sf::FloatRect> weightBounds(6);

                        sf::Event clearEvent;
                        while (window.pollEvent(clearEvent)) {
                        }

                        while (Option2 && window.isOpen()) {
                            sf::Event weightEvent;
                            while (window.pollEvent(weightEvent)) {
                                if (weightEvent.type == sf::Event::Closed) {
                                    window.close();
                                    Option2 = false;
                                }
                                if (weightEvent.type == sf::Event::KeyPressed) {
                                    if (weightEvent.key.code == sf::Keyboard::Escape) {
                                        Option2 = false;
                                    }
                                    // if enter is pressed, check if user input is valid
                                    if (weightEvent.key.code == sf::Keyboard::Enter && !inputString.empty()) {
                                        // Check if inputString contains only digits 1-5
                                        bool weightValid = true;
                                        for (char c : inputString) {
                                            if (c < '1' || c > '5') {
                                                weightValid = false;
                                                break;
                                            }
                                        }
                                        //if user input for weight is valid, add weight to vector weight
                                        if (weightValid && inputString.length() == 1) {
                                            int newWeight = inputString[0] - '0';
                                            weights[selected] = newWeight;
                                        }
                                        inputString = "";
                                    }
                                }
                                // only allow user input to be a number between 0 and 100. If backspace, remove last digit
                                // of inputstring
                                if (weightEvent.type == sf::Event::TextEntered) {
                                    if (weightEvent.text.unicode >= '0' && weightEvent.text.unicode <= '5' && inputString.length() < 1){
                                        inputString += static_cast<char>(weightEvent.text.unicode);
                                    }
                                    if (weightEvent.text.unicode == '\b' && !inputString.empty()){
                                        inputString.pop_back();
                                    }
                                }
                                if (weightEvent.type == sf::Event::MouseButtonPressed){
                                    if (weightEvent.mouseButton.button == sf::Mouse::Left){
                                        sf::Vector2i mousePosition = sf::Mouse::getPosition(window);

                                        // Check if user clicked on any of the weight choices
                                        for (int i = 0; i < 6; i++) {
                                            if (weightBounds[i].contains(mousePosition.x, mousePosition.y)){
                                                selected= i;
                                                inputString = "";
                                                break;
                                            }
                                        }
                                    }
                                }
                            }

                            // Draw Option 2 Interface
                            window.clear(sf::Color::White);

                            sf::Text title("Customize Hospital Priorities", font, 40);
                            title.setFillColor(sf::Color::Blue);
                            title.setStyle(sf::Text::Bold);
                            sf::FloatRect titleBounds = title.getLocalBounds();
                            title.setPosition((1200 - titleBounds.width) / 2, 50);
                            window.draw(title);

                            // Instructions
                            sf::Text instruction("Click on a factor to select it, then type a number ([least] 1-5 [most]) and press ENTER", font, 24);
                            instruction.setFillColor(sf::Color::Blue);
                            sf::FloatRect instructionBounds = instruction.getLocalBounds();
                            instruction.setPosition((1200 - instructionBounds.width) / 2, 100);
                            window.draw(instruction);

                            // Display factors with current weights and selection boxes
                            for (int i = 0; i < 6; i++) {
                                sf::Text factorText(to_string(i+1) + ". " + factors[i] + ": [ " + to_string(weights[i]) + " ]",
                                    font, 28);
                                factorText.setFillColor(sf::Color::Black);
                                sf::FloatRect factorBounds = factorText.getLocalBounds();
                                factorText.setPosition((1200 - factorBounds.width) / 2, 150 + i * 60);

                                // Store the bounding box for click detection
                                weightBounds[i] = factorText.getGlobalBounds();

                                // Draw red selection box around currently selected weight
                                if (i == selected) {
                                    sf::RectangleShape selectionBox(sf::Vector2f(factorBounds.width + 20, factorBounds.height + 15));
                                    selectionBox.setFillColor(sf::Color::Transparent);
                                    selectionBox.setOutlineColor(sf::Color::Red);
                                    selectionBox.setOutlineThickness(2);
                                    selectionBox.setPosition((1200 - factorBounds.width) / 2 - 10, 145 + i * 60);
                                    window.draw(selectionBox);
                                }

                                window.draw(factorText);
                            }

                            // Selected weight info
                            sf::Text selectedInfo("Selected: " + factors[selected], font, 24);
                            selectedInfo.setFillColor(sf::Color::Red);
                            selectedInfo.setStyle(sf::Text::Bold);
                            sf::FloatRect infoBounds = selectedInfo.getLocalBounds();
                            selectedInfo.setPosition((1200 - infoBounds.width) / 2, 500);
                            window.draw(selectedInfo);

                            // Current weight value
                            sf::Text currentWeight("Current value: " + to_string(weights[selected]), font, 22);
                            currentWeight.setFillColor(sf::Color::Blue);
                            sf::FloatRect currentWeightBounds = currentWeight.getLocalBounds();
                            currentWeight.setPosition((1200 - currentWeightBounds.width) / 2, 530);
                            window.draw(currentWeight);

                            // Input area
                            sf::Text inputLabel("Enter new value (1-5):", font, 22);
                            inputLabel.setFillColor(sf::Color::Green);
                            sf::FloatRect inputBounds = inputLabel.getLocalBounds();
                            inputLabel.setPosition((1200 - inputBounds.width) / 2, 560);
                            window.draw(inputLabel);

                            sf::RectangleShape inputBox(sf::Vector2f(200, 40));
                            inputBox.setFillColor(sf::Color(240, 240, 240));
                            inputBox.setOutlineColor(sf::Color::Green);
                            inputBox.setOutlineThickness(3);
                            inputBox.setPosition((1000) / 2, 595);
                            window.draw(inputBox);

                            if (!inputString.empty()) {
                                sf::Text inputDisplay(inputString, font, 24);
                                inputDisplay.setFillColor(sf::Color::Black);
                                sf::FloatRect inputDisplayBounds = inputDisplay.getLocalBounds();
                                inputDisplay.setPosition((1200 - inputDisplayBounds.width) / 2, 600);
                                window.draw(inputDisplay);
                            } else {
                                // Show message when empty
                                sf::Text placeholder("type here", font, 20);
                                placeholder.setFillColor(sf::Color::Black);
                                sf::FloatRect placeholderBounds = placeholder.getLocalBounds();
                                placeholder.setPosition((1200 - placeholderBounds.width) / 2, 600);
                                window.draw(placeholder);
                            }

                            sf::Text navText("Press ENTER to apply, ESC to return to main menu", font, 20);
                            navText.setFillColor(sf::Color::Green);
                            sf::FloatRect navBounds = navText.getLocalBounds();
                            navText.setPosition((1200 - navBounds.width) / 2, 700);
                            window.draw(navText);

                            window.display();
                        }
                    }
                    //Option 3 - View Recommended Hospitals
                    if (choice==3) {
                        sf::Event clearEvent;
                        while (window.pollEvent(clearEvent)) {}
                        bool Option3 = true;

                        while (Option3 && window.isOpen()){
                            sf::Event Opt3Event;
                            while (window.pollEvent(Opt3Event)) {
                                if (Opt3Event.type == sf::Event::Closed) {
                                    window.close();
                                }
                                if (Opt3Event.type == sf::Event::KeyPressed){
                                    if ( Opt3Event.key.code == sf::Keyboard::Escape){
                                        Option3 = false;
                                    }
                                    if (Opt3Event.key.code >= sf::Keyboard::Num1 && Opt3Event.key.code <= sf::Keyboard::Num5) {
                                        // Logic for selecting a specific hospital (e.g., for Option 4)
                                        selectedHospitalInd = Opt3Event.key.code - sf::Keyboard::Num1;
                                    }
                                }
                            }

                            //Drawing option 3 interface
                            window.clear(sf::Color::White);

                            sf::Text title("Top Recommended Hospitals", font, 40);
                            title.setFillColor(sf::Color::Blue);
                            title.setStyle(sf::Text::Bold);
                            sf::FloatRect titleBounds = title.getLocalBounds();
                            title.setPosition((1200 - titleBounds.width) / 2, 50);
                            window.draw(title);

                            if (!citySelected) {
                                sf::Text warn("Please set your location (Option 1) first!", font, 30);
                                warn.setFillColor(sf::Color::Red);
                                sf::FloatRect warnBounds = warn.getLocalBounds();
                                warn.setPosition((1200 - warnBounds.width) / 2, 350);
                                window.draw(warn);
                            } else if (topHospitals.empty()) {
                                sf::Text info("No hospitals found within " + std::to_string(userDistanceMiles) + " miles of " + city + ".", font, 30);
                                info.setFillColor(sf::Color::Black);
                                sf::FloatRect infoBounds = info.getLocalBounds();
                                info.setPosition((1200 - infoBounds.width) / 2, 200);
                                window.draw(info);
                            }
                            else {
                                sf::Text subtitle("Top " + std::to_string(topHospitals.size()) + " hospitals near " + city + " (" + std::to_string(userDistanceMiles) + " miles radius)", font, 24);
                                subtitle.setFillColor(sf::Color::Blue);
                                sf::FloatRect subtitleBounds = subtitle.getLocalBounds();
                                subtitle.setPosition((1200 - subtitleBounds.width) / 2, 120);
                                window.draw(subtitle);

                                // Draw table headers
                                sf::Text rankCol("Rank", font, 28);
                                rankCol.setFillColor(sf::Color::Black);
                                rankCol.setStyle(sf::Text::Bold);
                                rankCol.setPosition(100, 180);
                                window.draw(rankCol);

                                sf::Text hospitalCol("Hospital Name", font, 28);
                                hospitalCol.setFillColor(sf::Color::Black);
                                hospitalCol.setStyle(sf::Text::Bold);
                                hospitalCol.setPosition(200, 180);
                                window.draw(hospitalCol);

                                sf::Text cityCol("Location", font, 28);
                                cityCol.setFillColor(sf::Color::Black);
                                cityCol.setStyle(sf::Text::Bold);
                                cityCol.setPosition(700, 180);
                                window.draw(cityCol);

                                sf::Text scoreCol("Score | Distance", font, 28);
                                scoreCol.setFillColor(sf::Color::Black);
                                scoreCol.setStyle(sf::Text::Bold);
                                scoreCol.setPosition(900, 180);
                                window.draw(scoreCol);

                                // Display the top 10 hospitals
                                for (size_t i = 0; i < topHospitals.size(); ++i) {
                                    const auto& result = topHospitals[i];
                                    int yPos = 220 + i * 40;

                                    // Rank
                                    sf::Text rankText(std::to_string(i + 1) + ".", font, 24);
                                    rankText.setFillColor(sf::Color::Black);
                                    rankText.setPosition(100, yPos);
                                    window.draw(rankText);

                                    // Hospital Name
                                    sf::Text nameText(result.hospital.name, font, 24);
                                    nameText.setFillColor(sf::Color::Black);
                                    nameText.setPosition(200, yPos);
                                    window.draw(nameText);

                                    // Location
                                    std::string location = result.hospital.city + ", " + result.hospital.state;
                                    sf::Text locText(location, font, 24);
                                    locText.setFillColor(sf::Color::Black);
                                    locText.setPosition(700, yPos);
                                    window.draw(locText);

                                    // Score and Distance
                                    std::stringstream ss;
                                    ss.precision(2);
                                    ss << std::fixed << result.score << " | " << (result.distanceKm / MILE_TO_KM);
                                    std::string scoreDist = ss.str() + " mi";

                                    sf::Text scoreDistText(scoreDist, font, 24);
                                    scoreDistText.setFillColor(sf::Color::Black);
                                    scoreDistText.setPosition(900, yPos);
                                    window.draw(scoreDistText);
                                }
                            }

                            sf::Text escText("Press ESC to return to main menu", font, 18);
                            escText.setFillColor(sf::Color::Green);
                            sf::FloatRect escBound=escText.getLocalBounds();
                            escText.setPosition((1200-escBound.width) / 2, 750);
                            window.draw(escText);

                            window.display();
                        }
                    }
                    //Option 4 - Display Hospital Details
                    if (choice == 4) {
                        // Logic for option 4
                    }

                    //Option 5 - Export Results
                    if (choice == 5) {
                        // Logic for option 5
                    }

                    //Option 6 - Choose Data Structure
                    if (choice == 6) {
                        // Logic for option 6
                    }

                    //Option 7 - About
                    if (choice == 7) {
                        // Logic for option 7
                    }

                    //Option 8 - Exit
                    if (choice == 8) {
                        window.close();
                    }
                }
            }
        }

        // Drawing main menu
        if (choice == 0 || (choice >= 1 && choice <= 8 && !window.pollEvent(event))) {
            window.clear(sf::Color::White);
            window.draw(titleText);
            window.draw(subtitleText);
            window.draw(separator1);
            window.draw(separator2);
            window.draw(menuText);
            window.draw(promptText);

            if (citySelected) {
                sf::Text locationStatus("Location Set: " + city + " (" + std::to_string(userDistanceMiles) + " miles)", font, 20);
                locationStatus.setFillColor(sf::Color(0, 100, 0));
                sf::FloatRect statusBounds = locationStatus.getLocalBounds();
                locationStatus.setPosition((1200 - statusBounds.width) / 2, 230);
                window.draw(locationStatus);
            }

            window.display();
        }
    }

    return 0;
}