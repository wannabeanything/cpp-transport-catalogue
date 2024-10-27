#pragma once
#include <set>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <optional>
struct Stop {
    std::string name;
    double latitude;
    double longitude;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    bool is_roundtrip;  // true для кольцевого маршрута, false для обычного
};

struct BusInfo {
    int stop_count;       
    int unique_stops;     
    double route_length;
    double curvature;
};

struct PairHash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2>& pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        return hash1 ^ hash2; 
    }
};
class TransportCatalogue {
public:
    void AddStop(const std::string& name, double latitude, double longitude);
    void AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip);
    void SetDistance(const std::string& from_stop_name, const std::string& to_stop_name, double distance);
    void AddDistance(const std::string& stop_name, const std::unordered_map<std::string, double>& distances);
    const Bus* FindBus(const std::string& name) const;
    const Stop* FindStop(const std::string& name) const;
    std::optional<double> GetDistance(const Stop* from_stop, const Stop* to_stop) const;
    std::optional<BusInfo> GetBusInfo(const std::string& bus_name) const;
    std::optional<std::set<std::string_view>> GetBusesForStop(const std::string& stop_name) const;

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

    std::unordered_map<std::string_view, std::unordered_set<std::string_view>> bus_to_stops_map_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, PairHash> distance_map_;
};
