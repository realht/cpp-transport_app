#pragma once
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "geo.h"

struct Bus;
struct Stop;

struct Stop {
	std::string name;
	geo::Coordinates coordiante;
};

struct Bus{
	std::string name;
	bool is_roundtrip;
	std::vector<const Stop*> stops;
	std::unordered_set<const Stop*> uniq_stops;
};

struct BusStat {
	int stops_;
	int uniq_stops_;
	double route_length_;
	double curvature_;
};
