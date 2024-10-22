#include "request_handler.h"

void RequestHandler::AnswerOnRequests() const {
    Print(RequestToHandler(rq_),std::cout);
}

json::Document RequestHandler::RequestToHandler(const std::vector<RequestList>& rl) const {
    json::Array result;
    
    for (const auto& request : rl) {
        if (request.type_ == RequestType::Bus) {
            auto answer = GetBusStat(request.name_);
            if (answer != std::nullopt) {              
                result.emplace_back(CreateBusRequest(request.id_, answer.value()));
            }
            else {
                result.emplace_back(CreateErrorMessage(request.id_));
            }      
        }
        if (request.type_ == RequestType::Stop) {
            result.emplace_back(CreateStopRequest(request.id_, GetBusesByStop(request.name_)));
        }
        if (request.type_ == RequestType::Map) {
            result.emplace_back(CreateMap(request.id_));
        }
        if (request.type_ == RequestType::Route) {
            result.emplace_back(CreateRoute(request.id_, request.from_, request.to_));
        }
    }
    return json::Document{ result };
}

std::optional<BusStat> RequestHandler::GetBusStat(const std::string_view bus_name) const {
    BusStat bs;
    if (tc_.HasBusRoute(bus_name)) {
        bs.curvature_ = tc_.CalcCurvature(bus_name);
        bs.route_length_ = tc_.RealDistanceBusRoute(bus_name);
        bs.stops_ = tc_.GetBusStopsCount(bus_name);
        bs.uniq_stops_ = tc_.GetUniqBusStopsCount(bus_name);
    }
    else {
        return std::nullopt;
    }
    return bs;
}

json::Dict RequestHandler::CreateStopRequest(int id, const std::vector<std::string_view>& buses) const {
    if (buses.size() && buses[0] == "no stop") {
        return CreateErrorMessage(id);
    }
    else {      
        json::Array bus_list{};
        if (buses.size() != 0) {
            for (const auto& bus : buses) {
                std::string sbus = { bus.begin(),bus.end() };
                bus_list.emplace_back(sbus);
            }
        }

        json::Dict req = json::Builder{}.StartDict().Key("buses").Value(bus_list)
            .Key("request_id").Value(id).EndDict().Build().AsDict();
        return req;
    }
}

std::vector<std::string_view> RequestHandler::GetBusesByStop(const std::string_view stop_name) const {
    std::vector<std::string_view> stops;
    if (tc_.HasStop(stop_name)) {
        if (tc_.GetBusList(stop_name).size()) {
            for (const auto& s : tc_.GetBusList(stop_name)) {
                stops.emplace_back(s);
            }
        }
        std::sort(stops.begin(), stops.end());
    }
    else {
        stops.emplace_back("no stop");
    }
    return stops;
}

json::Dict RequestHandler::CreateBusRequest(int id, const BusStat& bs) const {
    using namespace std::literals;
    json::Dict req = json::Builder{}.StartDict()
        .Key("curvature"s).Value(bs.curvature_)
        .Key("request_id"s).Value(id)
        .Key("route_length"s).Value(bs.route_length_)
        .Key("stop_count"s).Value(bs.stops_)
        .Key("unique_stop_count"s).Value(bs.uniq_stops_)
        .EndDict().Build().AsDict();
    return req;
}

json::Dict RequestHandler::CreateErrorMessage(int id) const {
    using namespace std::literals;
    json::Dict req = json::Builder{}.StartDict().Key("request_id"s).Value(id)
        .Key("error_message"s).Value("not found"s)
        .EndDict().Build().AsDict();
    return req;
}

json::Dict RequestHandler::CreateMap(int id) const {
    std::vector<const Bus*> bl = tc_.GetBusesVector();
    using namespace std::literals;
    json::Dict req = json::Builder{}.StartDict().Key("map"s).Value(renderer_.PrintBusRoutes(bl)
        .str()).Key("request_id"s).Value(id)
        .EndDict().Build().AsDict();
    return req;
}

json::Dict RequestHandler::CreateRoute(int id, const std::string& from, const std::string& to) const {
    double time = 0;
    auto route_way = tr_.GetRouteMap(from, to);

    json::Array route_answer;
    for (const auto& a : route_way) {
        if (a.type_ == "error") {
            return CreateErrorMessage(id);
        }
        json::Dict route_elemet;
        if (a.type_ == "Wait") {
            route_elemet["stop_name"] = a.element_name_;
            route_elemet["time"] = a.time_;
            route_elemet["type"] = a.type_;
        }
        if (a.type_ == "Bus") {
            route_elemet["bus"] = a.element_name_;
            route_elemet["span_count"] = a.span_;
            route_elemet["time"] = a.time_;
            route_elemet["type"] = a.type_;
        }
        time += a.time_;
        route_answer.emplace_back(route_elemet);
    }

    json::Dict answer = json::Builder{}.StartDict().Key("items").Value(route_answer).Key("request_id").Value(id).Key("total_time").Value(time).EndDict().Build().AsDict();
    return answer;

}

const RequestType RequestHandler::GetRequestType(const std::string& str) {
    if (str == "Bus") {
        return RequestType::Bus;
    }
    if (str == "Stop") {
        return RequestType::Stop;
    }
    if (str == "Map") {
        return RequestType::Map;
    }
    if (str == "Route") {
        return RequestType::Route;
    }
    return RequestType::Non;
}