
#include <algorithm>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>

struct Exercise
{
	std::pair<int, int> repRange;
	std::pair<int, int> setRange;
	int weight;
	int weightIncrement;
};


static constexpr char noArgs[] = "gymvol "
                                 "<rep-start> <rep-end> "
                                 "<set-start> <set-end> "
                                 "<weight> <weight-inc> "
                                 "<week> <out_file>";

int main(int argc, char **argv)
{
	if (argc != 9)
	{
		std::cout << noArgs << std::endl;
		return 0;
	}

	auto val = [&]<size_t... p>(std::index_sequence<p...>) {
		return std::make_tuple(std::atoi(argv[(p + 1)])...);
	}(std::make_index_sequence<7>{});

	if (!std::apply([](auto &&...args) { return std::min({args...}); }, val))
	{
		std::cout << "Invalid arguments!" << std::endl;
		return 0;
	}

	auto [repStart, repEnd, setStart, setEnd, weightStart, weightInc, week] =
	    val;

	int repLength = repEnd - repStart;
	int setLength = setEnd - setStart;

	int rep = repStart;
	int set = setStart;
	int weight = weightStart;

	int volume = rep * set * weight;

	auto multiplyElements = [&]<size_t... p>(auto &&t,
	                                         std::index_sequence<p...>) {
		return (std::get<p>(t) * ...);
	};

	auto findNextLowestVolume = [&]() {
		int lowest = volume;
		std::tuple<int, int, int> minVolume = std::make_tuple(1000, 1000, 1000);
		for (int _rep = repStart; _rep <= repEnd; _rep++)
			for (int _set = setStart; _set <= setEnd; _set++)
				for (int _weight = weightStart;
				     _weight <= weight + 8 * weightInc; _weight += weightInc)
				{
					auto rsw = std::tie(_rep, _set, _weight);
					int prev = multiplyElements(minVolume,
					                            std::make_index_sequence<3>{});
					int current =
					    multiplyElements(rsw, std::make_index_sequence<3>{});
					minVolume =
					    (current < prev && current > lowest) ? rsw : minVolume;

					// std::cout << std::format("{} < {} && {} > {}", current,
					// prev, current, lowest) << std::endl;
				}
		volume = multiplyElements(minVolume, std::make_index_sequence<3>{});
		rep = std::get<0>(minVolume);
		set = std::get<1>(minVolume);
		weight = std::get<2>(minVolume);
	};

	std::ofstream out(argv[8]);

	for (int i = 1; i < week; i++)
	{
		out << std::format(
		           "Week #{}:\n\tVolume = {}, Rep = {}, Set = {}, Weight = {}",
		           i, volume, rep, set, weight)
		    << std::endl;
		findNextLowestVolume();
	}
	out << std::format(
	           "Week #{}:\n\tVolume = {}, Rep = {}, Set = {}, Weight = {}", week,
	           volume, rep, set, weight)
	    << std::endl;

	out.flush();
	out.close();

	return 0;
}
