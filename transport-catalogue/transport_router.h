#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "json.h"
#include "graph.h"
#include <map>



class TransportRouter{
public:
    using builder = std::optional<json::Node>;
    TransportRouter(const TransportCatalogue& catalogue);
    
    const graph::DirectedWeightedGraph<double>& GetGraph() const;
    builder BuildRoute(const std::string& from, const std::string to, int request_id)const;
    
private:
    void BuildGraph(const TransportCatalogue& catalogue);
    const std::vector<std::string>& GetStopsNumber()const;
    
    std::map<std::string, graph::VertexId> stop_ids_;
    std::vector<std::string> stop_names_;
    graph::Router<double>* router_ = nullptr;
    graph::DirectedWeightedGraph<double> graph_;
    
};