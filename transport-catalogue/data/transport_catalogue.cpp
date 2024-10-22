#include <typeinfo>
#include "transport_catalogue.h"


namespace transportcatalogue {

	void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates) {
		Stop stop;
		stop.name = name;
		stop.coordiante = coordinates;
		stops_.push_back(stop);
		std::string_view sv_name = stops_.back().name;
		stopname_to_stop_[sv_name] = &stops_.back();
		stops_to_bus_[sv_name];
	}

	void TransportCatalogue::FillDistanceList(const std::vector<DistanceBetStops>& vector_distance) {
		for (const auto& dist : vector_distance) {
			if (dist.name_stop1 == "" || dist.name_stop2 == "" || dist.distance == 0) {
				return;
			}
			std::string_view stop1 = stopname_to_stop_.at(dist.name_stop1)->name;
			std::string_view stop2 = stopname_to_stop_.at(dist.name_stop2)->name;
			std::pair<std::string_view, std::string_view> pair_stops = std::make_pair(stop1, stop2);

			distance_between_stops_[pair_stops] = dist.distance;
		}
	}

	void TransportCatalogue::AddBus(const std::string& name, bool is_roundtrip, const std::vector<std::string_view>& stops) {
		Bus bus;
		bus.name = name;
		bus.stops.reserve(stops.size());
		bus.is_roundtrip = is_roundtrip;

		for (auto& stop : stops) {
			auto st = stopname_to_stop_.at(stop);
			bus.stops.emplace_back(st);
			bus.uniq_stops.emplace(st);
		}

		buses_.push_back(bus);
		std::string_view sv_name = buses_.back().name;
		busname_to_bus_[sv_name] = &buses_.back();
		

		for (auto& st : buses_.back().stops) {
			stops_to_bus_.at(st->name).insert(buses_.back().name);
		}
	}

	int TransportCatalogue::GetBusStopsCount(std::string_view bus_number) const {
		return static_cast<int>(busname_to_bus_.at(bus_number)->stops.size());
	}

	int TransportCatalogue::GetUniqBusStopsCount(std::string_view bus_number) const  {
		return static_cast<int>(busname_to_bus_.at(bus_number)->uniq_stops.size());
	}

	double TransportCatalogue::MinDistanceBusRoute(std::string_view bus_number) const {
		double result = 0;
		geo::Coordinates a;
		geo::Coordinates b;
		for (size_t i = 0; i != busname_to_bus_.at(bus_number)->stops.size() - 1; ++i) {
			a = busname_to_bus_.at(bus_number)->stops[i]->coordiante;
			b = busname_to_bus_.at(bus_number)->stops[i + 1]->coordiante;
			result += geo::ComputeDistance(a, b);
		}
		return result;
	}

	double TransportCatalogue::GetDistanceByStops(std::string_view stop1, std::string_view stop2) const {
		std::pair<std::string_view, std::string_view> pair_stops = std::make_pair(stop1, stop2);
		if (distance_between_stops_.count(pair_stops)) {
			return distance_between_stops_.at(pair_stops);
		}
		else {
			pair_stops = std::make_pair(stop2, stop1);
			return distance_between_stops_.at(pair_stops);
		}
	}

	double TransportCatalogue::RealDistanceBusRoute(std::string_view bus_number) const {
		double result = 0;

		for (size_t i = 0; i != busname_to_bus_.at(bus_number)->stops.size() - 1; ++i) {
			std::string_view stop1 = busname_to_bus_.at(bus_number)->stops[i]->name;
			std::string_view stop2 = busname_to_bus_.at(bus_number)->stops[i + 1]->name;
			result += GetDistanceByStops(stop1, stop2);
		}
		return result;
	}

	double TransportCatalogue::CalcCurvature(std::string_view bus_number) const {
		return RealDistanceBusRoute(bus_number) / MinDistanceBusRoute(bus_number);
	}

	std::unordered_set<std::string_view> TransportCatalogue::GetBusList(std::string_view stop) const {
		return stops_to_bus_.at(stop);
	}

	bool TransportCatalogue::HasBusRoute(std::string_view bus_number) const {
		return busname_to_bus_.count(bus_number);
	}

	bool TransportCatalogue::HasStop(std::string_view stop) const {
		return stops_to_bus_.count(stop);
	}

	std::vector<const Bus*> TransportCatalogue::GetBusesVector() const {
		std::vector<const Bus*> bus_vector;
		for (const auto& bus : busname_to_bus_) {
			bus_vector.emplace_back(bus.second);
		}
		return bus_vector;
	}

	int TransportCatalogue::GetStopsCount() const {
		return static_cast<int>(stops_.size());
	}

	const std::deque<Stop>& TransportCatalogue::GetStops() const {
		return stops_;
	}

	const std::deque<Bus>& TransportCatalogue::GetBuses() const {
		return buses_;
	}


}