#include "transport_router.h"


void TransportRouter::CreateRouteMap() {
	TranslateStopToId();
	FillRouteMap();
}

void TransportRouter::TranslateStopToId() {
	size_t id = 0;
	stop_to_id_.reserve(tc_.GetStops().size());
	id_to_stop_.reserve(tc_.GetStops().size());
	for (const Stop& stop : tc_.GetStops()) {
		stop_to_id_[stop.name] = id;
		id_to_stop_[id] = stop.name;
		++id;
	}
}

void TransportRouter::FillRouteMap() {
	for (const Bus& bus : tc_.GetBuses()) {
		for (size_t i = 0; i != bus.stops.size() - 1; ++i) {
			double total_time = rstg_.bus_wait_time;

			for (size_t j = i + 1; j != bus.stops.size(); ++j) {
				total_time += CalcTimeBetweenStops(bus.stops.at(j - 1), bus.stops.at(j));
				stops_graph_.AddEdge({ stop_to_id_.at(bus.stops.at(i)->name), stop_to_id_.at(bus.stops.at(j)->name), total_time });
				edge_param.emplace_back(bus.name, stop_to_id_.at(bus.stops.at(i)->name),
					stop_to_id_.at(bus.stops.at(j)->name), j-i, total_time);
			}
		}
	}
	rt_ = new graph::Router{ stops_graph_ };
}

std::vector<RouteResult> TransportRouter::GetRouteMap(const std::string& stop1, const std::string& stop2) const {
	std::vector<RouteResult> result;

	if (!stop_to_id_.count(stop1) || !stop_to_id_.count(stop2)) {
		RouteResult ep("error");
		result.emplace_back(ep);
		return result;
	}

	auto route = rt_->BuildRoute(stop_to_id_.at(stop1), stop_to_id_.at(stop2));

	if (!route.has_value()) {
		RouteResult ep("error");
		result.emplace_back(ep);
		return result;
	}

	for (size_t i = 0; i != route.value().edges.size(); ++i) {
		RouteResult wait("Wait", id_to_stop_.at(edge_param[route.value().edges[i]].from_), rstg_.bus_wait_time, 0);
		result.emplace_back(wait);

		RouteResult way("Bus", edge_param[route.value().edges[i]].type_, edge_param[route.value().edges[i]].time_ - rstg_.bus_wait_time, edge_param[route.value().edges[i]].span_);
		result.emplace_back(way);
	}

	return result;
}


double TransportRouter::CalcTimeBetweenStops(const Stop* stop1, const Stop* stop2) const {
	if (stop1 == stop2) {
		return rstg_.bus_wait_time;
	}
	return tc_.GetDistanceByStops(stop1->name, stop2->name) /  (rstg_.bus_velocity * RATIO_KILOMETERS_TO_METERS / RATIO_MINUTES_TO_HOURS);
}