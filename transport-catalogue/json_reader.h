#pragma once
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "json_builder.h"
#include <vector>
#include <string>
#include <unordered_map>


class BaseRequestsHandler {
public:
    explicit BaseRequestsHandler(TransportCatalogue& catalogue)
        : catalogue_(catalogue) {}

    void Parse(const json::Node& base_requests);
    void Process();

private:
    TransportCatalogue& catalogue_;
    std::vector<json::Node> stop_requests_;
    std::vector<json::Node> bus_requests_;
    void ParseStop(const json::Node& stop_request);
    void ParseBus(const json::Node& bus_request);
};




class StatRequestsHandler {
public:
    explicit StatRequestsHandler(TransportCatalogue& catalogue)
        : catalogue_(catalogue), map_(catalogue){}

    void Parse(const json::Node& stat_requests);
    void InitializeMap(const json::Node& render_settings);
    std::vector<json::Node> Process();
    
private:
    TransportCatalogue& catalogue_;
    std::vector<json::Node> parsed_requests_;
    MapRenderer map_;
    json::Node ProcessBusRequest(int request_id, const std::string& bus_name)const;
    json::Node ProcessStopRequest(int request_id, const std::string& stop_name)const;
    json::Node ProcessMapRequest(int request_id);
};


class RequestManager {
public:
    RequestManager(TransportCatalogue& catalogue)
        : base_requests_handler_(catalogue), stat_requests_handler_(catalogue){}

    void ProcessInput(const json::Node& input);
    json::Node GetResponses();

private:
    BaseRequestsHandler base_requests_handler_;
    StatRequestsHandler stat_requests_handler_;
};
