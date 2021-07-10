#pragma once
#include <stack>
#include <queue>

namespace util {
	const char* itoc(unsigned int a) {
		char* ans = (char*)malloc(11 * sizeof(char));
		std::stack<int> s;
		while (a != 0)
		{
			s.push(a % 10);
			a /= 10;
		}
		int k = 0;
		while (!s.empty())
		{
			ans[k++] = s.top() + 48;
			s.pop();
		}
		ans[k] = '\0';
		return ans;
	}

	const char* sctoc(const char* a, const char* b) {
		std::queue<char> q;
		int i = 0;
		while (a[i] != '\0')
			q.push(a[i++]);
		i = 0;
		while (b[i] != '\0')
			q.push(b[i++]);
		char* ans = (char*)malloc(q.size() * sizeof(char));
		i = 0;
		while (!q.empty()) {
			ans[i++] = q.front();
			q.pop();
		}
		return ans;
	}
}