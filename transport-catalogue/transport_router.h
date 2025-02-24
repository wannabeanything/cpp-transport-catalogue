#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include <map>



class TransportRouter{
public:
    TransportRouter() = default;
    //TransportRouter(const TransportCatalogue& catalogue, const std::string& from, const std::string& to);
    void BuildGraph(const TransportCatalogue* catalogue);
    const graph::DirectedWeightedGraph<double>& GetGraph() const;
    //graph::VertexId GetIdFrom()const;
    //graph::VertexId GetIdTo()const;
    //const std::map<std::string, graph::VertexId>& GetStopsNumber()const;
    const std::vector<std::string>& GetStopsNumber()const;
private:
    const TransportCatalogue* catalogue_ = nullptr;
    //graph::VertexId from_;
    //graph::VertexId to_;
    std::map<std::string, graph::VertexId> stop_ids_;
    std::vector<std::string> stop_names_;

    graph::DirectedWeightedGraph<double> graph_;
    
};