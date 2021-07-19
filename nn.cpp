#include "head.hpp"


float gauss(float x) {
	return expf(-0.5 * x * x);
}

Eigen::MatrixXf A;
Eigen::VectorXf W;
Eigen::VectorXf B;
void train(const ImVector<ImVec2>& points) {
	int n = points.size();
	// X, Y
	Eigen::VectorXf X(n);
	for (int i = 0; i < n; i++)
		X[i] = points[i].x;
	Eigen::VectorXf Y(n);
	for (int i = 0; i < n; i++)
		Y[i] = points[i].y;
	printf("%f\n", Y.sum() / Y.size());

	// X1
	Eigen::MatrixXf X1(n, 2);
	for (int i = 0; i < n; i++)
	{
		X1(i, 0) = points[i].x;
		X1(i, 1) = 1.0;
	}

	// A
	A.resize(2, 3);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 3; j++)
			A(i, j) = rand() / float(RAND_MAX) * 2 - 1;
	// W
	W.resize(3);
	for (int i = 0; i < 3; i++)
		W[i] = rand() / float(RAND_MAX) * 2 - 1;
	// B
	B.resize(n);
	for (int i = 0; i < n; i++)
		B[i] = 0.0;

	int loops = 100;
	float lr = 0.01;
	for (int loop = 0; loop < loops; loop++)
	{
		// A
		Eigen::MatrixXf S = X1 * A;
		
		Eigen::VectorXf fs(3);
		for (int i = 0; i < 3; i++)
		{
			fs[i] = 0.0;
			for (int j = 0; j < n; j++)
				fs[i] += S(j, i);
			fs[i] /= n;
		}

		// Gauss
		for (int i = 0; i < n; i++)
			for (int j = 0; j < 3; j++)
				S(i, j) = gauss(S(i, j));

		Eigen::VectorXf fss(3);
		for (int i = 0; i < 3; i++)
		{
			fss[i] = 0.0;
			for (int j = 0; j < n; j++)
				fss[i] += S(j, i);
			fss[i] /= n;
		}

		// W, B
		Eigen::VectorXf SS = S * W + B; 
		float fx = SS.sum() / SS.size();
		float fy = Y.sum() / Y.size();

		// loss
		float loss = 0.0;
		for (int i = 0; i < n; i++)
			loss += 0.5 * (SS[i] - Y[i]) * (SS[i] - Y[i]);
		loss /= n;
		//if (loop % 100 == 0)
			printf("%d\t%f\t%f\n", loop, fx, loss);

		Eigen::VectorXf kw = (fx - fy) * fss;
		Eigen::VectorXf k0(n);
		for (int i = 0; i < n; i++)
			k0[i] = (fx - fy);

		Eigen::VectorXf ka = (-(fx - fy)) * W.cwiseProduct(fss).cwiseProduct(fs).cwiseProduct(X);
		Eigen::VectorXf kb = (-(fx - fy)) * W.cwiseProduct(fss).cwiseProduct(fs);

		W -= lr * kw;
		B -= lr * k0;

		Eigen::VectorXf ta(3);
		Eigen::VectorXf tb(3);
		for (int i = 0; i < 3; i++)
		{
			ta[i] = A(0, i);
			tb[i] = A(1, i);
		}
		ta -= lr * ka;
		tb -= lr * kb;
		for (int i = 0; i < 3; i++)
		{
			A(0, i) = ta[i];
			A(1, i) = tb[i];
		}
	}

}

int main() {
	//srand(time(0));
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
	Eigen::VectorXf tt(2);
	tt[0] = 0.41;
	tt[1] = 1.0;
	Eigen::VectorXf r1 = tt * A;
	for (int i = 0; i < W.size(); i++)
		r1[i] = gauss(r1[i]);
	auto r = (r1 * W)[0] + B[0];
	std::cout << r << std::endl;
}