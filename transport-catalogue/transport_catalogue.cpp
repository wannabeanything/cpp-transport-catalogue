#include "transport_catalogue.h"
#include "geo.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <tuple>
#include <cassert>
#include <set>

void TransportCatalogue::AddStop(const std::string& name, double latitude, double longitude) {
    stops_.emplace_back(Stop{name, latitude, longitude});
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip) {
    buses_.emplace_back(Bus{name, {}, is_roundtrip});
    Bus& bus = buses_.back();
    for (const auto& stop_name : stop_names) {
        const Stop* stop = FindStop(stop_name);
        bus.stops.push_back(stop);
    }
    busname_to_bus_[buses_.back().name] = &buses_.back();
}
std::optional<std::vector<std::string>> TransportCatalogue::GetBusesForStop(const std::string& stop_name) const {
    const Stop* stop = FindStop(stop_name);
    if (!stop) {
        return std::nullopt; // Stop not found
    }

    std::vector<std::string> buses;
    for (const auto& [bus_name, bus] : busname_to_bus_) {
        if (std::find(bus->stops.begin(), bus->stops.end(), stop) != bus->stops.end()) {
            buses.push_back(std::string(bus_name));
        }
    }

    if (buses.empty()) {
        return std::vector<std::string>(); // Stop found but no buses pass through
    }

    std::sort(buses.begin(), buses.end()); // Sort the bus names alphabetically
    return buses;
}



const Bus* TransportCatalogue::FindBus(const std::string& name) const {
    auto it = busname_to_bus_.find(name);
    return it != busname_to_bus_.end() ? it->second : nullptr;
}

const Stop* TransportCatalogue::FindStop(const std::string& name) const {
    auto it = stopname_to_stop_.find(name);
    return it != stopname_to_stop_.end() ? it->second : nullptr;
}

std::optional<std::tuple<int, int, double>> TransportCatalogue::GetBusInfo(const std::string& bus_name) const {
    const Bus* bus = FindBus(bus_name);
    if (!bus) {
        
        return std::nullopt;
    }

    int stop_count = bus->stops.size();
    std::unordered_set<const Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
    int unique_stop_count = unique_stops.size();

    double route_length = 0.0;

    if (!bus->is_roundtrip) {
        for (size_t i = bus->stops.size() - 1; i > 0; --i) {
            route_length += ComputeDistance(
                {bus->stops[i]->latitude, bus->stops[i]->longitude}, 
                {bus->stops[i - 1]->latitude, bus->stops[i - 1]->longitude}
            );
        }
    }
    else{
        for (size_t i = 1; i < bus->stops.size(); ++i) {
            route_length += ComputeDistance(
                {bus->stops[i - 1]->latitude, bus->stops[i - 1]->longitude}, 
                {bus->stops[i]->latitude, bus->stops[i]->longitude}
            );
        }
    }

    return std::make_tuple(stop_count, unique_stop_count, route_length);
}