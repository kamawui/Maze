#ifndef MAZE_H
#define MAZE_H

#include <cstdint>
#include <vector>
#include <queue>
#include <cstdlib>

constexpr int64_t EMPTY = 0;
constexpr int64_t MAX_TRAPS = 5;
constexpr int64_t MIN_MAZE_AREA_TO_GENERATE_TREASURE = 15;
constexpr int64_t MIN_MAZE_AREA_TO_GENERATE_TRAPS = 20;

struct Cell {
	int64_t set_id = EMPTY;
	bool right_wall = false;
	bool bottom_wall = false;
    bool treasure = false;
	bool trap = false;
};

struct Point {
    int64_t x;
    int64_t y;
    int64_t dist;
};

class Maze {
private:
	int64_t rows;
	int64_t cols;
	std::vector<std::vector<Cell>> maze;
	int64_t set_id = 1;                   // counter to get unique set values for maze cells
	int64_t entrance = -1;                // index of entrance at the top row
	int64_t exit = -1;                    // index of exit at the bottom row
	int64_t traps_amount = 0;

	// return true with 50% probability
	bool fifty_percent_chance() {
		return std::rand() % 2 == 0;
	}

	int64_t random_from_0_to_max(int64_t max) {
		return std::rand() % (max + 1);
	}
public:
	Maze(int64_t length, int64_t width);

	// generates the entire maze using Eller's algorithm
	void generate_maze();

	// randomly chooses entrance and exit columns
	void generate_way_through();

	// assigns unique set_id value for every EMPTY cell of a row 
	void assign_set_values(int64_t row);

	// adds right walls between adjacent cells with 50% chance
	// 1) always adds if they belong to the same set
	// 2) merges two sets if it was decided not to add the wall
	void add_right_walls(int64_t row);

	// for every cell of a row with old_set value changes set_id to new_set value 
	void merge_set(int64_t row, int64_t old_set, int64_t new_set);

	// for every cell of a row with 50% chance decides to add bottom wall
	// 1) if current cell is the only cell in its set bottom wall is not added
	// 2) if current cell is the only cell without bottom wall in its set bottom wall is not added
	void add_bottom_walls(int64_t row);

	// for every cell of the next row after current_row 
	// 1) assigns cell[next_row][i].set_id = cell[current_row][i].set_id]
	// 2) if current_row cell has bottom wall then next row cell with same index will have EMPTY set_id
	void prepare_next_row(int64_t current_row);

	// for every cell of the last row 
	// 1) adds bottom wall
	// 2) if current cell and the next cell don`t have same set_id removes rigth wall between them and merges their sets
	void add_last_row(int64_t row);

	// removes bottom wall at the exit point
	void open_exit();

	void print_maze() const;

	// adds treasure to the most distant from start cell
	void add_treasure();

	// adds traps randomly 
	// there are no traps on the ways to exit and treasure
	void add_traps(int64_t traps_amount);

	// finds distances between entrance cell and every other cell with Lee algorithm
	std::vector<std::vector<int64_t>> compute_distances(int64_t sx, int64_t sy) const;

	// finds the biggest distance found by Lee algorithm
	std::pair<int64_t, int64_t> find_distant_point() const;

	// returns vector of pairs of coordinates to maze[fx][fy]
	std::vector<std::pair<int64_t, int64_t>> find_path(int64_t fx, int64_t fy) const;
};

#endif