#pragma once

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
};

class TransportCatalogue {
public:
    void AddStop(const std::string& name, double latitude, double longitude);
    void AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip);

    const Bus* FindBus(const std::string& name) const;
    const Stop* FindStop(const std::string& name) const;

    std::optional<BusInfo> GetBusInfo(const std::string& bus_name) const;
    std::optional<std::unordered_set<std::string_view>> GetBusesForStop(const std::string& stop_name) const;

private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

    
    std::unordered_map<std::string, std::unordered_set<std::string_view>> bus_to_stops_;
};
