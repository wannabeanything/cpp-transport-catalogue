#include "map_renderer.h"



void RenderSettingsHandler::Parse(const json::Node& render_settings){
    const auto& settings_as_map = render_settings.AsMap();
    width_ = settings_as_map.at("width").AsDouble();
    height_ = settings_as_map.at("height").AsDouble();
    padding_ = settings_as_map.at("padding").AsDouble();
    routes_settings_.SetStrokeWidth(settings_as_map.at("line_width").AsDouble());
    stops_settings_.SetRadius(settings_as_map.at("stop_radius").AsDouble());

    names_for_bus_settings_.SetFontSize(settings_as_map.at("bus_label_font_size").AsInt());
    names_for_bus_settings_underlayer_.SetFontSize(settings_as_map.at("bus_label_font_size").AsInt());
    svg::Point point = {settings_as_map.at("bus_label_offset").AsArray()[0].AsDouble(), settings_as_map.at("bus_label_offset").AsArray()[1].AsDouble()};
    names_for_bus_settings_.SetOffset(point);
    names_for_bus_settings_underlayer_.SetOffset(point);

    names_for_stop_settings_.SetFontSize(settings_as_map.at("stop_label_font_size").AsInt());
    names_for_stop_settings_underlayer_.SetFontSize(settings_as_map.at("stop_label_font_size").AsInt());
    point = {settings_as_map.at("stop_label_offset").AsArray()[0].AsDouble(), settings_as_map.at("stop_label_offset").AsArray()[1].AsDouble()};
    names_for_stop_settings_.SetOffset(point);
    names_for_stop_settings_underlayer_.SetOffset(point);

    try{
        const auto underlayer_colors = settings_as_map.at("underlayer_color").AsArray();
        if(underlayer_colors.size() == 3){
            svg::Rgb rgb(underlayer_colors[0].AsInt(), underlayer_colors[1].AsInt(), underlayer_colors[2].AsInt());
            names_for_bus_settings_underlayer_.SetStrokeColor(rgb);
            names_for_bus_settings_underlayer_.SetFillColor(rgb);
            names_for_stop_settings_underlayer_.SetStrokeColor(rgb);
            names_for_stop_settings_underlayer_.SetFillColor(rgb);
        }
        else if(underlayer_colors.size() == 4){
            svg::Rgba rgba(underlayer_colors[0].AsInt(), underlayer_colors[1].AsInt(), underlayer_colors[2].AsInt(), underlayer_colors[3].AsDouble());
            names_for_bus_settings_underlayer_.SetStrokeColor(rgba);
            names_for_bus_settings_underlayer_.SetFillColor(rgba);
            names_for_stop_settings_underlayer_.SetStrokeColor(rgba);
            names_for_stop_settings_underlayer_.SetFillColor(rgba);
        }
    }catch(const std::logic_error& e){
        const auto underlayer_color = settings_as_map.at("underlayer_color").AsString();
        names_for_bus_settings_underlayer_.SetStrokeColor(underlayer_color);
        names_for_bus_settings_underlayer_.SetFillColor(underlayer_color);
        names_for_stop_settings_underlayer_.SetStrokeColor(underlayer_color);
        names_for_stop_settings_underlayer_.SetFillColor(underlayer_color);
    }
    names_for_bus_settings_underlayer_.SetStrokeWidth(settings_as_map.at("underlayer_width").AsDouble());
    names_for_stop_settings_underlayer_.SetStrokeWidth(settings_as_map.at("underlayer_width").AsDouble());
    for(const auto& color: settings_as_map.at("color_palette").AsArray()){
        try{
            if(color.AsArray().size() == 3){
                svg::Rgb rgb(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt());
                colors_.push_back(rgb);
            }
            else if(color.AsArray().size() == 4){
                svg::Rgba rgba(color.AsArray()[0].AsInt(), color.AsArray()[1].AsInt(), color.AsArray()[2].AsInt(), color.AsArray()[3].AsDouble());
                colors_.push_back(rgba);
            }
        }catch(const std::logic_error& e){
            colors_.push_back(color.AsString());
        }
    }
    routes_settings_.SetFillColor(svg::NoneColor);
    routes_settings_.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    routes_settings_.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    names_for_bus_settings_underlayer_.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    names_for_bus_settings_underlayer_.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    names_for_stop_settings_underlayer_.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    names_for_stop_settings_underlayer_.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    names_for_bus_settings_.SetFontFamily("Verdana");
    names_for_bus_settings_underlayer_.SetFontFamily("Verdana");
    names_for_bus_settings_.SetFontWeight("bold");
    names_for_bus_settings_underlayer_.SetFontWeight("bold");
    names_for_stop_settings_.SetFontFamily("Verdana");
    names_for_stop_settings_underlayer_.SetFontFamily("Verdana");
    names_for_stop_settings_.SetFillColor("black");
}


void MapRenderer::FillMapRenderer(const json::Node& render_settings){
    rsh_.Parse(render_settings);
    projected_coords_ = MakeSphereProjector();
}
void MapRenderer::DrawMap(std::ostream& out) {
    DrawLines();
    DrawRoutesNames();
    DrawStopsCircles();
    DrawStopsNames();
    final_drawing_.Render(out);
}

SphereProjector MapRenderer::MakeSphereProjector() const{
    std::vector<geo::Coordinates> stops_coords;
    for(const auto& bus: tc_.GetSortedRoutes()){
        for(const auto& stop: bus.stops){
            stops_coords.push_back({stop->latitude, stop->longitude});
        }   
    }
    return SphereProjector{stops_coords.begin(), stops_coords.end(), rsh_.GetWidth(), rsh_.GetHeight(), rsh_.GetPadding()};
}

std::vector<geo::Coordinates> MapRenderer::GetUpdatedCoords(std::vector<const Stop*> stops, SphereProjector proj) const{
    std::vector<geo::Coordinates> temp;
    for(const Stop* stop: stops){
        temp.push_back({stop->latitude, stop->longitude});
    }
    std::vector<geo::Coordinates> result;
    for(const auto coord: temp){
        result.push_back({proj(coord).x, proj(coord).y});
    }
    return result;
}
void MapRenderer::DrawLines(){
    svg::Document doc;
    const auto sorted_routes = tc_.GetSortedRoutes();
    std::vector<svg::Color> colors = rsh_.GetColorPalette();
    size_t index = 0;
    for(const auto& route: sorted_routes){
        if(route.stops.empty()){
            continue;
        }
        std::vector<geo::Coordinates> updated_route_coords = GetUpdatedCoords(route.stops, projected_coords_);
        //Отрисовка линий
        svg::Polyline route_line = rsh_.GetRoutesSettings();
        svg::Color color = colors[index%colors.size()];
        route_line.SetStrokeColor(color);
        for (size_t i = 0; i < updated_route_coords.size(); i++)
        {
            route_line.AddPoint({updated_route_coords[i].lat, updated_route_coords[i].lng});

        }
        final_drawing_.Add(route_line);
        index++;
    }
    
    
}

void MapRenderer::DrawRoutesNames(){
    svg::Text buses_name, buses_name_underlayer;
    buses_name = rsh_.GetBusNamesSettings();
    buses_name_underlayer = rsh_.GetBusNamesUnderlayerSettings();
    std::vector<svg::Color> colors = rsh_.GetColorPalette();
    size_t index = 0;
    for(const auto& route: tc_.GetSortedRoutes()){
        if(route.stops.empty()){
            continue;
        }
        
        buses_name.SetData(route.name);
        buses_name_underlayer.SetData(route.name);
        svg::Color color = colors[index%colors.size()];
        buses_name.SetFillColor(color);
        
        std::vector<geo::Coordinates> updated_coords = GetUpdatedCoords(route.stops, projected_coords_);
        if(route.is_roundtrip || (route.stops[0]->name == route.stops[route.stops.size()/2]->name)){
            buses_name_underlayer.SetPosition({updated_coords[0].lat, updated_coords[0].lng});
            buses_name.SetPosition({updated_coords[0].lat, updated_coords[0].lng});
            final_drawing_.Add(buses_name_underlayer);
            final_drawing_.Add(buses_name);   
        }

        else{
            const auto q = updated_coords[updated_coords.size()/2];
            buses_name_underlayer.SetPosition({updated_coords.front().lat, updated_coords.front().lng});
            buses_name.SetPosition({updated_coords.front().lat, updated_coords.front().lng});
            final_drawing_.Add(buses_name_underlayer);
            final_drawing_.Add(buses_name);
            buses_name_underlayer.SetPosition({q.lat, q.lng});
            buses_name.SetPosition({q.lat, q.lng});
            final_drawing_.Add(buses_name_underlayer);
            final_drawing_.Add(buses_name);
        }
        index++;
    }
    
}
void MapRenderer::DrawStopsCircles(){
    svg::Circle circle = rsh_.GetStopsSettings();
    circle.SetFillColor("white");
    std::vector<geo::Coordinates> updated_coords = GetUpdatedCoords(tc_.GetSortedStops(), projected_coords_);
    for(const auto updated_coord: updated_coords){
        circle.SetCenter({updated_coord.lat, updated_coord.lng});
        final_drawing_.Add(circle);
    }
}

void MapRenderer::DrawStopsNames(){
    svg::Text stops_name, stops_name_underlayer;
    stops_name = rsh_.GetStopNamesSettings();
    stops_name_underlayer = rsh_.GetStopNameUnderlayerSetting();
    const auto sorted_stops = tc_.GetSortedStops();
    std::vector<geo::Coordinates> updated_coords = GetUpdatedCoords(sorted_stops, projected_coords_);
    for (size_t i = 0; i < updated_coords.size(); i++)
    {
        stops_name_underlayer.SetData(sorted_stops[i]->name);
        stops_name_underlayer.SetPosition({updated_coords[i].lat, updated_coords[i].lng});
        stops_name.SetData(sorted_stops[i]->name);
        stops_name.SetPosition({updated_coords[i].lat, updated_coords[i].lng});
        
        final_drawing_.Add(stops_name_underlayer);
        final_drawing_.Add(stops_name);
        
    }
    

}



