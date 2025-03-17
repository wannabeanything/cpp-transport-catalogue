#include "transport_router.h"
/*
TransportRouter::TransportRouter(const TransportCatalogue& catalogue, const std::string& from, const std::string& to):catalogue_(catalogue){
    TransportRouter::BuildGraph(from, to);
}*/
#include <iostream>
#include <algorithm>
TransportRouter::TransportRouter(const TransportCatalogue& catalogue){
    BuildGraph(catalogue);
}
void TransportRouter::BuildGraph(const TransportCatalogue &catalogue)
{
    
    const auto& all_stops = catalogue.GetStops();
    const std::deque<Bus> all_buses = catalogue.GetSortedRoutes();
    //std::map<std::string, graph::VertexId> stop_ids;
    size_t vertexId = 0;
    graph::DirectedWeightedGraph<double> temp_graph(catalogue.GetStopsCount() * 2);
    stop_names_.resize(all_stops.size() * 2);
    for(const auto& stop: all_stops){
        
        //stop_ids[stop->name] = vertexId;
        stop_names_[vertexId] = stop.name;   
        temp_graph.AddEdge({stop.name,
                            0,
                            vertexId,
                            ++vertexId,
                            catalogue.GetWaitTime()});
                         
        ++vertexId;
    }
    //stop_ids_ = std::move(stop_ids);
    
    


    
    std::for_each(
        all_buses.begin(),
        all_buses.end(),
        [&temp_graph,catalogue, this](const auto& item)
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
}
const graph::DirectedWeightedGraph<double> &TransportRouter::GetGraph() const
{
    
    
    
    
    return graph_;
}



const std::vector<std::string>& TransportRouter::GetStopsNumber()const{
    return stop_names_;
}



/*
const std::map<std::string, graph::VertexId>& TransportRouter::GetStopsNumber() const
{
    
    return stop_ids_;
}*/
/*
graph::VertexId TransportRouter::GetIdFrom()const{
    return from_;
}
graph::VertexId TransportRouter::GetIdTo()const{
    return to_;
}*/
