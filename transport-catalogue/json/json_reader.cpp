#include "json_reader.h"

void JSONReader::ReadJSON(std::istream& input){
    json::Document parsed_node = json::Load(input);
    ParseJSON(parsed_node);
}

void JSONReader::ParseJSON(const json::Document& doc) {
    if (!doc.GetRoot().IsDict()) {
        throw json::ParsingError("wrong json");
    }

    for (const auto& req : doc.GetRoot().AsDict()) {

        if (req.first == "base_requests") {
            ParseBaseRequests(req.second.AsArray());
        }

        if (req.first == "render_settings") {
            ParseRenderSettings(req.second.AsDict());
        }

        if (req.first == "stat_requests") {
            ParseStatRequests(req.second.AsArray());
        }  

        if(req.first == "routing_settings") {
            ParseRoutingSettings(req.second.AsDict());
        }
    }
}

void JSONReader::ParseBaseRequests(const json::Node& node) {
    std::vector<json::Node> stops;
    std::vector<json::Node> buses;
    if (!node.IsArray()) {
        throw json::ParsingError("wrong json");
    }

    for (const auto& n : node.AsArray()) {
        if (!n.IsDict()) {
            throw json::ParsingError("wrong json");
        }

        for (const auto& m : n.AsDict()) {
            if (m.first == "type" && m.second.AsString() == "Stop") {
                stops.emplace_back(n);
            }
            if (m.first == "type" && m.second.AsString() == "Bus") {
                buses.emplace_back(n);
            }
        }
    }
   
    AddStopToCatalogue(stops);
    stops.clear();
    AddBusToCatalogue(buses);
    buses.clear();
}

transportcatalogue::TransportCatalogue JSONReader::GetTransportCatalague() const {
    return tc_;
}


void JSONReader::ParseStatRequests(const json::Node & node) {
    RequestList rl;
    if (!node.IsArray()) {
        throw json::ParsingError("wrong stat_requests");
    }

    for (const auto& request : node.AsArray()) {
        if (!request.IsDict()) {
            throw json::ParsingError("wrong stat_requests");
        }
        for (const auto& req : request.AsDict()) {
            if (req.first == "id") {
                rl.id_ = req.second.AsInt();
            }
            if (req.first == "type") {
                rl.type_ = RequestHandler::GetRequestType(req.second.AsString());
            }
            if (req.first == "name") {
                rl.name_ = req.second.AsString();
            }   
            if (req.first == "from") {
                rl.from_ = req.second.AsString();
            }
            if (req.first == "to") {
                rl.to_ = req.second.AsString();
            }
        }
        req_list_.emplace_back(rl);
    }
}

std::vector<RequestList> JSONReader::GetRequestList() const {
    return req_list_;
}

svg::Color JSONReader::FindColor(const json::Node& node) const {
    svg::Color color;
    if (node.IsString()) {
        svg::Color color{ node.AsString() };
        return color;
    }
    else if (node.IsArray()) {
        if (node.AsArray().size() == 3) {
            svg::Color color;
            return color = svg::Rgb{ static_cast<uint8_t>(node.AsArray()[0].AsInt()),
                static_cast<uint8_t>(node.AsArray()[1].AsInt()), static_cast<uint8_t>(node.AsArray()[2].AsInt()) };
        }
        if (node.AsArray().size() == 4) {
            svg::Color color;
            return color = svg::Rgba{ static_cast<uint8_t>(node.AsArray()[0].AsInt()), static_cast<uint8_t>(node.AsArray()[1].AsInt()),
                static_cast<uint8_t>(node.AsArray()[2].AsInt()), node.AsArray()[3].AsDouble() };
        }
    }
    return color;
}

void JSONReader::ParseRenderSettings(const json::Node& node) {
    using namespace std;
    if (!node.IsDict()) {
        throw json::ParsingError("wrong stat_requests");
    }

    for (const auto& n : node.AsDict()) {
        if (n.first == "width") {
            rs_.width = n.second.AsDouble();
        }
        if (n.first == "height") {
            rs_.height = n.second.AsDouble();
        }
        if (n.first == "padding") {
            rs_.padding = n.second.AsDouble();
        }
        if (n.first == "stop_radius") {
            rs_.stop_radius = n.second.AsDouble();
        }
        if (n.first == "line_width") {
            rs_.line_width = n.second.AsDouble();
        }
        if (n.first == "bus_label_font_size") {
            rs_.bus_label_font_size = n.second.AsInt();
        }
        if (n.first == "bus_label_offset") {
            rs_.bus_label_offset.x = n.second.AsArray().at(0).AsDouble();
            rs_.bus_label_offset.y = n.second.AsArray().at(1).AsDouble();
        }
        if (n.first == "stop_label_font_size") {
            rs_.stop_label_font_size = n.second.AsInt();
        }
        if (n.first == "stop_label_offset") {
            rs_.stop_label_offset.x = n.second.AsArray().at(0).AsDouble();
            rs_.stop_label_offset.y = n.second.AsArray().at(1).AsDouble();
        }
        if (n.first == "underlayer_color") {
            rs_.underlayer_color = FindColor(n.second);
        }
        if (n.first == "underlayer_width") {
            rs_.underlayer_width = n.second.AsDouble();
        }
        if (n.first == "color_palette") {
            for (const auto& palette : n.second.AsArray()) {
                rs_.color_palette.emplace_back(FindColor(palette));
            }
        }
    }
}

RenderSettings JSONReader::GetRenderSettings() const {
    return rs_;
}

void JSONReader::ParseRoutingSettings(const json::Node& node) {
    if (!node.IsDict()) {
        throw json::ParsingError("wrong json");
    }

    for (const auto& setting : node.AsDict()) {
        if (setting.first == "bus_wait_time") {
            rstg_.bus_wait_time = setting.second.AsInt();
        }
        if (setting.first == "bus_velocity") {
            rstg_.bus_velocity = setting.second.AsDouble();
        }
    }
}

RouteSetting JSONReader::GetRoutSetting() const {
    return rstg_;
}

void JSONReader::AddStopToCatalogue(const std::vector<json::Node>& stops) {
    std::string name;
    std::vector<std::pair<std::string,int>> next_stop;
    geo::Coordinates coords;
    std::vector<transportcatalogue::DistanceBetStops> dbp;
    std::vector<std::vector<transportcatalogue::DistanceBetStops>> distance_list;
    
    for (const auto& stop : stops) {
        if (!stop.IsDict()) {
            throw json::ParsingError("wrong json");
        }
            for (const auto& s : stop.AsDict()) {
                if (s.first == "name") {
                    name = s.second.AsString();
                }
                if (s.first == "latitude") {
                    coords.lat = s.second.AsDouble();
                }
                if (s.first == "longitude") {
                    coords.lng = s.second.AsDouble();
                }
                if (s.first == "road_distances") {
                    if (s.second.IsDict()) {
                        for (const auto& next : s.second.AsDict()) {
                            next_stop.emplace_back(std::make_pair(next.first, next.second.AsInt()));
                        }
                    }
                }
            }
        
        for (const auto& a : next_stop) {
            dbp.emplace_back(transportcatalogue::DistanceBetStops( name, a.first, a.second ));
        }
        distance_list.emplace_back(dbp);
        tc_.AddStop(name, coords);

        next_stop.clear();
        dbp.clear();
    }
    
    for (const auto& list : distance_list) {
        tc_.FillDistanceList(list);
    }
    distance_list.clear();
}

void JSONReader::AddBusToCatalogue(const std::vector<json::Node>& buses) {
    std::string name;
    bool is_roundtrip = false;
    std::vector<std::string_view> stops;
    std::vector<std::string_view> route;

    for (const auto& bus : buses) {
        if (!bus.IsDict()) {
            throw json::ParsingError("wrong json");
        }
        for (const auto& bu : bus.AsDict()) {
            if (bu.first == "name") {
                name = bu.second.AsString();
            }
            if (bu.first == "is_roundtrip") {
                is_roundtrip = bu.second.AsBool();
            }
            if (bu.first == "stops") {
                if (!bu.second.IsArray()) {
                    throw json::ParsingError("wrong json");
                }
                for (const auto& b : bu.second.AsArray()) {  
                        stops.emplace_back(b.AsString());
                }
            }
        }

        route = stops;
        if (!is_roundtrip) {
            for (size_t i = stops.size() - 1; i != 0; --i) {
                route.emplace_back(stops[i - 1]);
            }
        }
    
        tc_.AddBus(name, is_roundtrip, route);
        route.clear();
        stops.clear();
    }
}
