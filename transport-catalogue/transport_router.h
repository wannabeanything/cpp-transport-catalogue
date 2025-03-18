#pragma once

#include "transport_catalogue.h"
#include "router.h"
#include "graph.h"
#include <map>



class TransportRouter{
public:
    TransportRouter(const TransportCatalogue& catalogue);
    const graph::Router<double>* GetRouter()const{
        return router_;
    }
    const graph::DirectedWeightedGraph<double>& GetGraph() const;

    const std::vector<std::string>& GetStopsNumber()const;
private:
void BuildGraph(const TransportCatalogue& catalogue);
    std::map<std::string, graph::VertexId> stop_ids_;
    std::vector<std::string> stop_names_;
    graph::Router<double>* router_ = nullptr;
    graph::DirectedWeightedGraph<double> graph_;
    
};