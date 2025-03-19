#include "transport_router.h"
#include "json_builder.h"
#include <iostream>
#include <algorithm>
TransportRouter::TransportRouter(const TransportCatalogue& catalogue)
{
    BuildGraph(catalogue);
}
void TransportRouter::BuildGraph(const TransportCatalogue &catalogue)
{
    
    
    const auto& all_stops = catalogue.GetStops();
    const std::deque<Bus> all_buses = catalogue.GetSortedRoutes();
    
    size_t vertexId = 0;
    graph::DirectedWeightedGraph<double> temp_graph(catalogue.GetStopsCount() * 2);
    stop_names_.resize(all_stops.size() * 2);
    for(const auto& stop: all_stops){
        
        
        stop_names_[vertexId] = stop.name;   
        temp_graph.AddEdge({stop.name,
                            0,
                            vertexId,
                            ++vertexId,
                            catalogue.GetWaitTime()});
                         
        ++vertexId;
    }
    
    std::for_each(
        all_buses.begin(),
        all_buses.end(),
        [&temp_graph,&catalogue, this](const auto& item)
        {

            const auto& bus_ptr = item;
            const std::vector<const Stop*>& stops = bus_ptr.stops;
            size_t stop_count = stops.size();

            for (size_t i = 0; i < stop_count; ++i)
            {   double dist_sum = 0.0;
                for (size_t j = i+1; j < stop_count; ++j)
                {
                    const Stop* stop_from = stops[i];
                    const Stop* stop_to = stops[j];
                    
                   std::optional<double> distance = catalogue.GetDistance(stops[j-1], stop_to);
                   
                   if(distance.has_value()){
                        graph::VertexId departureId = std::distance(stop_names_.begin(),
                        std::find(stop_names_.begin(), stop_names_.end(), stop_from->name));
                        graph::VertexId arrivingId = std::distance(stop_names_.begin(),
                        std::find(stop_names_.begin(), stop_names_.end(), stop_to->name));
                        dist_sum += distance.value();
                        temp_graph.AddEdge({bus_ptr.name,
                        j-i,
                        departureId+1,
                        arrivingId,
                        dist_sum / bus_ptr.velocity});
                        
                   }
                   
                   
                    
                    if(!bus_ptr.is_roundtrip && stop_to == bus_ptr.stops.back() && j == stop_count/2)break;                    
                }
                
            }
            
        }
    );
    

    
    graph_ = std::move(temp_graph);
    router_ = new graph::Router(graph_);
    
}

std::optional<json::Node> TransportRouter::BuildRoute(const std::string& from, const std::string to, int request_id)const{
    
    graph::VertexId fromId, toId;
    fromId = std::distance(stop_names_.begin(),
            std::find(stop_names_.begin(), stop_names_.end(), from));
    toId = std::distance(stop_names_.begin(),
            std::find(stop_names_.begin(), stop_names_.end(), to));
    const auto& route = router_->BuildRoute(fromId, toId);
    
    if(route.has_value()){
        double total_time = 0.0;
        
        
        auto builder = json::Builder{};
        builder.StartDict()
        .Key("items").StartArray();
        
        for(const graph::EdgeId edgeId: route.value().edges){
            
            graph::Edge edge = graph_.GetEdge(edgeId);
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
    return std::nullopt;
}
const graph::DirectedWeightedGraph<double> &TransportRouter::GetGraph() const
{
    return graph_;
}



const std::vector<std::string>& TransportRouter::GetStopsNumber()const{
    return stop_names_;
}




