#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

class state
{
public:
	state() {
		for (int i = 0; i < 9; i++)
			s[i] = 0;
		int r = rand() % 9;
		s[r] = 1;
	}
	state(int c) {
		for (int i = 0; i < 9; i++)
			s[i] = 0;
		s[c] = 1;
	}
	state(state a, int c, int f) {
		for (int i = 0; i < 9; i++)
			s[i] = a[i];
		s[c] = f;
	}

	bool isWin() {
		if (s[0] == s[1] && s[1] == s[2] && s[0] != 0)
			return true;
		if (s[3] == s[4] && s[4] == s[5] && s[3] != 0)
			return true;
		if (s[6] == s[7] && s[7] == s[8] && s[6] != 0)
			return true;
		if (s[0] == s[3] && s[3] == s[6] && s[0] != 0)
			return true;
		if (s[1] == s[4] && s[4] == s[7] && s[1] != 0)
			return true;
		if (s[2] == s[5] && s[5] == s[8] && s[2] != 0)
			return true;
		if (s[0] == s[4] && s[4] == s[8] && s[0] != 0)
			return true;
		if (s[2] == s[4] && s[4] == s[6] && s[2] != 0)
			return true;
		return false;
	}

	int operator[](int i) const {
		return s[i];
	}

private:
	int s[9]; // 0 default 1 first 2 second
};

size_t state_hash(const state& s) {
	int r = 0, temp = 1;
	for (int i = 0; i < 9; i++)
	{
		r += s[i] * temp;
		temp *= 10;
	}
	return std::hash<int>()(r);
}

bool state_equal(const state& s1, const state& s2) {
	for (int i = 0; i < 9; i++)
		if (s1[i] != s2[i])
			return false;
	return true;
}

std::ostream& operator << (std::ostream& os, const state& s) {
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
		cur = 2;
	}

	bool isFull() {
		return ss.size() == 9;
	}

	bool isWin() {
		return s.isWin();
	}

	bool isOver() {
		return isFull() || isWin();
	}

	void pushBack() {
		std::vector<int> t;
		for (int i = 0; i < 9; i++)
			if (s[i] == 0)
				t.push_back(i);
		int r = rand() % t.size();
		ss.push_back(state(s, t[r], cur));
		s = ss.back();
		if (s.isWin())
		{
			res = cur;
			return;
		}
		if (ss.size() == 9)
		{
			res = 0;
			return;
		}
		cur = cur == 1 ? 2 : 1;
	}

	state back() {
		return s;
	}

	state operator[](int i) const {
		return ss[i];
	}

	int size() {
		return ss.size();
	}

	int getRes() {
		return res;
	}
private:
	std::vector<state> ss; // state
	state s; // back
	int cur; // the current next move 1 or 2
	int res; // 0 draw 1 first 2 second
};