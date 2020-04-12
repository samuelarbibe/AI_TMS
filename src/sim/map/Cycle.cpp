//
// Created by Samuel Arbibe on 06/04/2020.
//

#include "Cycle.hpp"

int Cycle::CycleCount = 0;

bool compare_priority(Phase *first, Phase *second) {
	return (first->GetPriorityScore() < second->GetPriorityScore());
}

template<typename T>
void pop_front(std::vector<T> &vec) {
	assert(!vec.empty());
	vec.erase(vec.begin());
}

Cycle::Cycle(int cycleNumber, Intersection *intersection) {
	cycle_number_ = cycleNumber;
	intersection_ = intersection;
	number_of_phases_ = 0;

	input_values_ = vector<double>(2, 0);
	output_values_ = vector<double>(1, 0);
}

Cycle::~Cycle() {
	for (Phase *p : phases_)
	{
		delete p;
	}

	cout << "Cycle number " << cycle_number_ << " has been deleted." << endl;
}

void Cycle::Update(float elapsedTime) {
	for (Phase *p : phases_)
	{
		p->Update(elapsedTime);
	}

	cycle_phases();
}

void Cycle::ReloadCycle() {

	for (Phase *p : phases_)
	{
		p->ReloadPhase();
	}
}

Phase *Cycle::AddPhase(int phaseNumber, float cycleTime) {

	Phase *temp;

	if (phaseNumber == 0)
	{
		phaseNumber = Phase::PhaseCount + 1;
	}

	temp = new Phase(phaseNumber, this->cycle_number_, cycleTime);

	phases_.push_back(temp);

	++number_of_phases_;
	++Phase::PhaseCount;

	if (Settings::DrawAdded)
		cout << "phase " << phaseNumber << " added" << endl;

	return temp;
}

Phase *Cycle::GetPhase(int phaseNumber) {

	for (Phase *p : phases_)
	{
		if (p->GetPhaseNumber() == phaseNumber)
		{
			return p;
		}
	}

	return nullptr;
}

/// calculate the phases priority using neural network
void Cycle::calculate_priority() {

	for (int p = 0; p < number_of_phases_ - 1; p++)
	{
		// get input values
		phases_[p]->GetInputValues(input_values_);

		if(input_values_[0] > 0)
		{
			Net::NeuralNetwork.FeedForward(input_values_);
			Net::NeuralNetwork.GetResults(output_values_);

			phases_[p]->SetPhasePriority(output_values_[0]);	
			phases_[p]->SetCycleTime(clamp(float(output_values_[1]) * Settings::MaxCycleTime, Settings::MinCycleTime, Settings::MaxCycleTime));
		}
		else
		{
			phases_[p]->SetCycleTime(Settings::MinCycleTime);
			phases_[p]->SetPhasePriority(0);
		}
	}
}

/// cycle the phases by the phase array order.
// all the phases but the active one are constantly evaluated
// and sorted by the score they have been given by the NN
// when phase is finished, it gets swapped with the previous one in the array
// to advance and start the next phase in order.
void Cycle::cycle_phases() {
	// if cycle has a minimum of 2 phases
	if (number_of_phases_ >= 2)
	{
		// when current phase is closed, advance to next phase and open it
		if (!phases_.back()->GetIsOpen())
		{
			Phase *backPhase = phases_[number_of_phases_ - 1];
			phases_[number_of_phases_ - 1] = phases_[number_of_phases_ - 2];
			phases_[number_of_phases_ - 2] = backPhase;

			phases_[number_of_phases_ - 1]->Open();

			//Net::NeuralNetwork.printNet();
		}
			// constantly sort the list by their priority score
		else
		{
			// calculate the priority of each phase
			calculate_priority();
			// sort(arr[0:-2])
			partial_sort(phases_.begin(), phases_.end() - 1, phases_.end() - 1, compare_priority);
		}
	}
}

void Cycle::Draw(RenderWindow *window) {

	for (Phase *p : phases_)
	{
		p->Draw(window);
	}
}