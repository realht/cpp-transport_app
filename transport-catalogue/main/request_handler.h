#pragma once
#include <algorithm>
#include <optional>

#include "../json/json.h"
#include "../json/json_builder.h"
#include "../img/map_renderer.h"
#include "../data/transport_catalogue.h"
#include "../route/transport_router.h"

enum class RequestType {
    Bus,
    Stop,
    Map,
    Route,
    Non
};

struct RequestList {
    int id_;
    RequestType type_;
    std::string name_;
    std::string from_;
    std::string to_;
};

class RequestHandler {
public:
    RequestHandler(const transportcatalogue::TransportCatalogue& tc, const std::vector<RequestList>& rq, const renderer::MapRenderer& renderer, const RouteSetting& rstg)
        : tc_(tc), rq_(rq), renderer_(renderer), tr_(tc, rstg)
    {}

    void AnswerOnRequests() const;
    const static RequestType GetRequestType(const std::string& str) ;

private:
    const transportcatalogue::TransportCatalogue& tc_;
    const std::vector<RequestList> rq_;
    const renderer::MapRenderer& renderer_;
    TransportRouter tr_;

    json::Document RequestToHandler(const std::vector<RequestList>& rl) const;
    json::Dict CreateStopRequest(int id, const std::vector<std::string_view>& buses) const;
    json::Dict CreateBusRequest(int id, const BusStat& bs) const;
    json::Dict CreateErrorMessage(int id) const;

    json::Dict CreateMap(int id) const;
    json::Dict CreateRoute(int id, const std::string& from, const std::string& to) const;

    std::optional<BusStat> GetBusStat(const std::string_view bus_name) const;
    std::vector<std::string_view> GetBusesByStop(const std::string_view stop_name) const;

};
