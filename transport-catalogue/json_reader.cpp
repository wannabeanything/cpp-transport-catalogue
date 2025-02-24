#include "json_reader.h"
#include "log_duration.h"
#include <sstream>
using namespace std::literals;

void BaseRequestsHandler::Parse(const json::Node& base_requests) {
    for (const auto& request : base_requests.AsArray()) {
        const auto& request_map = request.AsMap();
        const std::string& type = request_map.at("type").AsString();

        if (type == "Stop") {
            stop_requests_.push_back(request);
        } else if (type == "Bus") {
            bus_requests_.push_back(request);
        }
    }
}

void BaseRequestsHandler::ParseStop(const json::Node& stop_request) {
    const auto& stop_map = stop_request.AsMap();
    const std::string& name = stop_map.at("name").AsString();
    double latitude = stop_map.at("latitude").AsDouble();
    double longitude = stop_map.at("longitude").AsDouble();

    catalogue_.AddStop(name, latitude, longitude);

    if (stop_map.count("road_distances") > 0) {
        for (const auto& [to_stop, distance_node] : stop_map.at("road_distances").AsMap()) {
            catalogue_.AddDistance(name, to_stop, distance_node.AsInt());
        }
    }
}

void BaseRequestsHandler::ParseBus(const json::Node& bus_request) {
    const auto& bus_map = bus_request.AsMap();
    const std::string& name = bus_map.at("name").AsString();
    bool is_roundtrip = bus_map.at("is_roundtrip").AsBool();
    std::vector<std::string> stop_names;

    for (const auto& stop_node : bus_map.at("stops").AsArray()) {
        stop_names.push_back(stop_node.AsString());
    }

    catalogue_.AddBus(name, stop_names, is_roundtrip);
}

void BaseRequestsHandler::Process() {
    for (const auto& stop_request : stop_requests_) {
        ParseStop(stop_request);
    }
    catalogue_.SetDistance();
    for (const auto& bus_request : bus_requests_) {
        ParseBus(bus_request);
    }
}

void StatRequestsHandler::Parse(const json::Node& stat_requests) {
    if (stat_requests.IsArray()) {
        for (const auto& request : stat_requests.AsArray()) {
            parsed_requests_.push_back(request);
        }
    }
}
void StatRequestsHandler::SetRoutingSettings(const json::Node& routing_settings){
    double wait_time = routing_settings.AsMap().at("bus_wait_time").AsDouble();
    double velocity = routing_settings.AsMap().at("bus_velocity").AsDouble();
    catalogue_.SetVelocityAndWaitTime(velocity, wait_time);
}

void StatRequestsHandler::InitializeMap(const json::Node& render_settings){
    map_.FillMapRenderer(render_settings);
}


json::Node StatRequestsHandler::ProcessBusRequest(int request_id, const std::string& bus_name) const{
    auto bus_info = catalogue_.GetBusInfo(bus_name);
    if (bus_info) {
        /*
        return json::Node{
            std::map<std::string, json::Node>{
                {"request_id", request_id},
                {"stop_count", bus_info->stop_count},
                {"unique_stop_count", bus_info->unique_stops},
                {"route_length", bus_info->route_length},
                {"curvature", bus_info->curvature}
            }
        };
        */
       return json::Builder{}.StartDict()
       .Key("request_id").Value(request_id)
       .Key("stop_count").Value(bus_info->stop_count)
       .Key("unique_stop_count").Value(bus_info->unique_stops)
       .Key("route_length").Value(bus_info->route_length)
       .Key("curvature").Value(bus_info->curvature)
       .EndDict().Build();
    } else {
        /*
        return json::Node{
            std::map<std::string, json::Node>{
            {"request_id", request_id},
            {"error_message", "not found"s}}
        };*/
        return json::Builder{}.StartDict()
        .Key("request_id").Value(request_id)
        .Key("error_message").Value("not found")
        .EndDict().Build();
    }
}

json::Node StatRequestsHandler::ProcessStopRequest(int request_id, const std::string& stop_name) const {
    const auto* buses_for_stop = catalogue_.GetBusesForStop(stop_name);
    if (buses_for_stop) {
        /*
        json::Array ar;
        for (const auto bus_name : *buses_for_stop){
            ar.push_back(std::string(bus_name));
        }*/
        /*
        return json::Node{
            std::map<std::string, json::Node>{
                {"buses", ar},
                {"request_id", request_id}
            }
        };*/
        auto builder = json::Builder{};
        builder.StartDict()
        .Key("buses").StartArray();
        for(const auto bus_name: *buses_for_stop){
            builder.Value(std::string(bus_name));
        }
        return builder.EndArray()
        .Key("request_id").Value(request_id)
        .EndDict().Build();
        /*
        return json::Builder{}.StartDict()
        .Key("buses").Value(ar)
        .Key("request_id").Value(request_id)
        .EndDict().Build();
        */
    }
    else if(catalogue_.FindStop(stop_name)!=nullptr){
        /*
        return json::Node{
            std::map<std::string, json::Node>{
                {"buses", json::Node(json::Array())},
                {"request_id", request_id}
            }
        };*/
        return json::Builder{}.StartDict()
        .Key("buses").StartArray().EndArray()
        .Key("request_id").Value(request_id)
        .EndDict().Build();

    }
    /*
    return json::Node{
        std::map<std::string, json::Node>{
            {"request_id", request_id},
            {"error_message", "not found"s}
        }
    };*/
    return json::Builder{}.StartDict()
    .Key("request_id").Value(request_id)
    .Key("error_message").Value("not found")
    .EndDict().Build();
}


json::Node StatRequestsHandler::ProcessRouteRequest(int request_id, const std::string& from, const std::string& to)const{
    if(catalogue_.StopIsUseless(from) || catalogue_.StopIsUseless(to)){
        return json::Builder{}.StartDict()
        .Key("request_id").Value(request_id)
        .Key("error_message").Value("not found")
        .EndDict().Build();
    }
    
    if(from == to){
        return json::Builder{}.StartDict()
        .Key("request_id").Value(request_id)
        .Key("total_time").Value(0)
        .Key("items").StartArray().EndArray()
        .EndDict()
        .Build();
    }
    graph::VertexId fromId, toId;
    const graph::DirectedWeightedGraph<double>& const_graph = ts_router_.GetGraph();
    const auto& stops_number = ts_router_.GetStopsNumber();

    /*
    graph::VertexId departureId = std::distance(stop_names_.begin(),
                        std::find(stop_names_.begin(), stop_names_.end(), stop_from->name));
    
    */
    fromId = std::distance(stops_number.begin(),
            std::find(stops_number.begin(), stops_number.end(), from));
    toId = std::distance(stops_number.begin(),
            std::find(stops_number.begin(), stops_number.end(), to));        
            
    //fromId = stops_number.at(from);
    //toId = stops_number.at(to);

    const auto& route = router_->BuildRoute(fromId, toId);

    
    if(route.has_value()){
        double total_time = 0.0;
        
        
        auto builder = json::Builder{};
        builder.StartDict()
        .Key("items").StartArray();
        
        for(const graph::EdgeId edgeId: route.value().edges){
            
            graph::Edge edge = const_graph.GetEdge(edgeId);
            if(edge.span_count != 0){
                
                builder.StartDict()
                .Key("bus").Value(edge.name)
                .Key("span_count").Value(static_cast<int>(edge.span_count))
                .Key("time").Value(edge.weight)
                .Key("type").Value("Bus")
                .EndDict();
                
            }
            else{
                
                builder.StartDict()
                .Key("stop_name").Value(edge.name)
                .Key("time").Value(edge.weight)
                .Key("type").Value("Wait")
                .EndDict();
                
            }
            
            total_time+= edge.weight;
            
        }
        
        
        return builder.EndArray()
        .Key("request_id").Value(request_id)
        .Key("total_time").Value(total_time)
        .EndDict()
        .Build();
        
    }
    

    return json::Builder{}.StartDict()
        .Key("request_id").Value(request_id)
        .Key("error_message").Value("not found")
        .EndDict().Build();
}


json::Node StatRequestsHandler::ProcessMapRequest(int request_id){
    std::ostringstream oss;
    map_.DrawMap(oss);
    /*
    return json::Node{
        std::map<std::string, json::Node>{
            {"map", oss.str()},
            {"request_id", request_id}
        }
    };*/
    
    return json::Builder{}.StartDict()
    .Key("map").Value(oss.str())
    .Key("request_id").Value(request_id)
    .EndDict().Build();
}

void StatRequestsHandler::BuildGraph(){
    
    ts_router_.BuildGraph(&catalogue_);
    router_ = new graph::Router<double>(ts_router_.GetGraph());
}

std::vector<json::Node> StatRequestsHandler::Process(){
    std::vector<json::Node> responses;

    for (const auto& request : parsed_requests_) {
        if (request.IsMap()) {
            const auto& request_map = request.AsMap();
            int request_id = request_map.at("id").AsInt();
            const std::string& type = request_map.at("type").AsString();

            if (type == "Bus") {
                responses.push_back(ProcessBusRequest(request_id, request_map.at("name").AsString()));
                
            } else if (type == "Stop") {
                responses.push_back(ProcessStopRequest(request_id, request_map.at("name").AsString()));
                
            }
            else if(type == "Map"){
                responses.push_back(ProcessMapRequest(request_id));
                
            }
            else if(type == "Route"){
                responses.push_back(ProcessRouteRequest(request_id, request_map.at("from").AsString(), request_map.at("to").AsString()));
                
            }
        }
    }

    return responses;
}




void RequestManager::ProcessInput(const json::Node& input) {
    const auto& input_map = input.AsMap();

    
    base_requests_handler_.Parse(input_map.at("base_requests"));
    base_requests_handler_.Process();  
    
    
    stat_requests_handler_.InitializeMap(input_map.at("render_settings"));
    stat_requests_handler_.SetRoutingSettings(input_map.at("routing_settings"));
    stat_requests_handler_.BuildGraph();
    stat_requests_handler_.Parse(input_map.at("stat_requests"));
}

json::Node RequestManager::GetResponses() {
    json::Array responses; 
    for (const auto& response : stat_requests_handler_.Process()) {
        responses.emplace_back(response);
    }
    

    return json::Node(responses);
}



