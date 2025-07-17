#include <iostream>
#include <cstdint>
#include <ctime>
#include "maze.h"

int64_t get_integer() {
	int64_t value;
	std::cin >> value;

	if (std::cin.fail()) {
		throw std::invalid_argument("value must be integer.");
	}
	if (value < 2) {
		throw std::invalid_argument("value must be greater than 2.");
	}

	return value;
}

int main() {
	std::srand(time(0));

	try {
		std::cout << "Enter maze length: ";
		int64_t l = get_integer();
		std::cout << "Enter maze width: ";
		int64_t w = get_integer();
		std::cout << std::endl;

		Maze maze(l, w);
		maze.generate_maze();
		maze.print_maze();
	}
	catch (const std::invalid_argument& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}
	catch (...) {
		std::cerr << "Unexpected error occured." << std::endl;
	}
}