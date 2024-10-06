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

class TransportCatalogue {
public:
    void AddStop(const std::string& name, double latitude, double longitude);
    void AddBus(const std::string& name, const std::vector<std::string>& stop_names, bool is_roundtrip);

    const Bus* FindBus(const std::string& name) const;
    const Stop* FindStop(const std::string& name) const;

    // Метод для получения информации о маршруте
    std::optional<std::tuple<int, int, double>> GetBusInfo(const std::string& bus_name) const;
    std::optional<std::vector<std::string>> GetBusesForStop(const std::string& stop_name) const;

private:
    std::deque<Stop> stops_;  // Дек для хранения остановок
    std::deque<Bus> buses_;   // Дек для хранения маршрутов

    std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;  // Индекс остановок по названию
    std::unordered_map<std::string_view, const Bus*> busname_to_bus_;     // Индекс маршрутов по названию
};