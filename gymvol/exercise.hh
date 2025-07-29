#ifndef GYMVOL_EXERCISE_HH
#define GYMVOL_EXERCISE_HH

#include <string>
#include <tuple>

class ExerciseData
{
	std::string m_Name{};
	std::tuple<int, int> m_RepRange{0, 0}; // [start, end]
	std::tuple<int, int> m_SetRange{0, 0}; // [start, end]
	double m_WeightInitial{0};
	double m_WeightIncrement{0};

public:
	ExerciseData()
	{
	}

	template <class Tuple>
	ExerciseData(const std::string &name, Tuple &&repRange, Tuple &&setRange,
	             double weightInitial, double weightIncrement)
	    : m_Name(name)
	    , m_RepRange(std::forward<Tuple>(repRange))
	    , m_SetRange(std::forward<Tuple>(setRange))
	    , m_WeightInitial(weightInitial)
	    , m_WeightIncrement(weightIncrement)
	{
	}

	ExerciseData(const ExerciseData &o)
	    : m_Name(o.m_Name)
	    , m_RepRange(o.m_RepRange)
	    , m_SetRange(o.m_SetRange)
	    , m_WeightInitial(o.m_WeightInitial)
	    , m_WeightIncrement(o.m_WeightIncrement)
	{
	}

	const std::string &GetName() const
	{
		return m_Name;
	}
	int GetLowRep() const
	{
		return std::get<0>(m_RepRange);
	}
	int GetHighRep() const
	{
		return std::get<1>(m_RepRange);
	}
	int GetLowSet() const
	{
		return std::get<0>(m_SetRange);
	}
	int GetHighSet() const
	{
		return std::get<1>(m_SetRange);
	}
	double GetInitialWeight() const
	{
		return m_WeightInitial;
	}
	double GetWeightIncrement() const
	{
		return m_WeightIncrement;
	}
};

#include <format>
#include <iostream>

inline std::ostream &operator<<(std::ostream &out, const ExerciseData &e)
{
	return out << std::format(
	           "{}: \n\tReps: [{}, {}], \n\tSets: [{}, {}], \n\tWeight: "
	           "[Initial: {}, Increment: {}]",
	           e.GetName(), e.GetLowRep(), e.GetHighRep(), e.GetLowSet(),
	           e.GetHighSet(), e.GetInitialWeight(), e.GetWeightIncrement());
}

class ExerciseRun
{
	const ExerciseData *m_Exercise;
	double m_Weight;
	int m_Reps;
	int m_Sets;

public:
	ExerciseRun(const ExerciseData &exercise)
	    : m_Exercise(&exercise)
	    , m_Weight(exercise.GetInitialWeight())
	    , m_Reps(exercise.GetLowRep())
	    , m_Sets(exercise.GetLowSet())
	{
	}

	ExerciseRun(const ExerciseData &exercise, double weight, int reps, int sets)
	    : m_Exercise(&exercise)
	    , m_Weight(weight)
	    , m_Reps(reps)
	    , m_Sets(sets)
	{
	}

	const std::string &GetName() const
	{
		return m_Exercise->GetName();
	}
	double GetWeight() const
	{
		return m_Weight;
	};
	int GetReps() const
	{
		return m_Reps;
	};
	int GetSets() const
	{
		return m_Sets;
	};

	double GetVolume() const
	{
		return m_Weight * (double)m_Reps * (double)m_Sets;
	}

	ExerciseRun GetNext() const
	{
		double weight = m_Weight;
		int reps = m_Reps;
		int sets = m_Sets;

		reps++;
		if (reps <= m_Exercise->GetHighRep())
			goto L_End;
		reps = m_Exercise->GetLowRep();
		sets++;
		if (sets <= m_Exercise->GetHighSet())
			goto L_End;
		sets = m_Exercise->GetLowSet();
		weight += m_Exercise->GetWeightIncrement();
	L_End:
		return ExerciseRun(*m_Exercise, weight, reps, sets);
	};
};

inline std::ostream &operator<<(std::ostream &out, const ExerciseRun &r)
{
	return out << std::format("{}: \n\t{} sets of {} reps of {} kg; {} kg",
	                          r.GetName(), r.GetSets(), r.GetReps(),
	                          r.GetWeight(), r.GetVolume());
}

#endif