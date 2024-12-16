#pragma once
#include "geo.h"
#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <map>




class RenderSettingsHandler{
public:
    RenderSettingsHandler() = default;
    void Parse(const json::Node& render_settings);
    svg::Circle GetStopsSettings() const{
        return stops_settings_;
    }
    svg::Polyline GetRoutesSettings() const{
        return routes_settings_;
    }
    svg::Text GetStopNamesSettings() const{
        return names_for_stop_settings_;
    }
    svg::Text GetStopNameUnderlayerSetting() const{
        return names_for_stop_settings_underlayer_;
    }
    svg::Text GetBusNamesSettings() const{
        return names_for_bus_settings_;
    }
    svg::Text GetBusNamesUnderlayerSettings() const{
        return names_for_bus_settings_underlayer_;
    }
    std::vector<svg::Color> GetColorPalette() const{
        return colors_;
    }
    double GetHeight() const{
        return height_;
    }
    double GetWidth() const{
        return width_;
    }
    double GetPadding() const{
        return padding_;
    }
private:
    svg::Circle stops_settings_;
    svg::Polyline routes_settings_;
    svg::Text names_for_stop_settings_;
    svg::Text names_for_stop_settings_underlayer_;
    svg::Text names_for_bus_settings_;
    svg::Text names_for_bus_settings_underlayer_;
    std::vector<svg::Color> colors_;
    double height_;
    double width_;
    double padding_;
};






inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    SphereProjector() = default;    
    // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                    double max_width, double max_height, double padding)
        : padding_(padding) //
    {
        // Если точки поверхности сферы не заданы, вычислять нечего
        if (points_begin == points_end) {
            return;
        }

        // Находим точки с минимальной и максимальной долготой
        const auto [left_it, right_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        // Находим точки с минимальной и максимальной широтой
        const auto [bottom_it, top_it] = std::minmax_element(
            points_begin, points_end,
            [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        // Вычисляем коэффициент масштабирования вдоль координаты x
        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        // Вычисляем коэффициент масштабирования вдоль координаты y
        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            // Коэффициенты масштабирования по ширине и высоте ненулевые,
            // берём минимальный из них
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            // Коэффициент масштабирования по ширине ненулевой, используем его
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            // Коэффициент масштабирования по высоте ненулевой, используем его
            zoom_coeff_ = *height_zoom;
        }
    }

    // Проецирует широту и долготу в координаты внутри SVG-изображения
    svg::Point operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRenderer{
public:
    MapRenderer(TransportCatalogue& catalogue):tc_(catalogue){}
    void FillMapRenderer(const json::Node& render_settings);
    void DrawMap(std::ostream& out);

private:
    void DrawLines();
    void DrawRoutesNames();
    void DrawStopsCircles();
    void DrawStopsNames(); 
    SphereProjector MakeSphereProjector() const;
    std::vector<geo::Coordinates> GetUpdatedCoords(std::vector<const Stop*> stops, SphereProjector proj) const;
    RenderSettingsHandler rsh_;
    TransportCatalogue& tc_;
    SphereProjector projected_coords_;
    svg::Document final_drawing_;
    
};




