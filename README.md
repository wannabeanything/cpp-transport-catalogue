# Transport Catalogue

A transport directory written in C++17. It reads stops and bus routes from a JSON document, answers queries about them, finds the fastest route between two stops and renders the route map as SVG.

## Features

- Own JSON library: parser, printer and a builder (`json.*`, `json_builder.*`).
- Base requests: add stops with coordinates and distances, add bus routes (circular and linear).
- Stat requests: bus statistics (stops, unique stops, route length), stops with the buses that go through them, route search between two stops, map rendering.
- Routing on a weighted graph (`graph.h`, `router.h`, `transport_router.*`) with configurable bus wait time and velocity.
- SVG map renderer with configurable colors, sizes and labels (`svg.*`, `map_renderer.*`).
- `LOG_DURATION` helper for measuring execution time.

## Build and run

```
cd transport-catalogue
g++ -std=c++17 -O2 *.cpp -o transport_catalogue
./transport_catalogue
```

The program reads its input from `text.txt` (an example set of requests) and prints a JSON array of responses to the standard output.

## Input format

The input is one JSON object with these sections:

- `base_requests`: stops and buses to load;
- `routing_settings`: bus wait time and velocity;
- `render_settings`: map appearance;
- `stat_requests`: queries to answer.

## Project layout

- `transport_catalogue.*`, `domain.*`, `geo.*`: data model and geography helpers.
- `json_reader.*`: request parsing and handling.
- `transport_router.*`, `graph.h`, `router.h`: route search.
- `map_renderer.*`, `svg.*`: map output.
- `json.*`, `json_builder.*`: JSON library.
