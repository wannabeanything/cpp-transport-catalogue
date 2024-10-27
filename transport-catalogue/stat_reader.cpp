#include "stat_reader.h"
#include <iomanip>  // For std::fixed and std::setprecision
#include <iostream>
#include <string>

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    if (request.substr(0, 3) == "Bus") {
        request = request.substr(4);
        auto bus_info = transport_catalogue.GetBusInfo(std::string(request));

        if (bus_info) {
            output << "Bus " << request << ": "
                   << bus_info->stop_count << " stops on route, "
                   << bus_info->unique_stops << " unique stops, "
                   << std::fixed  << std::setprecision(0) << bus_info->route_length << " route length, "
                   << std::defaultfloat << std::setprecision(6) << bus_info->curvature << " curvature" << std::endl;
        } else {
            output << "Bus " << request << ": not found\n";
        }
    } else if (request.substr(0, 4) == "Stop") {
        request = request.substr(5);
        auto buses = transport_catalogue.GetBusesForStop(std::string(request));

        if (!buses) {
            output << "Stop " << request << ": not found\n";
        } else if (buses->empty()) {
            output << "Stop " << request << ": no buses\n";
        } else {
            output << "Stop " << request << ": buses";
            for (const auto& bus : *buses) {
                output << " " << bus;
            }
            output << "\n";
        }
    }
}

void ProcessStatRequests(TransportCatalogue& catalogue, std::istream& input, std::ostream& output) {
    int stat_request_count;
    input >> stat_request_count >> std::ws;  

    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        std::getline(input, line);  

        ParseAndPrintStat(catalogue, line, output);  
    }
}
