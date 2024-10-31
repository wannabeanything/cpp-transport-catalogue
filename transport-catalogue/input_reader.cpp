#include "input_reader.h"

#include <iostream>
#include <algorithm>
#include <cassert>
#include <iterator>


void InputReader::ProcessInput(std::istream& input, TransportCatalogue& catalogue){
    int base_request_count;
    input >> base_request_count >> std::ws;  

    for (int i = 0; i < base_request_count; ++i) {
        std::string line;
        std::getline(input, line);  
        ParseLine(line);
    }
    ApplyCommands(catalogue);
}


/**
 * Парсит строку вида "10.123,  -30.1837" и возвращает пару координат (широта, долгота)
 */
Coordinates ParseCoordinates(std::string_view str) {
    static const double nan = std::nan("");

    auto not_space = str.find_first_not_of(' ');
    auto comma = str.find(',');

    if (comma == str.npos) {
        return {nan, nan};
    }

    auto not_space2 = str.find_first_not_of(' ', comma + 1);

    double lat = std::stod(std::string(str.substr(not_space, comma - not_space)));
    double lng = std::stod(std::string(str.substr(not_space2)));

    return {lat, lng};
}

/**
 * Удаляет пробелы в начале и конце строки
 */
std::string_view Trim(std::string_view string) {
    const auto start = string.find_first_not_of(' ');
    if (start == string.npos) {
        return {};
    }
    return string.substr(start, string.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
std::vector<std::string_view> Split(std::string_view string, char delim) {
    std::vector<std::string_view> result;

    size_t pos = 0;
    while ((pos = string.find_first_not_of(' ', pos)) < string.length()) {
        auto delim_pos = string.find(delim, pos);
        if (delim_pos == string.npos) {
            delim_pos = string.size();
        }
        if (auto substr = Trim(string.substr(pos, delim_pos - pos)); !substr.empty()) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

/**
 * Парсит маршрут.
 * Для кольцевого маршрута (A>B>C>A) возвращает массив названий остановок [A,B,C,A]
 * Для некольцевого маршрута (A-B-C-D) возвращает массив названий остановок [A,B,C,D,C,B,A]
 */
std::vector<std::string_view> ParseRoute(std::string_view route) {
    if (route.find('>') != route.npos) {
        return Split(route, '>');
    }

    auto stops = Split(route, '-');
    std::vector<std::string_view> results(stops.begin(), stops.end());
    results.insert(results.end(), std::next(stops.rbegin()), stops.rend());

    return results;
}

CommandDescription ParseCommandDescription(std::string_view line) {
    auto colon_pos = line.find(':');
    if (colon_pos == line.npos) {
        return {};
    }

    auto space_pos = line.find(' ');
    if (space_pos >= colon_pos) {
        return {};
    }

    auto not_space = line.find_first_not_of(' ', space_pos);
    if (not_space >= colon_pos) {
        return {};
    }

    return {std::string(line.substr(0, space_pos)),
            std::string(line.substr(not_space, colon_pos - not_space)),
            std::string(line.substr(colon_pos + 1))};
}

void InputReader::ParseLine(std::string_view line) {
    auto command_description = ParseCommandDescription(line);
    if (command_description) {
        commands_.push_back(std::move(command_description));
    }
}
std::unordered_map<std::string, int> ParseStopDistances(std::string_view description) {
    std::unordered_map<std::string, int> distances;

    // Find the position of the first and second commas
    auto first_comma = description.find(',');
    if (first_comma == std::string_view::npos) {
        return distances;  // Return an empty map if no distances are found
    }
    
    auto second_comma = description.find(',', first_comma + 1);
    if (second_comma == std::string_view::npos) {
        return distances;  // Return an empty map if no distances are found
    }

    // Get the part of the description after the second comma
    std::string_view distance_str = description.substr(second_comma + 1);

    // Split by comma to get individual distances
    auto distance_tokens = Split(distance_str, ',');
    for (const auto& token : distance_tokens) {
        auto to_pos = token.find("m to ");
        if (to_pos != std::string_view::npos) {
            int distance = std::stoi(std::string(token.substr(0, to_pos)));  // Parse the distance
            std::string_view stop_name = Trim(token.substr(to_pos + 5));  // Get the stop name after "m to "
            distances[std::string(stop_name)] = distance;  // Store the distance in the map
        }
    }

    return distances;
}


void InputReader::ApplyCommands(TransportCatalogue& catalogue) const {
    // First pass: Adding stops with coordinates
    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            auto coords = ParseCoordinates(command.description);
            catalogue.AddStop(command.id, coords.lat, coords.lng);
        }
    }

    // Second pass: Adding distances between stops
    for (const auto& command : commands_) {
        if (command.command == "Stop") {
            auto distances = ParseStopDistances(command.description);
            for (const auto& [to_stop_name, distance] : distances) {
                catalogue.AddDistance(command.id, to_stop_name, distance);
            }
        }
    }

    // Finalizing distances after adding all stops
    catalogue.SetDistance();

    // Adding buses
    for (const auto& command : commands_) {
        if (command.command == "Bus") {
            bool is_roundtrip = command.description.find('>') != std::string::npos;
            auto stops_view = ParseRoute(command.description);

            std::vector<std::string> stops(stops_view.begin(), stops_view.end());
            catalogue.AddBus(command.id, stops, is_roundtrip);
        }
    }
}
