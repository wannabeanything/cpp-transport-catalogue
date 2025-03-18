#pragma once
#include "domain.h"

#include <set>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <optional>


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
    void SetVelocityAndWaitTime(double velocity, double wait_time);
    size_t GetStopsCount() const;
    
    double GetWaitTime()const;
    void AddDistance(const std::string& from_stop_name, const std::string& to_stop_name, int distance);
    void SetDistance();
    const Bus* FindBus(const std::string& name) const;
    const Stop* FindStop(const std::string& name) const;
    std::optional<double> GetDistance(const Stop* from_stop, const Stop* to_stop) const;
    std::optional<BusInfo> GetBusInfo(const std::string& bus_name) const;
    const std::set<std::string_view>* GetBusesForStop(const std::string& stop_name) const;
    const std::deque<Bus> GetSortedRoutes() const;
    const std::deque<Bus>& GetRoutes() const{
        return buses_;
    }
    const std::deque<Stop>& GetStops()const{
        return stops_;
    }
    const std::vector<const Stop*> GetSortedStops() const;
    bool StopIsUseless(const std::string& name)const;
    
    
private:
    std::deque<Stop> stops_;
    std::deque<Bus> buses_;

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;

    std::unordered_map<std::string_view, std::set<std::string_view>> stop_to_buses_;

    std::unordered_map<std::string, std::unordered_set<std::string_view>> bus_to_stops_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, double, PairHash> distance_map_;
    std::vector<std::tuple<std::string, std::string, int>> temp_distances_;
};