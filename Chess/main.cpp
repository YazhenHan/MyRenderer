#include "head.hpp"

int main() {
	srand(time(0));
	
	std::unordered_map<state, int, decltype(state_hash)*, decltype(state_equal)*> sumA(100, state_hash, state_equal);
	std::unordered_map<state, int, decltype(state_hash)*, decltype(state_equal)*> sumF(100, state_hash, state_equal);
	std::unordered_map<state, int, decltype(state_hash)*, decltype(state_equal)*> sumS(100, state_hash, state_equal);

	std::vector<chess> chesses;
	int cF = 0, cS = 0, cD = 0;
	for (int i = 0; i < 100000; i++) {
		chess c;
		while (!c.isOver())
			c.pushBack();
		for (int j = 0; j < c.size(); j++) {
			if (sumA.find(c[j]) != sumA.end())
				sumA[c[j]]++;
			else
				sumA.insert({ c[j], 1 });
		}
		std::cout << i + 1 << " " << c.size() << " ";
		switch (c.getRes())
		{
		case 1:
			std::cout << "First" << std::endl; cF++;
			for (int j = 0; j < c.size(); j++) {
				if (sumF.find(c[j]) != sumF.end())
					sumF[c[j]]++;
				else
					sumF.insert({ c[j], 1 });
				j++;
				if (sumS.find(c[j]) != sumS.end())
					sumS[c[j]]--;
				else
					sumS.insert({ c[j], -1 });
			}
			break;
		case 2:
			std::cout << "Second" << std::endl; cS++;
			for (int j = 0; j < c.size(); j++) {
				if (sumF.find(c[j]) != sumF.end())
					sumF[c[j]]--;
				else
					sumF.insert({ c[j], -1 });
				j++;
				if (sumS.find(c[j]) != sumS.end())
					sumS[c[j]]++;
				else
					sumS.insert({ c[j], 1 });
			}
			break;
		default:
			std::cout << "Draw" << std::endl; cD++;
			break;
		}
	}
	std::cout << sumF.size() << " " << sumS.size() << std::endl;
	std::cout << cF << " " << cD << " " << cS << std::endl;

	double res = 0.0;
	int res2 = -1;
	for (int i = 0; i < 9; i++) {
		state s(i);
		if ((double)sumF[s] / (double)sumA[s] > res)
		{
			res = (double)sumF[s] / (double)sumA[s];
			res2 = i;
		}
	}
	std::cout << res << " " << res2 << std::endl;
	return 0;
}