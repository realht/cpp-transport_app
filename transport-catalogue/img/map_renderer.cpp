#include "map_renderer.h"

namespace renderer {

    std::ostringstream MapRenderer::PrintBusRoutes(std::vector<const Bus*>& bus_list) const {
        std::sort(bus_list.begin(), bus_list.end(), [](const auto& l, const auto& r) {
            return l->name < r->name; });

        svg::Document routemap;
        size_t palette_number = 0;
        std::vector<geo::Coordinates> all_stops_coord;
        std::vector<const Stop*> all_stops;

        for (const auto& bus : bus_list) {
            all_stops_coord.reserve(all_stops_coord.size() + bus->stops.size());
            for (const auto& stop : bus->stops) {
                if(std::find(all_stops.begin(), all_stops.end(), stop) == all_stops.end()){
                    all_stops.emplace_back(stop);
                    all_stops_coord.emplace_back(stop->coordiante);
                }
            }           
        }

        const SphereProjector proj = CreateProjector(all_stops_coord);

        //print route line
        for (const auto& bus : bus_list) {
            svg::Polyline route;
            if(!bus->stops.empty()){
                for (const auto& stop : bus->stops) {
                    const svg::Point screen_coord = proj(stop->coordiante);
                    route.AddPoint({ screen_coord.x, screen_coord.y });
                }
                route.SetStrokeColor(rs_.color_palette[palette_number]).SetFillColor("none").SetStrokeWidth(rs_.line_width);
                route.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                routemap.Add(route);

                palette_number = GetNextColorPalette(palette_number);
            }
        }

        //print route name
        palette_number = 0;
        for (const auto& bus : bus_list) {
            if (!bus->stops.empty()) {
                const svg::Point screen_coord = proj(bus->stops[0]->coordiante);
                routemap.Add(CreateRouteBusNameMain(bus->name, screen_coord));
                routemap.Add(CreateRouteBusNameAdd(bus->name, screen_coord, palette_number));

                if (!bus->is_roundtrip && bus->stops[0] != bus->stops[bus->stops.size() / 2]) {
                    const svg::Point screen_coord2 = proj(bus->stops[bus->stops.size()/2]->coordiante);
                    routemap.Add(CreateRouteBusNameMain(bus->name, screen_coord2));
                    routemap.Add(CreateRouteBusNameAdd(bus->name, screen_coord2, palette_number));
                }
                palette_number = GetNextColorPalette(palette_number);
            }
        }

        std::sort(all_stops.begin(), all_stops.end(), [](const auto& l, const auto& r) {
            return l->name < r->name; });

        //Print stop circle
        for (const auto& stop : all_stops) {
            const svg::Point sc = proj(stop->coordiante);
            routemap.Add(svg::Circle{}.SetCenter(sc).SetRadius(rs_.stop_radius).SetFillColor("white"));
        }

        //Print stop name
        for (const auto& stop : all_stops) {
            const svg::Point sc = proj(stop->coordiante);
            routemap.Add(CreateRouteStopNameMain(stop->name, sc));
            routemap.Add(CreateRouteStopNameAdd(stop->name, sc));
        }
        
        std::ostringstream out;
        routemap.Render(out);
        return out;
    }

    SphereProjector MapRenderer::CreateProjector(const std::vector<geo::Coordinates>& vc) const {
        const SphereProjector proj{
        vc.begin(), vc.end(), rs_.width, rs_.height, rs_.padding };
        return proj;
    }

    size_t MapRenderer::GetNextColorPalette(size_t n) const {
        return n >= rs_.color_palette.size() - 1 ? n = 0 : ++n;
    }

    svg::Text MapRenderer::CreateRouteBusNameMain(const std::string& name, svg::Point co) const {
        svg::Text text;
        text.SetFillColor(rs_.underlayer_color).SetStrokeColor(rs_.underlayer_color).SetStrokeWidth(rs_.underlayer_width);
        text.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text.SetPosition({ co.x, co.y }).SetOffset({ rs_.bus_label_offset.x,rs_.bus_label_offset.y });
        text.SetFontSize(rs_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(name);
        return text;
    }

    svg::Text MapRenderer::CreateRouteBusNameAdd(const std::string& name, svg::Point co, size_t color_id) const {
        svg::Text text;
        text.SetFillColor(rs_.color_palette[color_id]);
        text.SetPosition({ co.x, co.y }).SetOffset({ rs_.bus_label_offset.x,rs_.bus_label_offset.y });
        text.SetFontSize(rs_.bus_label_font_size).SetFontFamily("Verdana").SetFontWeight("bold").SetData(name);
        return text;
    }

    svg::Text MapRenderer::CreateRouteStopNameMain(const std::string& name, svg::Point co) const {
        svg::Text text;
        text.SetFillColor(rs_.underlayer_color).SetStrokeColor(rs_.underlayer_color).SetStrokeWidth(rs_.underlayer_width);
        text.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text.SetPosition({ co.x, co.y }).SetOffset({ rs_.stop_label_offset.x,rs_.stop_label_offset.y });
        text.SetFontSize(rs_.stop_label_font_size).SetFontFamily("Verdana").SetData(name);
        return text;
    }

    svg::Text MapRenderer::CreateRouteStopNameAdd(const std::string& name, svg::Point co) const {
        svg::Text text;
        text.SetFillColor("black");
        text.SetPosition({ co.x, co.y }).SetOffset({ rs_.stop_label_offset.x,rs_.stop_label_offset.y });
        text.SetFontSize(rs_.stop_label_font_size).SetFontFamily("Verdana").SetData(name);
        return text;
    }
}