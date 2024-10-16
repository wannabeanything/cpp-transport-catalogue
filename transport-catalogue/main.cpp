#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
    TransportCatalogue catalogue;

    InputReader reader;
    reader.ProcessInput(catalogue);
    ProcessStatRequests(catalogue);
}