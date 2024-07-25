// #pragma once



#include <iostream>
#include <exception>
#include <format>
#include <fstream>

#include "vtk.hpp"
#include "lbm.hpp"
#include "utils.hpp"


int32_t main() {

	std::cout << "Start simulation!" << std::endl;
	ClockT::time_point starTime = ClockT::now();
	const std::uint32_t max_steps = 100;
	LBMArrays out_arr = RunSimulation(max_steps);
	ClockT::time_point endTime = ClockT::now();

	ClockT::duration elapsedTime = endTime - starTime;
	const std::string time = TimeConverter(elapsedTime);
	std::cout << "Finished simulation in " << time  << "!"<< std::endl;


	std::cout << "Exporting Data ..." << std::endl;

	out_arr.ExportArrays(max_steps);


	return 0;
}

