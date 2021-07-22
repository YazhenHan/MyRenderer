#pragma once
#include <iostream>
#include <vector>

class state
{
public:
	state() {
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				s[i][j] = 0;
		int r = rand() % 9;
		s[r / 3][r % 3] = 1;
	}

	int& operator[](int i) {
		return s[i / 3][i % 3];
	}

	bool isWin() {
		if (s[0][0] == s[0][1] && s[0][1] == s[0][2])
			return true;
		if (s[1][0] == s[1][1] && s[1][1] == s[1][2])
			return true;
		if (s[2][0] == s[2][1] && s[2][1] == s[2][2])
			return true;
		if (s[0][0] == s[1][0] && s[1][0] == s[2][0])
			return true;
		if (s[0][1] == s[1][1] && s[1][1] == s[2][1])
			return true;
		if (s[0][2] == s[1][2] && s[1][2] == s[2][2])
			return true;
		if (s[0][0] == s[1][1] && s[1][1] == s[2][2])
			return true;
		if (s[0][2] == s[1][1] && s[1][1] == s[2][0])
			return true;
		return false;
	}

private:
	int s[3][3];
};

std::ostream& operator << (std::ostream& os, state s) {
	os << s[0] << " " << s[1] << " " << s[2] << std::endl
		<< s[3] << " " << s[4] << " " << s[5] << std::endl
		<< s[6] << " " << s[7] << " " << s[8] << std::endl;
	return os;
}

class chess {
public:
	
	chess() {
		ss.push_back(state());
		s = ss.back();
		cur = -1;
	}

	state& operator[](int i) {
		return ss[i];
	}

	bool isFull() {
		return ss.size() == 9;
	}

	bool isOver() {
		return isFull() || s.isWin();
	}

	void pushBack() {
		std::vector<int> t;
		for (int i = 0; i < 9; i++)
			if (s[i] == 0)
				t.push_back(i);
		int r = rand() % t.size();
		
	}

	state& back() {
		return ss.back();
	}
private:
	std::vector<state> ss; // store move or state ?
	state s; // back
	int cur; // the current next move
};