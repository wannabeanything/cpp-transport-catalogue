#include "stat_reader.h"
#include <iomanip>
#include <iostream>

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request, std::ostream& output) {
    if (request.substr(0, 3) == "Bus") {
        request = request.substr(4);
        auto bus_info = transport_catalogue.GetBusInfo(std::string(request));

        if (bus_info) {
            int stops_on_route, unique_stops;
            double route_length;
            std::tie(stops_on_route, unique_stops, route_length) = *bus_info;

            output << "Bus " << request << ": "
                   << stops_on_route << " stops on route, "
                   << unique_stops << " unique stops, "
                   << route_length << " route length\n";
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