#pragma once
#include <sstream>
#include "json.h"
#include "../img/map_renderer.h"
#include "../main/request_handler.h"
#include "../data/transport_catalogue.h"


class JSONReader {
public:
	JSONReader(std::istream& input) {
		ReadJSON(input);
	}

	transportcatalogue::TransportCatalogue GetTransportCatalague() const;
	std::vector<RequestList> GetRequestList() const;
	RenderSettings GetRenderSettings() const;
	RouteSetting GetRoutSetting() const;
	
private:
	transportcatalogue::TransportCatalogue tc_;
	std::vector<RequestList> req_list_;
	RenderSettings rs_;
	RouteSetting rstg_;

	void ReadJSON(std::istream& input);
	void ParseJSON(const json::Document& doc);
	void ParseBaseRequests(const json::Node& node);
	svg::Color FindColor(const json::Node& node) const;
	void ParseRenderSettings(const json::Node& node);
	void ParseStatRequests(const json::Node& node);
	void ParseRoutingSettings(const json::Node& node);
	void AddStopToCatalogue(const std::vector<json::Node>& stops);
	void AddBusToCatalogue(const std::vector<json::Node>& buses);
	
};
