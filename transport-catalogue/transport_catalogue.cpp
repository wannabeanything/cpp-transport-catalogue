#include "transport_catalogue.h"
#include "geo.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <tuple>
#include <cassert>
#include <set>
#include <iostream>
void TransportCatalogue::AddStop(const std::string& name, double latitude, double longitude) {
    stops_.emplace_back(Stop{name, latitude, longitude});
    stopname_to_stop_[stops_.back().name] = &stops_.back();
}

void TransportCatalogue::AddDistance(const std::string& from_stop_name, const std::string& to_stop_name, int distance) {
    temp_distances_.emplace_back(from_stop_name, to_stop_name, distance);
}
void TransportCatalogue::SetDistance() {
    for (const auto& [from_name, to_name, distance] : temp_distances_) {
        const Stop* from_stop = FindStop(from_name);
        const Stop* to_stop = FindStop(to_name);

        if (from_stop && to_stop) {
            distance_map_[{from_stop, to_stop}] = distance;

            if (distance_map_.find({to_stop, from_stop}) == distance_map_.end()) {
                distance_map_[{to_stop, from_stop}] = distance;
            }
        }
    }
    temp_distances_.clear();
}



void TransportCatalogue::AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip) {
    Bus bus{name, {}, is_roundtrip};

    for (const auto& stop_name : stop_names) {
        const Stop* stop = FindStop(stop_name);
        if (stop) {
            bus.stops.push_back(stop);
            bus_to_stops_[name].insert(stop->name);
            stop_to_buses_[stop->name].insert(name);  
        }
    }

    buses_.push_back(bus);
    busname_to_bus_[name] = &buses_.back();
}

const std::set<std::string_view>* TransportCatalogue::GetBusesForStop(const std::string& stop_name) const {
    auto it = stop_to_buses_.find(stop_name);
    return it != stop_to_buses_.end() ? &it->second : nullptr;
}





const Bus* TransportCatalogue::FindBus(const std::string& name) const {
    auto it = busname_to_bus_.find(name);
    return it != busname_to_bus_.end() ? it->second : nullptr;
}

const Stop* TransportCatalogue::FindStop(const std::string& name) const {
    auto it = stopname_to_stop_.find(name);
    return it != stopname_to_stop_.end() ? it->second : nullptr;
}


std::optional<double> TransportCatalogue::GetDistance(const Stop* from_stop, const Stop* to_stop) const {
    if (!from_stop || !to_stop) {
        return std::nullopt; 
    }
    auto it = distance_map_.find({from_stop, to_stop});
    if (it != distance_map_.end()) {
        return it->second; 
    }
    return std::nullopt; 
}


std::optional<BusInfo> TransportCatalogue::GetBusInfo(const std::string& bus_name) const {
    const Bus* bus = FindBus(bus_name);
    if (!bus) {
        return std::nullopt;  
    }

    int stop_count = bus->stops.size();
    std::unordered_set<const Stop*> unique_stops(bus->stops.begin(), bus->stops.end());
    int unique_stop_count = unique_stops.size();

    double route_length = 0.0;
    double geo_distance = 0.0;

    for (size_t i = 0; i < bus->stops.size(); ++i) {
        const Stop* from_stop = bus->stops[i];
        if (i + 1 < bus->stops.size()) {
            const Stop* to_stop = bus->stops[i + 1];

            
            auto road_distance = GetDistance(from_stop, to_stop);
            if (road_distance) {
                route_length += *road_distance; 

            }
            geo_distance += ComputeDistance(
                {from_stop->latitude, from_stop->longitude},
                {to_stop->latitude, to_stop->longitude}
            );
        }
    }

    double curvature = (geo_distance > 0) ? (route_length / geo_distance) : std::nan("");


    return BusInfo{
        stop_count,        
        unique_stop_count, 
        route_length,      
        curvature
    };
}

