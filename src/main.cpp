
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <cmath>
#include "geofilter.h"
#include "hospital.h"
// #include "scoring.h"
#include "csv_parser.h"
#include "user_prefs.h"
#include "assign_coords.h"
#include "city_coords.h"
using namespace std;


double userLat = 0.0;
double userLon = 0.0;
string userCityState = "";


struct HospitalResult {
    Hospital hospital;
    double distanceKm;
    double score;
};


vector<HospitalResult> topHospitals;

constexpr double MILE_TO_KM = 1.60934;

// Helper function to find coordinates for a city name (simple search)
bool findCityCoords(const string& cityName, const unordered_map<string,pair<double, double>>& coords, double& lat, double& lon, string& foundCityState) {
    string standardizedCity = cityName;

    for (auto& pair : coords) {
        size_t commaPos = pair.first.find(',');
        if (commaPos != string::npos) {
            string keyCity = pair.first.substr(0, commaPos);

            if (keyCity == standardizedCity) {
                lat = pair.second.first;
                lon = pair.second.second;
                foundCityState = pair.first;
                return true;
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

    UserPreferences prefs = makeUserPreferences(maxDistanceMiles, weights);

    // auto scored = compute(const_cast<vector<Hospital>&>(allHospitals), uLat, uLon, prefs);

    // for (auto &entry : scored) {
    //     HospitalResult hr;
    //     hr.hospital = *(entry.second);
    //     hr.distanceKm = GeoFilter::haversine(uLat, uLon, hr.hospital.latitude, hr.hospital.longitude) * 1.60934;
    //     hr.score = entry.first;
    //     results.push_back(hr);
    //
    //     if (results.size() >= 5) break;
    // }

    return results;
}



int main() {
    //
    // vector<Hospital> hospitals = parseHospitalCSV("../data/hospitals.csv");
    // unordered_map<string, pair<double, double>> coords = loadCityCoords("../data/uscities.csv");
    // assignCoordinates(hospitals, coords);

    sf::RenderWindow window(sf::VideoMode(1200, 800), "MedMetrics");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("../include/arial.ttf")) {
        return -1;
    }

    sf::Text titleText("Welcome to MedMetrics", font, 40);
    titleText.setFillColor(sf::Color::Blue);
    titleText.setStyle(sf::Text::Bold);
    sf::FloatRect titleBound = titleText.getLocalBounds();
    titleText.setPosition((1200 - titleBound.width) / 2, 80);

    sf::Text subtitleText("Geofiltered Hospital Rankings Program", font, 30);
    subtitleText.setFillColor(sf::Color::Blue);
    sf::FloatRect subtitleBound = subtitleText.getLocalBounds();
    subtitleText.setPosition((1200 - subtitleBound.width) / 2, 130);

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

    int choice;
    vector<int> weights = {3, 3, 3, 3, 3, 3};
    int selectedHospitalInd = -1;
    int DSindicator = 1;
    string city = "";
    int userDistanceMiles = 0;

    bool citySelected = false;
    // Main loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code >= sf::Keyboard::Num1 && event.key.code <= sf::Keyboard::Num8) {
                    choice = event.key.code - sf::Keyboard::Num1 + 1;
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
                                    if (!inputName.empty() && !nameFound) {
                                        nameEnteredOnce = true;
                                        // if (findCityCoords(inputName, coords, userLat, userLon, userCityState)) {
                                        //     nameFound = true;
                                        //     cityNameDisplay = inputName;
                                        //     city = inputName;
                                        //     cityNameDisplay[0] = toupper(cityNameDisplay[0]);
                                        // }
                                        // else {
                                        //     inputName = "";
                                        // }
                                    }

                                    else if (nameFound && !inputDistance.empty() && !distanceEnter) {
                                        try {
                                            int distanceValue = stoi(inputDistance);
                                            if (distanceValue > 0 && distanceValue < 1000) {
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
                            window.clear(sf::Color::White);

                            sf::Text title("Set Location and Search Radius", font, 40);
                            title.setFillColor(sf::Color::Blue);
                            title.setStyle(sf::Text::Bold);
                            sf::FloatRect titleBound = title.getLocalBounds();
                            title.setPosition((1200 - titleBound.width) / 2, 50);
                            window.draw(title);

                            if (!nameFound  && !nameEnteredOnce) {
                                sf::Text prompt1("Enter your city name:", font, 30);
                                prompt1.setFillColor(sf::Color::Black);
                                sf::FloatRect prompt1Bounds = prompt1.getLocalBounds();
                                prompt1.setPosition((1200 - prompt1Bounds.width) / 2, 150);
                                window.draw(prompt1);

                                sf::RectangleShape inputBox(sf::Vector2f(600, 50));
                                inputBox.setFillColor(sf::Color(240, 240, 240));
                                inputBox.setOutlineColor(sf::Color::Blue);
                                inputBox.setOutlineThickness(2);
                                inputBox.setPosition(300, 220);
                                window.draw(inputBox);

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

                                sf::Text prompt2("Enter maximum distance (miles) from " + cityNameDisplay + ":", font, 30);
                                prompt2.setFillColor(sf::Color::Black);
                                sf::FloatRect prompt2Bounds = prompt2.getLocalBounds();
                                prompt2.setPosition((1200 - prompt2Bounds.width) / 2, 150);
                                window.draw(prompt2);

                                sf::RectangleShape distanceBox(sf::Vector2f(200, 50));
                                distanceBox.setFillColor(sf::Color(240, 240, 240));
                                distanceBox.setOutlineColor(sf::Color::Blue);
                                distanceBox.setOutlineThickness(2);
                                distanceBox.setPosition(500, 220);
                                window.draw(distanceBox);

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
                                // topHospitals = findClosestHospitals(hospitals, userLat, userLon, radiusKm, weights);

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
                                    // check if user input is valid
                                    if (weightEvent.key.code == sf::Keyboard::Enter && !inputString.empty()) {
                                        // Check if inputString contains only digits 1-5
                                        bool weightValid = true;
                                        for (char c : inputString) {
                                            if (c < '1' || c > '5') {
                                                weightValid = false;
                                                break;
                                            }
                                        }

                                        if (weightValid && inputString.length() == 1) {
                                            int newWeight = inputString[0] - '0';
                                            weights[selected] = newWeight;
                                        }
                                        inputString = "";
                                    }
                                }
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

                            for (int i = 0; i < 6; i++) {
                                sf::Text factorText(to_string(i+1) + ". " + factors[i] + ": [ " + to_string(weights[i]) + " ]",
                                    font, 28);
                                factorText.setFillColor(sf::Color::Black);
                                sf::FloatRect factorBounds = factorText.getLocalBounds();
                                factorText.setPosition((1200 - factorBounds.width) / 2, 150 + i * 60);

                                weightBounds[i] = factorText.getGlobalBounds();

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
                                        selectedHospitalInd = Opt3Event.key.code - sf::Keyboard::Num1;
                                    }
                                }
                            }
                            window.clear(sf::Color::White);

                            sf::Text title("Top Recommended Hospitals", font, 40);
                            title.setFillColor(sf::Color::Blue);
                            title.setStyle(sf::Text::Bold);
                            sf::FloatRect titleBounds = title.getLocalBounds();
                            title.setPosition((1200 - titleBounds.width) / 2, 50);
                            window.draw(title);

                            if (!citySelected) {
                                sf::Text warn("Please set your location first!", font, 30);
                                warn.setFillColor(sf::Color::Red);
                                sf::FloatRect warnBounds = warn.getLocalBounds();
                                warn.setPosition((1200 - warnBounds.width) / 2, 350);
                                window.draw(warn);
                            } else if (topHospitals.empty()) {
                                sf::Text info("No hospitals found within " + to_string(userDistanceMiles) + " miles of " + city + ".", font, 30);
                                info.setFillColor(sf::Color::Black);
                                sf::FloatRect infoBounds = info.getLocalBounds();
                                info.setPosition((1200 - infoBounds.width) / 2, 200);
                                window.draw(info);
                            }
                            else {
                                sf::Text subtitle("Top " + to_string(topHospitals.size()) + " hospitals near " + city + " (" +to_string(userDistanceMiles) + " miles radius)", font, 24);
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

                                    sf::Text rankText(to_string(i + 1) + ".", font, 24);
                                    rankText.setFillColor(sf::Color::Black);
                                    rankText.setPosition(100, yPos);
                                    window.draw(rankText);

                                    sf::Text nameText(result.hospital.name, font, 24);
                                    nameText.setFillColor(sf::Color::Black);
                                    nameText.setPosition(200, yPos);
                                    window.draw(nameText);

                                    string location = result.hospital.city + ", " + result.hospital.state;
                                    sf::Text locText(location, font, 24);
                                    locText.setFillColor(sf::Color::Black);
                                    locText.setPosition(700, yPos);
                                    window.draw(locText);

                                    stringstream ss;
                                    ss.precision(2);
                                    ss << fixed << result.score << " | " << (result.distanceKm / MILE_TO_KM);
                                    string scoreDist = ss.str() + " mi";

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
                    if (choice==4) {
                        sf::Event clearEvent;
                        while (window.pollEvent(clearEvent)) {}
                        bool Option4 = true;
                        //event processing for option 4
                        while (Option4 && window.isOpen()){
                            sf::Event aboutEvent;
                            while (window.pollEvent(aboutEvent)){
                                if (aboutEvent.type == sf::Event::Closed){
                                    window.close();
                                }
                                if (aboutEvent.type == sf::Event::KeyPressed && aboutEvent.key.code == sf::Keyboard::Escape){
                                    Option4 = false;
                                }


                            }
                            // Instructions
                            window.clear(sf::Color::White);

                            if (selectedHospitalInd == -1 && citySelected) {
                                sf::Text instructionErrorTitle("No Hospital Details Displayed"
                                    , font, 40);
                                instructionErrorTitle.setFillColor(sf::Color::Red);
                                sf::FloatRect instructionErrorTitleBounds = instructionErrorTitle.getLocalBounds();
                                instructionErrorTitle.setPosition((1200 - instructionErrorTitleBounds.width) / 2, 100);
                                window.draw(instructionErrorTitle);

                                sf::Text instructionError("Please complete option 1 from the main menu or select a ranked hospital from option 3 before continuing"
                                    , font, 25);
                                instructionError.setFillColor(sf::Color::Green);
                                sf::FloatRect instructionErrorBounds = instructionError.getLocalBounds();
                                instructionError.setPosition((1200 - instructionErrorBounds.width) / 2, 200);
                                window.draw(instructionError);
                            }
                            else if (selectedHospitalInd >=0 && selectedHospitalInd <= 4 && citySelected) {
                                vector<string> categoryNames = {
                                    "Timeliness of care",
                                    "Effectiveness of treatment",
                                    "Patient experience",
                                    "Emergency department quality",
                                    "Preventive care"
                                };

                                // Title
                                sf::Text title("Hospital Details", font, 40);
                                title.setFillColor(sf::Color::Blue);
                                title.setStyle(sf::Text::Bold);
                                sf::FloatRect titleBounds = title.getLocalBounds();
                                title.setPosition((1200 - titleBounds.width) / 2, 50);
                                window.draw(title);

                                // Hospital Name
                                sf::Text hospitalName("Details for: [hospital Name]", font, 32);
                                hospitalName.setFillColor(sf::Color::Black);
                                hospitalName.setStyle(sf::Text::Bold);
                                sf::FloatRect nameBounds = hospitalName.getLocalBounds();
                                hospitalName.setPosition((1200 - nameBounds.width) / 2, 120);
                                window.draw(hospitalName);

                                // Detailed info for selected hospital (anything in [] will need to be replaced with their
                                //actual values.
                                sf::Text cityText("City: [name], [state]", font, 28);
                                cityText.setFillColor(sf::Color::Black);
                                cityText.setPosition(200, 180);
                                window.draw(cityText);

                                sf::Text distanceText("Distance: [miles] miles", font, 28); // Placeholder
                                distanceText.setFillColor(sf::Color::Black);
                                distanceText.setPosition(200, 220);
                                window.draw(distanceText);

                                sf::Text scoreText("Overall Score: [score]", font, 28); // Placeholder
                                scoreText.setFillColor(sf::Color::Black);
                                scoreText.setPosition(200, 260);
                                window.draw(scoreText);

                                // Category Breakdown Title
                                sf::Text categoryTitle("Category Breakdown:", font, 28);
                                categoryTitle.setFillColor(sf::Color::Blue);
                                categoryTitle.setStyle(sf::Text::Bold);
                                categoryTitle.setPosition(200, 320);
                                window.draw(categoryTitle);

                                // Category Scores - PLACEHOLDERS need to be replaced with actual values
                                //categories with scores will be iterated and displayed with the for loop below
                                vector<string> categories = {
                                    "Timeliness of care: [score] / 5",
                                    "Effectiveness: [score] / 5",
                                    "Patient Experience: [score] / 5",
                                    "Emergency Department: [score] / 5",
                                    "Preventive Care: [score] / 5"
                                };

                                for (int i = 0; i < categories.size(); i++) {
                                    sf::Text categoryText(categories[i], font, 24);
                                    categoryText.setFillColor(sf::Color::Black);
                                    categoryText.setPosition(220, 370 + i * 35);
                                    window.draw(categoryText);
                                }




                            }
                            sf::Text instruction1("Press ESC to return to main menu", font, 23);
                            instruction1.setFillColor(sf::Color::Green);
                            sf::FloatRect instruction1Bounds = instruction1.getLocalBounds();
                            instruction1.setPosition((1200 - instruction1Bounds.width) / 2, 750);
                            window.draw(instruction1);

                            window.display();
                        }
                    }

                    //Option 5 - Export Results
                    if (choice == 5) {
                         sf::Event clearEvent;
                        while (window.pollEvent(clearEvent)) {}
                        bool Option5 = true;
                        string userInput = "";
                        bool exportSuccess = false;

                        while (Option5 && window.isOpen()) {
                            sf::Event Opt5Event;
                            while (window.pollEvent(Opt5Event)) {
                                if (Opt5Event.type == sf::Event::Closed) {
                                    window.close();
                                }
                                if (Opt5Event.type == sf::Event::KeyPressed && Opt5Event.key.code == sf::Keyboard::Escape) {
                                    Option5 = false;
                                }

                                if (Opt5Event.type == sf::Event::KeyPressed && Opt5Event.key.code == sf::Keyboard::Enter) {
                                    if (userInput == "Y" || userInput == "y") {
                                        // Export results
                                        ofstream file("results.txt");
                                        if (file.is_open()) {

                                            file << "Results\n";
                                            file << "=======\n";
                                            file << "Top 5 Recommended Hospitals:\n";
                                            file << "----------------------------\n";

                                            //REPLACE PLACEHOLDERS
                                            for (int i = 0; i < 5; i++) {
                                                file << i + 1 << ". [Hospital Name] - [City, State] - Score: [score]\n";
                                            }

                                            file << "\n";
                                            file << "Inputs\n";
                                            file << "------\n";

                                            // Replace placeholders
                                            file << "Location: " << "[City Name]" << "\n";
                                            file << "Search Radius: " << "[distance] miles" << "\n\n";

                                            // Weights
                                            file << "Category Weights (1-5):\n";
                                            vector<string> categories = {
                                                "Timeliness of care", "Effectiveness of treatment", "Patient experience",
                                                "Distance", "Preventive care", "Emergency department quality"
                                            };

                                            for (int i = 0; i < categories.size() && i < weights.size(); i++) {
                                                file << "- " << categories[i] << ": " << weights[i] << "\n";
                                            }

                                            file.close();
                                            exportSuccess = true;
                                        }
                                    }
                                    userInput = "";
                                }


                                if (Opt5Event.type == sf::Event::TextEntered) {
                                    if (Opt5Event.text.unicode == '\b' && !userInput.empty()) {
                                        userInput.pop_back();
                                    }
                                    else if (isalpha(Opt5Event.text.unicode) && userInput.length() < 1) {
                                        userInput += static_cast<char>(toupper(Opt5Event.text.unicode));
                                    }
                                }
                            }

                            window.clear(sf::Color::White);

                            sf::Text title("Export Results", font, 40);
                            title.setFillColor(sf::Color::Blue);
                            title.setStyle(sf::Text::Bold);
                            sf::FloatRect titleBounds = title.getLocalBounds();
                            title.setPosition((1200 -titleBounds.width) / 2, 50);
                            window.draw(title);

                            //export File
                            if (!exportSuccess) {
                                sf::Text prompt("Would you like to export results?", font, 30);
                                prompt.setFillColor(sf::Color::Black);
                                sf::FloatRect promptBounds=prompt.getLocalBounds();
                                prompt.setPosition((1200 -promptBounds.width) / 2, 150);
                                window.draw(prompt);

                                sf::RectangleShape inputBox(sf::Vector2f(200, 50));
                                inputBox.setFillColor(sf::Color(240, 240, 240));
                                inputBox.setOutlineColor(sf::Color::Blue);
                                inputBox.setOutlineThickness(2);
                                inputBox.setPosition(500, 220);
                                window.draw(inputBox);

                                sf::Text inputDisplay(userInput, font, 28);
                                inputDisplay.setFillColor(sf::Color::Black);
                                sf::FloatRect textBounds = inputDisplay.getLocalBounds();
                                inputDisplay.setPosition((1200-textBounds.width) / 2, 225);
                                window.draw(inputDisplay);

                                sf::Text instruction("Type Y and press ENTER", font, 20);
                                instruction.setFillColor(sf::Color::Green);
                                sf::FloatRect instructionBounds = instruction.getLocalBounds();
                                instruction.setPosition((1200-instructionBounds.width)/2, 300);
                                window.draw(instruction);
                            }
                            else {
                                // Success message
                                sf::Text success("Export successful!", font, 36);
                                success.setFillColor(sf::Color::Green);
                                success.setStyle(sf::Text::Bold);
                                sf::FloatRect successBounds = success.getLocalBounds();
                                success.setPosition((1200 - successBounds.width) / 2, 200);
                                window.draw(success);

                                sf::Text fileInfo("results.txt file created/updated", font, 28);
                                fileInfo.setFillColor(sf::Color::Black);
                                sf::FloatRect fileBounds = fileInfo.getLocalBounds();
                                fileInfo.setPosition((1200 -fileBounds.width) / 2, 280);
                                window.draw(fileInfo);
                            }

                            sf::Text escText("Press ESC to return to main menu", font, 25);
                            escText.setFillColor(sf::Color::Green);
                            sf::FloatRect escBounds = escText.getLocalBounds();
                            escText.setPosition((1200- escBounds.width) / 2, 700);
                            window.draw(escText);

                            window.display();
                        }
                    }

                   if (choice == 6) {
                       bool Option6=true;
                       string userInput="";
                       string DS;
                       if (DSindicator == 1) {
                           DS = "Geohash data structure";
                       } else if (DSindicator == 2) {
                           DS = "Octree data structure";
                       }

                       while (Option6 &&window.isOpen()) {
                           sf::Event Opt6event;
                           while (window.pollEvent(Opt6event)) {
                               if (Opt6event.type == sf::Event::Closed){
                                   window.close();
                               }
                               if (Opt6event.type == sf::Event::KeyPressed){
                                   if (Opt6event.key.code == sf::Keyboard::Escape) {
                                       Option6=false;
                                   }
                                   else if (Opt6event.key.code == sf::Keyboard::Enter) {
                                       if (userInput =="1") {
                                           DS = "Geohash data structure";
                                           DSindicator = 1;
                                       }
                                       else if (userInput == "2") {
                                           DS = "Octree data structure";
                                           DSindicator = 2;
                                       }
                                       userInput="";
                                   }
                               }
                               if (Opt6event.type == sf::Event::TextEntered) {
                                   if (Opt6event.text.unicode == '1') {
                                       userInput = "1";
                                   }
                                   else if (Opt6event.text.unicode == '2') {
                                       userInput = "2";
                                   }
                               }
                           }
                           window.clear(sf::Color::White);

                           sf::Text Title("Select which data structure you would like to use", font, 40);
                           Title.setFillColor(sf::Color::Blue);
                           Title.setStyle(sf::Text::Bold);
                           sf::FloatRect TitleBounds = Title.getLocalBounds();
                           Title.setPosition((1200- TitleBounds.width) / 2, 50);
                           window.draw(Title);

                           sf::Text Instruction("To use a geohash data structure press 1 and enter. To use an Octree data structure press 2 and enter", font, 22);
                           Instruction.setFillColor(sf::Color::Blue);
                           Instruction.setStyle(sf::Text::Bold);
                           sf::FloatRect InstructionBounds = Instruction.getLocalBounds();
                           Instruction.setPosition((1200- InstructionBounds.width) / 2, 200);
                           window.draw(Instruction);

                           if (DSindicator == 1) {
                               sf::Text Instruction2("You have selected "+ DS, font, 25);
                               Instruction2.setFillColor(sf::Color::Black);
                               Instruction2.setStyle(sf::Text::Bold);
                               sf::FloatRect Instruction2Bounds = Instruction2.getLocalBounds();
                               Instruction2.setPosition((1200- Instruction2Bounds.width) / 2, 500);
                               window.draw(Instruction2);
                           }
                           if (DSindicator == 2) {
                               sf::Text Instruction2("You have selected "+ DS, font, 25);
                               Instruction2.setFillColor(sf::Color::Black);
                               Instruction2.setStyle(sf::Text::Bold);
                               sf::FloatRect Instruction2Bounds = Instruction2.getLocalBounds();
                               Instruction2.setPosition((1200- Instruction2Bounds.width) / 2, 500);
                               window.draw(Instruction2);
                           }

                           sf::Text backText("Press ESC to return to main menu", font, 25);
                           backText.setFillColor(sf::Color::Green);
                           sf::FloatRect backBounds = backText.getLocalBounds();
                           backText.setPosition((1200 - backBounds.width) / 2, 750);
                           window.draw(backText);
                       }
                   }

                    //Option 7 - About
                    if (choice == 7) {
                        bool Option7 = true;
                        while (Option7 && window.isOpen()){
                            sf::Event aboutEvent;
                            while (window.pollEvent(aboutEvent)){
                                if (aboutEvent.type == sf::Event::Closed){
                                    window.close();
                                }
                                if (aboutEvent.type == sf::Event::KeyPressed && aboutEvent.key.code == sf::Keyboard::Escape){
                                    Option7 = false;
                                }
                            }
                            // Draw About screen
                            window.clear(sf::Color::White);

                            sf::Text aboutTitle("About MedMetrics |+|", font, 40);
                            aboutTitle.setFillColor(sf::Color::Blue);
                            aboutTitle.setStyle(sf::Text::Bold);
                            sf::FloatRect aboutTitleBounds = aboutTitle.getLocalBounds();
                            aboutTitle.setPosition((1200- aboutTitleBounds.width) / 2, 50);
                            window.draw(aboutTitle);

                            sf::RectangleShape aboutSeparator(sf::Vector2f(1100, 2));
                            aboutSeparator.setFillColor(sf::Color::Black);
                            aboutSeparator.setPosition(50, 110);
                            window.draw(aboutSeparator);

                            // Description
                            sf::Text descText("MedMetrics helps patients find the hospital that best fits their personal priorities."
                                              "\nFeatures:"
                                              "\n - Choose your city and distance limit"
                                              "\n - Set importance levels for hospital characteristics"
                                              "\n - Receive a personalized ranked list of hospitals nearby", font, 24);
                            descText.setFillColor(sf::Color::Black);
                            descText.setLineSpacing(1.3f);
                            sf::FloatRect descBounds = descText.getLocalBounds();
                            descText.setPosition((1200 -descBounds.width) / 2, 130);
                            window.draw(descText);

                            // Developed by
                            sf::Text devTitle("Developed by:", font, 40);
                            devTitle.setFillColor(sf::Color::Blue);
                            devTitle.setStyle(sf::Text::Bold);
                            sf::FloatRect devBounds = devTitle.getLocalBounds();
                            devTitle.setPosition((1200 - devBounds.width) / 2, 350);
                            window.draw(devTitle);

                            sf::Text devNames("Kaylee Driscoll - Category weighting and sorting"
                                              "\nRyan Truonghuynh - Visual/GUI Implementation"
                                              "\nMaximilian Preble - Geohashing and location filtering", font, 25);
                            devNames.setFillColor(sf::Color::Black);
                            devNames.setLineSpacing(1.2f);
                            sf::FloatRect namesBounds = devNames.getLocalBounds();
                            devNames.setPosition((1200 -namesBounds.width) / 2, 420);
                            window.draw(devNames);

                            sf::Text dataTitle("Data Source:", font, 37);
                            dataTitle.setFillColor(sf::Color::Blue);
                            dataTitle.setStyle(sf::Text::Bold);
                            sf::FloatRect dataBounds = dataTitle.getLocalBounds();
                            dataTitle.setPosition((1200- dataBounds.width) / 2, 560);
                            window.draw(dataTitle);

                            sf::Text dataText("CORGIS Hospitals Dataset"
                                              "\nhttps://corgis-edu.github.io/corgis/csv/hospitals/", font, 24);
                            dataText.setFillColor(sf::Color::Black);
                            dataText.setLineSpacing(1.2f);
                            sf::FloatRect dataTextBounds = dataText.getLocalBounds();
                            dataText.setPosition((1200 - dataTextBounds.width) / 2, 620);
                            window.draw(dataText);

                            sf::Text backText("Press ESC to return to main menu", font, 25);
                            backText.setFillColor(sf::Color::Green);
                            sf::FloatRect backBounds = backText.getLocalBounds();
                            backText.setPosition((1200 - backBounds.width) / 2, 750);
                            window.draw(backText);
                        }
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
                sf::Text locationStatus("Location Set: " + city + " (" + to_string(userDistanceMiles) + " miles)", font, 20);
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
