
#include <algorithm>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <simdjson.h>

#include "exercise.hh"

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
			nextRuns[i] = r.GetNext();
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

	void AdvanceWorkout()
	{
		CalculateNextRuns();
		nextCalculated = false;
		runs = nextRuns;

	}
};

std::map<std::string, ExerciseData> exerciseDataMap;

static constexpr char noArgs[] = "gymvol "
                                 "<exercises json> "
                                 "<routine json> "
                                 "<startpoint json> "
                                 "<output json> ";

int main(int argc, char **argv)
{
	if (argc != 5)
	{
		std::cout << noArgs << std::endl;
		return 0;
	}

	using namespace simdjson;

	dom::parser exerciseParser;
	dom::element exerciseJson = exerciseParser.load(argv[1]);

	dom::parser startpointParser;
	dom::element startpointJson = startpointParser.load(argv[2]);

	dom::parser routineParser;
	dom::element routineJson = routineParser.load(argv[3]);

	for (auto exercises : exerciseJson.get_object())
	{
		// too lazy for error checking
		std::string name(exercises.key);
		int lowRep =
		    exercises.value.get_object()["reps"].get_array().at(0).get_int64();
		int highRep =
		    exercises.value.get_object()["reps"].get_array().at(1).get_int64();
		int lowSet =
		    exercises.value.get_object()["sets"].get_array().at(0).get_int64();
		int highSet =
		    exercises.value.get_object()["sets"].get_array().at(1).get_int64();
		double weightInc =
		    exercises.value.get_object()["weightInc"].get_double();
		double weightInit = startpointJson.get_object()[name].get_double();
		if (weightInit < 0)
			weightInit =
			    startpointJson.get_object()["Body Weight"].get_double() * -weightInit;
		ExerciseData data(name, std::tie(lowRep, highRep),
		                  std::tie(lowSet, highSet), weightInit, weightInc);
		exerciseDataMap.emplace(name, std::move(data));
	}

	std::vector<Workout> workouts;
	auto workoutArray = routineJson.get_object()["workouts"].get_array();
	for (auto workout : workoutArray)
	{
		std::vector<ExerciseRun> runs;
		for (auto exercise : workout)
		{
			std::string name(exercise.get_c_str().value());
			runs.emplace(runs.end(), exerciseDataMap[name]);
		}
		workouts.push_back(Workout(runs));
	}

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
	Workout *current_wo = &workouts[cur_wo];

	int total_count = 1;
	int iterations = 5 * 52;
	std::string out_json("");

	out_json += "[\n";
	for (int i = 0; i < iterations; i++)
	{
		current_wo->AdvanceWorkout();
		out_json += "{\n";

		// exercise name
		out_json += "\t\"exercise\" : [";
		for (auto &run : current_wo->GetRuns())
			out_json += "\"" + run.GetName() + "\",";
		out_json.pop_back();
		out_json += "]\n,";

		// reps
		out_json += "\t\"reps\" : [";
		for (auto &run : current_wo->GetRuns())
			out_json += std::to_string(run.GetReps()) + ",";
		out_json.pop_back();
		out_json += "]\n,";

		// sets
		out_json += "\t\"sets\" : [";
		for (auto &run : current_wo->GetRuns())
			out_json += std::to_string(run.GetSets()) + ",";
		out_json.pop_back();
		out_json += "]\n,";

		// weight
		out_json += "\t\"weight\" : [";
		for (auto &run : current_wo->GetRuns())
			out_json += std::to_string(run.GetWeight()) + ",";
		out_json.pop_back();
		out_json += "]\n,";

		// volume
		out_json += "\t\"volume\" : [";
		for (auto &run : current_wo->GetRuns())
			out_json += std::to_string(run.GetVolume()) + ",";
		out_json.pop_back();
		out_json += "]\n,";

		// total_volume
		out_json += "\t\"tvolume\" : [";
		for (auto &run : current_wo->GetRuns())
			out_json += std::to_string(current_wo->GetRunVolume()) + ",";
		out_json.pop_back();
		out_json += "]\n,";

		// count
		out_json += "\t\"count\" : [";
		for (auto &run : current_wo->GetRuns())
			out_json += std::to_string(total_count) + ",";
		out_json.pop_back();
		out_json += "]\n";

		//

		cur_wo =
		    ((cur_wo + 1) % workouts.size()); // advance (mod workouts.size())
		current_wo = &workouts[cur_wo];
		total_count++;

		out_json += "}\n,";
	}
	out_json.pop_back();
	out_json += "]";

	std::ofstream out_json_file(argv[4],
	                            std::ofstream::out | std::ofstream::trunc);
	if (!out_json_file.is_open())
		return 0;
	out_json_file << out_json << std::endl;

	return 0;
}
