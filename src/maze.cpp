#include "maze.h"
#include <iostream>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <algorithm>

Maze::Maze(int64_t length, int64_t width) : rows(length), cols(width) {
	if (length > 0 && width > 0) {
		maze.resize(length);
		for (int64_t i = 0; i < length; i++) {
			maze[i].resize(width);
		}
	}
}

void Maze::generate_maze() {
	generate_way_through();

	for (int64_t row = 0; row < rows; row++) {
		assign_set_values(row);
		add_right_walls(row);
		add_bottom_walls(row);

		if (row == rows - 1) {
			add_last_row(row);
		}
		else {
			prepare_next_row(row);
		}
	}

	if (rows * cols >= MIN_MAZE_AREA_TO_GENERATE_TREASURE) {
		add_treasure();
	}

	if (rows * cols >= MIN_MAZE_AREA_TO_GENERATE_TRAPS) {
		traps_amount = random_from_0_to_max(MAX_TRAPS);

		add_traps(traps_amount);
	}

	open_exit();
}

void Maze::generate_way_through() {
	entrance = rand() % cols;
	exit = rand() % cols;
	entrance = std::min(entrance, cols - 1);
	exit = std::min(exit, cols - 1);
}

void Maze::assign_set_values(int64_t row) {
	for (Cell& cell : maze[row]) {
		if (cell.set_id == EMPTY) {
			cell.set_id = set_id++;
		}
	}
}

void Maze::add_right_walls(int64_t row) {
	for (int64_t i = 0; i < cols - 1; i++) {
		Cell& current_cell = maze[row][i];
		Cell& right_cell = maze[row][i + 1];

		if (current_cell.set_id == right_cell.set_id) {
			current_cell.right_wall = true;
		}
		else {
			bool add_wall = fifty_percent_chance();

			if (add_wall) {
				current_cell.right_wall = true;
			}
			else {
				merge_set(row, right_cell.set_id, current_cell.set_id);
			}
		}
	}

	maze[row][cols - 1].right_wall = true;
}

void Maze::merge_set(int64_t row, int64_t old_set, int64_t new_set) {
	for (Cell& cell : maze[row]) {
		if (cell.set_id == old_set) {
			cell.set_id = new_set;
		}
	}
}

void Maze::add_bottom_walls(int64_t row) {
	std::unordered_map<int64_t, int64_t> open_bottom_walls;

	for (int64_t i = 0; i < cols; i++) {
		const Cell& cell = maze[row][i];
		if (!cell.bottom_wall) {
			open_bottom_walls[cell.set_id]++;
		}
	}

	for (int64_t i = 0; i < cols; i++) {
		Cell& cell = maze[row][i];

		if (open_bottom_walls[cell.set_id] > 1 && fifty_percent_chance()) {
			cell.bottom_wall = true;
			open_bottom_walls[cell.set_id]--;
		}
	}
}

void Maze::prepare_next_row(int64_t current_row) {
	for (int64_t i = 0; i < cols; i++) {
		Cell& cell = maze[current_row][i];
		Cell& next_row_cell = maze[current_row + 1][i];

		if (!cell.bottom_wall) {
			next_row_cell.set_id = cell.set_id;
		}
		else {
			next_row_cell.set_id = EMPTY;
		}
	}
}

void Maze::add_last_row(int64_t row) {
	for (int64_t i = 0; i < cols - 1; i++) {
		Cell& current_cell = maze[row][i];
		Cell& right_cell = maze[row][i + 1];

		if (current_cell.set_id != right_cell.set_id) {
			current_cell.right_wall = false;
			merge_set(row, right_cell.set_id, current_cell.set_id);
		}

		current_cell.bottom_wall = true;
	}

	maze[row][cols - 1].bottom_wall = true;
}

void Maze::open_exit() {
	maze[rows - 1][exit].bottom_wall = false;
}

void Maze::print_maze() const {
	for (int64_t col = 0; col < cols; col++) {
		if (col == entrance)
			std::cout << " S";
		else
			std::cout << " __";
	}
	std::cout << "\n";

	for (int64_t row = 0; row < rows; row++) {
		for (int64_t col = 0; col < cols; col++) {
			const Cell& c = maze[row][col];
			if (col == 0) 
				std::cout << "|";

			std::string cellContent = "  ";
			if (c.treasure) {
				cellContent = "\033[43m T\033[0m";
			}

			if (c.trap) {
				cellContent = "\033[41m  \033[0m";
			}

			if (c.right_wall) {
				std::cout << cellContent << "|";
			}
			else {
				std::cout << cellContent << " ";
			}
		}
		std::cout << "\n";

		for (int64_t col = 0; col < cols; col++) {
			const Cell& c = maze[row][col];
			if (col == 0) std::cout << "|";

			if (row == rows - 1 && col == exit) {
				std::cout << "\033[42m F\033[0m";
			}
			else if (c.bottom_wall) {
				std::cout << "__";
			}
			else {
				std::cout << "  ";
			}

			std::cout << "|";
		}
		std::cout << "\n";
	}
}

std::vector<std::vector<int64_t>> Maze::compute_distances(int64_t sx, int64_t sy) const {
	std::vector<std::vector<int64_t>> dist(rows, std::vector<int64_t>(cols, -1));
	std::queue<Point> q;

	dist[sx][sy] = 0;
	q.push(Point{ sx, sy, 0 });

	int64_t dx[4] = { 1, -1, 0, 0 };
	int64_t dy[4] = { 0, 0, 1, -1 };

	while (!q.empty()) {
		Point p = q.front(); 
		q.pop();
		int64_t x = p.x, y = p.y, d = p.dist;

		for (int i = 0; i < 4; ++i) {
			int64_t nx = x + dx[i], ny = y + dy[i];
			if (nx < 0 || ny < 0 || nx >= rows || ny >= cols)
				continue;

			bool blocked = false;

			if (nx == x && ny == y + 1)      
				blocked = maze[x][y].right_wall;
			else if (nx == x && ny == y - 1) 
				blocked = maze[x][y - 1].right_wall;
			else if (nx == x + 1 && ny == y) 
				blocked = maze[x][y].bottom_wall;
			else if (nx == x - 1 && ny == y) 
				blocked = maze[x - 1][y].bottom_wall;

			if (blocked || dist[nx][ny] != -1)
				continue;

			dist[nx][ny] = d + 1;
			q.push(Point{ nx, ny, d + 1 });
		}
	}

	return dist;
}

std::pair<int64_t, int64_t> Maze::find_distant_point() const {
	auto distances = compute_distances(0, entrance);
	int64_t best = -1;
	int64_t best_x = -1;
	int64_t best_y = -1;

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			if (distances[i][j] > best) {
				best = distances[i][j];
				best_x = i;
				best_y = j;
			}
		}
	}

	return { best_x, best_y };
}

std::vector<std::pair<int64_t, int64_t>> Maze::find_path(int64_t fx, int64_t fy) const {
	std::vector<std::pair<int64_t, int64_t>> path;
	if (fx < 0 || fx >= rows || fy < 0 || fy >= cols)
		return path;

	auto distances = compute_distances(0, entrance);
	if (distances[fx][fy] < 0)
		return path;

	int64_t x = fx, y = fy, d = distances[fx][fy];
	path.emplace_back(x, y);

	int64_t dx[4] = { 1, -1, 0, 0 };
	int64_t dy[4] = { 0, 0, 1, -1 };

	while (d > 0) {
		for (int i = 0; i < 4; ++i) {
			int64_t nx = x + dx[i], ny = y + dy[i];
			if (nx < 0 || ny < 0 || nx >= rows || ny >= cols)
				continue;

			bool blocked = false;
			if (nx == x && ny == y + 1)      
				blocked = maze[x][y].right_wall;
			else if (nx == x && ny == y - 1) 
				blocked = maze[x][y - 1].right_wall;
			else if (nx == x + 1 && ny == y) 
				blocked = maze[x][y].bottom_wall;
			else if (nx == x - 1 && ny == y) 
				blocked = maze[x - 1][y].bottom_wall;

			if (!blocked && distances[nx][ny] == d - 1) {
				x = nx;
				y = ny;
				d--;
				path.emplace_back(x, y);
				break;
			}
		}
	}

	std::reverse(path.begin(), path.end());
	return path;
}

void Maze::add_treasure() {
	std::pair<int64_t, int64_t> distant_point = find_distant_point();

	if (distant_point.first >= 0 && distant_point.second >= 0)
		maze[distant_point.first][distant_point.second].treasure = true;
}

void Maze::add_traps(int64_t traps_amount) {
	auto path_to_finish = find_path(rows - 1, exit);
	std::pair<int64_t, int64_t> treasure_point = find_distant_point();
	auto path_to_treasure = find_path(treasure_point.first, treasure_point.second);

	std::vector<std::vector<bool>> forbidden(rows, std::vector<bool>(cols, false));

	for (auto [i, j] : path_to_finish)
		forbidden[i][j] = true;
	for (auto [i, j] : path_to_treasure)
		forbidden[i][j] = true;

	for (int i = 0; i < traps_amount; i++) {
		while (true) {
			int64_t random_row = random_from_0_to_max(rows - 1);
			int64_t random_col = random_from_0_to_max(cols - 1);
			Cell& random_cell = maze[random_row][random_col];

			if (!random_cell.treasure && !random_cell.trap
				&& !(random_row == 0 && random_col == entrance)
				&& !(random_row == rows - 1 && random_col == exit)
				&& !forbidden[random_row][random_col]) {
				random_cell.trap = true;
				break;
			}
		}
	}
}