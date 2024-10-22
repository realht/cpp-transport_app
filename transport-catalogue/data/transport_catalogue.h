#pragma once
#include <deque>
#include <functional>
#include <iostream>
#include <string_view>
#include <unordered_map>

#include "domain.h"

namespace transportcatalogue {

	struct DistanceBetStops {
		DistanceBetStops(std::string_view stop1, std::string_view stop2, int dis)
			: name_stop1(stop1), name_stop2(stop2), distance(dis)
		{}
		std::string name_stop1;
		std::string name_stop2;
		int distance;
	};

	class DBSHasher {
	public:
		size_t operator()(const std::pair<std::string_view, std::string_view>& d) const {
			std::size_t a = std::hash<std::string_view>{}(d.first);
			std::size_t b = std::hash<std::string_view>{}(d.second);
			const int c = 17;
			return a*c*c + b * c;
		}
	};

	class TransportCatalogue {
	public:
		//add Transport Catalogue information
		void AddStop(const std::string& name, geo::Coordinates coordinates);
		void FillDistanceList(const std::vector<DistanceBetStops>& vector_dist);
		void AddBus(const std::string& name, bool is_roundtrip, const std::vector<std::string_view>& stops);

		//get Transport Catalogue information
		int GetBusStopsCount(std::string_view bus_number) const ;
		int GetUniqBusStopsCount(std::string_view bus_number) const ;
		int GetStopsCount() const;
		double GetDistanceByStops(std::string_view stop1, std::string_view stop2) const;
		double RealDistanceBusRoute(std::string_view bus_number) const;
		double CalcCurvature(std::string_view bus_number) const;
		std::unordered_set<std::string_view> GetBusList(std::string_view stop) const;
		bool HasBusRoute(std::string_view bus_number) const;	
		bool HasStop(std::string_view stop) const;
		std::vector<const Bus*> GetBusesVector() const ;
		//
		const std::deque<Bus>& GetBuses() const;
		const std::deque<Stop>& GetStops() const;

	private:
		//base information
		std::deque<Stop> stops_;
		std::deque<Bus> buses_;

		//additional info container
		std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
		std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
		std::unordered_map<std::string_view, std::unordered_set<std::string_view>> stops_to_bus_;
		std::unordered_map<std::pair<std::string_view, std::string_view>, int, DBSHasher> distance_between_stops_;

		double MinDistanceBusRoute(std::string_view bus_number) const;
	};

}