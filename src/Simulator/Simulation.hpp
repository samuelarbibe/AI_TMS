//
// Created by Samuel Arbibe on 03/03/2020.
//

#ifndef SIMULATORSFML_SRC_SIMULATOR_SIMULATION_HPP
#define SIMULATORSFML_SRC_SIMULATOR_SIMULATION_HPP

#include <iostream>
#include <fstream>
#include <list>
#include <ctime>

#include <SFML/Graphics.hpp>
#include <QtWidgets>
#include <nlohmann/json.hpp>
#include "Settings.hpp"
#include "Vehicle.hpp"


using namespace sf;
using json = nlohmann::json;

class Simulation
{
  public:

	Simulation(int simulationNumber, int vehicleCount);
	~Simulation();

	void Update(float elapsedTime);

	int GetSimulationNumber(){return simulation_number_;}
	int GetVehicleCount(){return vehicle_count_;}
	int GetCurrentVehicleCount(){return current_vehicle_count_;}
	int IsFinished(){return finished_;}
	int IsRunning(){return running_;}

	tm * GetStartTime(){return start_time_;}
	tm * GetEndTime(){return end_time_;}
	float GetElapsedTime(){return elapsed_time_;}

	static int SimulationCount;

  private:

	int simulation_number_;
    int vehicle_count_;
    int current_vehicle_count_;
    bool finished_;
    bool running_;

    tm *  start_time_;
    tm *  end_time_;
    float elapsed_time_;
};

#endif //SIMULATORSFML_SRC_SIMULATOR_SIMULATION_HPP