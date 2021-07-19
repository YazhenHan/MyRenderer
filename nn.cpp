#include "head.hpp"


double gauss(double x) {
	return expf(-0.5 * x * x);
}

Eigen::VectorXd a;
Eigen::VectorXd b;
Eigen::VectorXd W;
Eigen::VectorXd B;
void train(const ImVector<ImVec2>& points) {
	int n = points.size();
	// X, Y
	Eigen::VectorXd X(n);
	for (int i = 0; i < n; i++)
		X[i] = points[i].x;
	Eigen::VectorXd Y(n);
	for (int i = 0; i < n; i++)
		Y[i] = points[i].y;
	printf("%f\n", Y.sum() / Y.size());

	// A
	a.resize(n);
	for (int i = 0; i < n; i++)
		a[i] = rand() / float(RAND_MAX) * 2 - 1;
	b.resize(n);
	for (int i = 0; i < n; i++)
		b[i] = rand() / float(RAND_MAX) * 2 - 1;

	// W
	W.resize(n);
	for (int i = 0; i < n; i++)
		W[i] = rand() / float(RAND_MAX) * 2 - 1;
	// B
	B.resize(1);
	B[0] = 0.0;

	int loops = 1000;
	double lr = 0.01;
	for (int loop = 0; loop < loops; loop++)
	{
		Eigen::VectorXd X1(2);
		X1[0] = points[loop % n].x;
		X1[1] = 1.0;

		// A
		Eigen::VectorXd S = X1[0] * a + X1[1] * b;
		Eigen::VectorXd SS = S;
		for (int i = 0; i < n; i++)
			SS[i] = gauss(S[i]);
		// W, B
		double fx = (SS * W + B)[0]; 
		double fy = points[loop % n].y;

		// loss
		double loss = 0.5 * (fx - fy) * (fx - fy);
		if ((loop + 1) % 10 == 0)
		printf("%d\t%f\t%f\t%f\t%f\n", loop + 1, X1[0], fx, fy, loss);

		Eigen::VectorXd kw = (fx - fy) * SS;
		Eigen::VectorXd k0(1); k0[0] = (fx - fy);
		Eigen::VectorXd ka = (-(fx - fy) * X1[0]) * W.cwiseProduct(SS).cwiseProduct(S);
		Eigen::VectorXd kb = (-(fx - fy)) * W.cwiseProduct(SS).cwiseProduct(S);

		W -= lr * kw;
		B -= lr * k0;
		a -= lr * ka;
		b -= lr * kb;
	}

}

int main() {
	srand(time(0));
	ImVector<ImVec2> t;
	t.push_back(ImVec2(0.097, 0.186));
	t.push_back(ImVec2(0.159, 0.107));
	t.push_back(ImVec2(0.257, 0.107));
	t.push_back(ImVec2(0.334, 0.207));
	t.push_back(ImVec2(0.410, 0.381));
	t.push_back(ImVec2(0.524, 0.466));
	t.push_back(ImVec2(0.656, 0.445));
	t.push_back(ImVec2(0.770, 0.354));
	train(t);
	Eigen::VectorXd tt(2);
	tt[0] = 0.097;
	tt[1] = 1.0;
	Eigen::VectorXd r1 = tt[0] * a + tt[1] * b;
	for (int i = 0; i < W.size(); i++)
		r1[i] = gauss(r1[i]);
	auto r = (r1 * W)[0] + B[0];
	std::cout << r << std::endl;
}