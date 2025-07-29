
#include <algorithm>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>

struct ExerciseData
{
	std::string name;
	std::pair<int, int> repRange; // [start, end]
	std::pair<int, int> setRange; // [start, end]
	float weightInitial{0};
	float weightIncrement{0};
};

struct ExerciseRun
{
	const ExerciseData *data;
	float weight;
	int reps;
	int sets;
	float cachedVolume;

	ExerciseRun(const ExerciseData &_data)
	    : data(&_data)
	    , weight(_data.weightInitial)
	    , reps(std::get<0>(_data.repRange))
	    , sets(std::get<0>(_data.setRange))
	    , cachedVolume(GetVolume())
	{
	}

	ExerciseRun(const ExerciseData *_data, float _weight, int _reps, int _sets)
	    : data(_data)
	    , weight(_weight)
	    , reps(_reps)
	    , sets(_sets)
	    , cachedVolume(GetVolume())
	{
	}

	float GetVolume() const
	{
		return weight * (float)reps * (float)sets;
	}

	float GetWeightedVolume() const
	{
		return (expf((weight /* - data->weightInitial*/) / 10.0)) *
		       ((float)(reps) * (float)(sets));
	}

	ExerciseRun GetLowestNextRun() const
	{
		float lowest = GetWeightedVolume();
		ExerciseRun minRun =
		    ExerciseRun{data, 10000, std::get<1>(data->repRange),
		                std::get<1>(data->setRange)};
		int repStart = std::get<0>(data->repRange);
		int setStart = std::get<0>(data->setRange);
		float weightStart = data->weightInitial;
		float weightTopLimit = weight + 8 * data->weightIncrement;

		for (int _rep = repStart; _rep <= std::get<1>(data->repRange); _rep++)
			for (int _set = setStart; _set <= std::get<1>(data->setRange);
			     _set++)
				for (float _weight = weightStart; _weight <= weightTopLimit;
				     _weight += data->weightIncrement)
				{
					ExerciseRun candidate{data, _weight, _rep, _set};
					float prev = minRun.GetWeightedVolume();
					float current = candidate.GetWeightedVolume();
					if (current < prev && current > lowest)
					{
						minRun = candidate;
						break;
					}

					/*std::cout << std::format("{} < {} && {} > {}", current,
					                         prev, current, lowest)
					          << std::endl;*/
				}
		return minRun;
	};
};

struct Workout
{
	std::vector<ExerciseRun> runs;
	std::vector<ExerciseRun> nextRuns;
	bool nextCalculated{false};

	Workout(const std::vector<ExerciseRun> &_runs)
	    : runs(_runs)
	    , nextRuns(_runs)
	{
		CalculateNextRuns();
	}

	const auto &GetRuns()
	{
		return runs;
	}

	void CalculateNextRuns()
	{
		if (nextCalculated)
			return;
		nextCalculated = true;
		int i = 0;
		for (auto &r : runs)
		{
			nextRuns[i] = r.GetLowestNextRun();
			i++;
		}
	}

	float GetRunVolume()
	{
		float volume{0};
		for (auto &r : runs)
		{
			volume += r.GetVolume();
		};
		return volume;
	}

	float GetRunWeightedVolume()
	{
		float volume{0};
		float tweight{0};
		float trepset{0};
		for (auto &r : runs)
		{
			tweight += r.weight;
			trepset += (r.reps * r.sets);
			volume += r.GetWeightedVolume();
		};
		volume = expf(tweight / 10.f) * trepset;
		return volume;
	}

	float GetNextRunVolume()
	{
		float volume{0};
		CalculateNextRuns();
		for (auto &r : nextRuns)
		{
			volume += r.GetVolume();
		};
		return volume;
	}

	float GetNextRunWeightedVolume()
	{
		float volume{0};
		CalculateNextRuns();
		float tweight{0};
		float trepset{0};
		for (auto &r : nextRuns)
		{
			tweight += r.weight;
			trepset += (r.reps * r.sets);
			volume += r.GetWeightedVolume();
		};
		volume = expf(tweight / 10.f) * trepset;
		return volume;
	}

	void AdvanceWorkout()
	{
		CalculateNextRuns();
		nextCalculated = false;
		runs = nextRuns;
	}
};

std::ostream &operator<<(std::ostream &out, const ExerciseData &e)
{
	return out << std::format(
	           "{}: \n\tReps: [{}, {}], \n\tSets: [{}, {}], \n\tWeight: "
	           "[Initial: {}, Increment: {}]",
	           e.name, std::get<0>(e.repRange), std::get<1>(e.repRange),
	           std::get<0>(e.setRange), std::get<1>(e.setRange),
	           e.weightInitial, e.weightIncrement);
}

std::ostream &operator<<(std::ostream &out, const ExerciseRun &r)
{
	return out << std::format("{}: \n\t{} sets of {} reps of {} kg; {} kg",
	                          r.data->name, r.sets, r.reps, r.weight,
	                          r.GetVolume());
}

void from_json(const nlohmann::json &j, ExerciseData &e)
{
	j.at("reps").at(0).get_to(std::get<0>(e.repRange));
	j.at("reps").at(1).get_to(std::get<1>(e.repRange));
	j.at("sets").at(0).get_to(std::get<0>(e.setRange));
	j.at("sets").at(1).get_to(std::get<1>(e.setRange));
	j.at("weightInc").get_to(e.weightIncrement);
}

std::map<std::string, ExerciseData> exerciseDataMap;

static constexpr char noArgs[] = "gymvol "
                                 "<routine json>";

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << noArgs << std::endl;
		return 0;
	}

	std::ifstream exercises_json_file("exercises2.json");
	if (!exercises_json_file.is_open())
	{
		std::cerr << "Could not open \"exercises.json\" file." << std::endl;
		return 0;
	}
	std::ifstream startpoint_json_file("startpoint.json");
	if (!startpoint_json_file.is_open())
	{
		exercises_json_file.close();
		std::cerr << "Could not open \"startpoint.json\" file." << std::endl;
		return 0;
	}
	std::ifstream routine_json_file(argv[1]);
	if (!routine_json_file.is_open())
	{
		exercises_json_file.close();
		startpoint_json_file.close();
		std::cerr << std::format("Could not open \"{}\" file.", argv[1])
		          << std::endl;
		return 0;
	}
	nlohmann::json exercises_json = nlohmann::json::parse(exercises_json_file);
	nlohmann::json startpoint_json =
	    nlohmann::json::parse(startpoint_json_file);
	nlohmann::json routine_json = nlohmann::json::parse(routine_json_file);
	exercises_json_file.close();
	startpoint_json_file.close();
	routine_json_file.close();
	std::string exercise_name;
	for (auto it = exercises_json.begin(); it != exercises_json.end(); it++)
	{
		exerciseDataMap[it.key()] = it.value().template get<ExerciseData>();
		exerciseDataMap[it.key()].name = it.key();
		exerciseDataMap[it.key()].weightInitial =
		    startpoint_json.at(it.key()).template get<float>();
	}

	std::vector<Workout> workouts;
	for (auto &workout : routine_json.at("workouts"))
	{
		std::vector<ExerciseRun> runs;
		for (auto &exercise : workout)
		{
			std::string name = exercise.template get<std::string>();
			runs.emplace(runs.end(), exerciseDataMap[name]);
		}
		workouts.push_back(Workout(runs));
	}
	float volume_slope =
	    startpoint_json.at("VOLUME_SLOPE").template get<float>();

	std::cout << "Workouts: \n";
	{
		int i = 0;
		for (auto &w : workouts)
		{
			std::cout << "Workout " << i << ": \n";
			for (auto &run : w.GetRuns())
				std::cout << run << std::endl;
			i++;
		}
	}

	/*std::cout << "\nNEXT RUN\n";

	for (auto &run : runs)
	{
	    run = run.GetLowestNextRun();
	    std::cout << run << std::endl;
	}*/

	std::cout.flush();

	int cur_wo = 0;
	auto &current_wo = workouts[cur_wo];
	float past_volume = current_wo.GetNextRunWeightedVolume() - volume_slope;
	float unchanged_days = 0.0;

	int total_count = 1;
	nlohmann::json out_json;
	for (int i = 0; i < 5 * 52; i++)
	{
		unchanged_days += 1.0;
		float slope =
		    (current_wo.GetNextRunWeightedVolume() - past_volume) / unchanged_days;
		bool changed{false};
		while (slope <= volume_slope)
		{
			changed = true;
			current_wo.AdvanceWorkout();
			slope =
			    (current_wo.GetNextRunWeightedVolume() - past_volume) / unchanged_days;
		}
		if (changed)
		{
			unchanged_days = 0.0;
			past_volume = current_wo.GetRunWeightedVolume();
		}
		int count = 0;
		for (auto &run : current_wo.GetRuns())
		{
			out_json[i]["exercise"][count] = run.data->name;
			out_json[i]["reps"][count] = run.reps;
			out_json[i]["sets"][count] = run.sets;
			out_json[i]["weight"][count] = run.weight;
			out_json[i]["volume"][count] = run.GetVolume();
			out_json[i]["wvolume"][count] = run.GetWeightedVolume();
			out_json[i]["tvolume"][count] = current_wo.GetRunVolume();
			out_json[i]["twvolume"][count] = current_wo.GetRunWeightedVolume();
			out_json[i]["count"][count] = total_count;
			count++;
		}
		cur_wo = ((cur_wo+1) % workouts.size()); // advance (mod workouts.size())
		current_wo = workouts[cur_wo];
		total_count++;
	}

	std::ofstream out_json_file("spreadsheet.json",
	                            std::ofstream::out | std::ofstream::trunc);
	if (!out_json_file.is_open())
		return 0;
	out_json_file << std::setw(4) << out_json << std::endl;

	/*auto val = [&]<size_t... p>(std::index_sequence<p...>) {
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
	           "Week #{}:\n\tVolume = {}, Rep = {}, Set = {}, Weight = {}",
	week, volume, rep, set, weight)
	    << std::endl;

	out.flush();
	out.close();*/

	return 0;
}
