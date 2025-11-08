#include "quadtree_geofilter.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// QBox is the bounding box, contains returns true if the point is inside the box.
bool QBox::contains(double lat, double lon) const {
    return lat >= minLat && lat <= maxLat && lon >= minLon && lon <= maxLon;
}

// Returns true if Qbox intersects with the given box.
bool QBox::intersects(const QBox& other) const {
    if (other.minLon > maxLon || other.maxLon < minLon) return false;
    if (other.minLat > maxLat || other.maxLat < minLat) return false;
    return true;
}

// Initializes the quadtree with the Qbox and a capacity
QuadTree::QuadTree(const QBox& b, int cap)
    : bounds(b), capacity(cap), divided(false) {}

//Subdivides the current node into quadrants
void QuadTree::subdivide() {
    double midLat = (bounds.minLat + bounds.maxLat) * 0.5;
    double midLon = (bounds.minLon + bounds.maxLon) * 0.5;

    // Creates new nodes for the four "intercardinal" directions
    nw = std::make_unique<QuadTree>(QBox{midLat, bounds.maxLat, bounds.minLon, midLon}, capacity);
    ne = std::make_unique<QuadTree>(QBox{midLat, bounds.maxLat, midLon, bounds.maxLon}, capacity);
    sw = std::make_unique<QuadTree>(QBox{bounds.minLat, midLat, bounds.minLon, midLon}, capacity);
    se = std::make_unique<QuadTree>(QBox{bounds.minLat, midLat, midLon, bounds.maxLon}, capacity);

    divided = true;
}

//Places a hospital into the tree
bool QuadTree::insert(const Hospital& h) {
    if (!bounds.contains(h.latitude, h.longitude)) return false;
    if (points.size() < capacity) {
        points.push_back(h);
        return true;
    }
    if (!divided) subdivide();
    return nw->insert(h) || ne->insert(h) || sw->insert(h) || se->insert(h);
}

// Queries every hospital in the Qbox
void QuadTree::queryBox(const QBox& range, std::vector<Hospital>& found) const {
    if (!bounds.intersects(range)) return;
    for (const auto& h : points) {
        if (range.contains(h.latitude, h.longitude)) found.push_back(h);
    }
    if (!divided) return;
    nw->queryBox(range, found);
    ne->queryBox(range, found);
    sw->queryBox(range, found);
    se->queryBox(range, found);
}

namespace QuadFilter {

static constexpr double EARTH_RADIUS_KM = 6371.0;

double haversine(double lat1, double lon1, double lat2, double lon2) {
    double toRad = M_PI / 180.0;
    lat1 *= toRad;
    lat2 *= toRad;
    double dLat = lat2 - lat1;
    double dLon = (lon2 - lon1) * toRad;
    double a = sin(dLat*0.5)*sin(dLat*0.5) +
               cos(lat1)*cos(lat2)*sin(dLon*0.5)*sin(dLon*0.5);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    return EARTH_RADIUS_KM * c;
}

// Filters the hospitals within the tree by distance and returns a list of the hospitals in the radius
std::vector<Hospital> filterByDistance(
    const QuadTree& tree,
    double userLat,
    double userLon,
    double radiusKm
) {
    // Deltas for the Qbox search
    double latDelta = radiusKm / 111.0;
    double lonDelta = radiusKm / (111.0 * std::cos(userLat * M_PI / 180.0));
    // Defines the Qbox for the user location
    QBox q{
        userLat - latDelta,
        userLat + latDelta,
        userLon - lonDelta,
        userLon + lonDelta
    };

    //Queries the tree for possible hospitals within the qbox
    std::vector<Hospital> candidates;
    tree.queryBox(q, candidates);
    //Filters those candidates using the haversine
    std::vector<Hospital> result;
    result.reserve(candidates.size());

    for (const auto& h : candidates) {
        if (h.latitude == 0 && h.longitude == 0) continue;
        double d = haversine(userLat, userLon, h.latitude, h.longitude);
        if (d <= radiusKm) result.push_back(h);
    }

    return result;
}

}
