#include "json_reader.h"  
#include <iostream>
#include <stdio.h>


int main() {

    //freopen("text.txt","r",stdin);
    /*
    json::Document input_json = json::Load(std::cin);
    TransportCatalogue catalogue;
    BaseRequestsHandler fill_catalogue(catalogue);
    fill_catalogue.Parse(input_json.GetRoot().AsMap().at("base_requests"));
    fill_catalogue.Process();
    MapRenderer mr(catalogue, input_json.GetRoot().AsMap().at("render_settings"));
    mr.DrawLines();
    mr.NameRoutes();
    mr.CircleStops();
    mr.NameStops();
    mr.DrawMap();
    */
    json::Document input_json = json::Load(std::cin);
    TransportCatalogue catalogue;
    RequestManager manager(catalogue);
    manager.ProcessInput(input_json.GetRoot());
    json::Node responses = manager.GetResponses();
    json::Document doc(responses);
    json::Print(doc, std::cout);
}