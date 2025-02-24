#pragma once

#include <string>
#include <vector>

struct Stop {
    std::string name;
    double latitude;
    double longitude;
};

struct Bus {
    std::string name;
    std::vector<const Stop*> stops;
    bool is_roundtrip;
    double velocity = .0;
    double wait_time = .0;
};

struct BusInfo {
    int stop_count;       
    int unique_stops;     
    double route_length;
    double curvature;
};

