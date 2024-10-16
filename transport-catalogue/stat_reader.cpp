#include "stat_reader.h"
#include <iomanip>
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
                   << bus_info->route_length << " route length\n";
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


void ProcessStatRequests(TransportCatalogue& catalogue) {
    int stat_request_count;
    std::cin >> stat_request_count >> std::ws;  

    for (int i = 0; i < stat_request_count; ++i) {
        std::string line;
        std::getline(std::cin, line);  

        ParseAndPrintStat(catalogue, line, std::cout);  
    }
}