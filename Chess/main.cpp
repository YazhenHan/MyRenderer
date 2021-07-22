#include "head.hpp"

int main() {
	srand(time(0));

	chess c;
	state s = c.back();
	std::cout << s << std::endl;
	s[0] = 3;
	std::cout << s << std::endl;


	/*std::vector<chess> chesses;
	for (int i = 0; i < 1000; i++) {
		chess c;
		while (!c.isOver())
		{
			
		}
	}*/
	return 0;
}