#pragma once
#include <vector>
#include <memory>
#include <utility>
#include <string>
#include "hospital.h"

struct QBox {
    double minLat, maxLat;
    double minLon, maxLon;
    bool contains(double lat, double lon) const;
    bool intersects(const QBox& other) const;
};

class QuadTree {
public:
    QuadTree(const QBox& bounds, int capacity = 16);
    bool insert(const Hospital& h);
    void queryBox(const QBox& range, std::vector<Hospital>& found) const;

private:
    QBox bounds;
    int capacity;
    std::vector<Hospital> points;
    bool divided;
    std::unique_ptr<QuadTree> nw, ne, sw, se;
    void subdivide();
};

namespace QuadFilter {
    double haversine(double lat1, double lon1, double lat2, double lon2);
    std::vector<Hospital> filterByDistance(const QuadTree& tree, double userLat, double userLon, double radiusKm);
}
