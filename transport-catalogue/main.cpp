#include "json_reader.h"  
#include <iostream>
#include <stdio.h>

#include "log_duration.h"
int main() {
    LOG_DURATION("CODE");
    freopen("text.txt","r",stdin);
    
    
    json::Document input_json = json::Load(std::cin);
    TransportCatalogue catalogue;
    RequestManager manager(catalogue);
    manager.ProcessInput(input_json.GetRoot());
    json::Node responses = manager.GetResponses();
    json::Document doc(responses);
    json::Print(doc, std::cout);
}