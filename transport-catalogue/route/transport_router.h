#pragma once
#include "router.h"
#include "../data/transport_catalogue.h"
#include <iostream>

const int RATIO_MINUTES_TO_HOURS = 60;
const int RATIO_KILOMETERS_TO_METERS = 1000;

struct RouteResult {
	std::string type_;
	std::string element_name_;
	double time_ = 0;
	int span_ = 0;

	RouteResult() = default;

	RouteResult(const std::string& str) :
		type_(str) {}

	RouteResult(const std::string_view type, const std::string_view ename, double time, int span) :
		type_(type), element_name_(ename), time_(time), span_(span) {};
};

struct RouteSetting {
	double bus_wait_time = 6;
	double bus_velocity = 40;
};

struct EdgeParam {
	std::string type_;
	size_t from_;
	size_t to_;
	size_t span_;
	double time_;

	EdgeParam(const std::string& type, size_t from, size_t to, size_t span, double time) :
		type_(type), from_(from), to_(to), span_(span), time_(time) {}
};


class TransportRouter {
public:
	TransportRouter(const transportcatalogue::TransportCatalogue& tc, const RouteSetting& rstg)
		:tc_(tc), rstg_(rstg), stops_graph_(tc.GetStopsCount())
	{
		CreateRouteMap();
	}

	std::vector<RouteResult> GetRouteMap(const std::string& stop1, const std::string& stop2) const ;

private:
	const transportcatalogue::TransportCatalogue& tc_;
	RouteSetting rstg_;

	graph::DirectedWeightedGraph<double> stops_graph_;
	const graph::Router<double>* rt_;

	std::unordered_map<std::string_view, size_t> stop_to_id_;
	std::unordered_map<size_t, std::string_view> id_to_stop_;
	std::vector<EdgeParam> edge_param;

	void CreateRouteMap();
	void TranslateStopToId();
	void FillRouteMap();

	double CalcTimeBetweenStops(const Stop* stop1, const Stop* stop2) const;

	

};

