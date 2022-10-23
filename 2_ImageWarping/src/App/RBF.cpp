#include "RBF.h"

using namespace Eigen;

CRBF::CRBF()
{
}

CRBF::~CRBF()
{
}

void CRBF::ImageTransfrom(QImage* ptr_image, QPoint start_pos)
{
	if (matching_list.empty()) return;
	size_t n = matching_list.size();
	MatrixXd A(n + 3, n + 3);
	auto sqr = [](int x) {return x * x; };
	auto dis = [sqr](QPoint p, QPoint q) {return sqr(p.rx() - q.rx()) + sqr(p.ry() - q.ry()); };
	int r = 1e9;
	std::vector<QPoint>p, q;
	for (auto x : matching_list) {
		p.push_back(x.second - start_pos);
		q.push_back(x.first - start_pos);
	}
	for (auto x : p)
		for (auto y : p)
			if (x != y)
				r = std::min(r, dis(x, y));
	auto R = [&](QPoint x, int j) {return sqrt((dis(x, p[j]) + r)); };
	for (size_t row = 0; row < n; row++)
		for (size_t col = 0; col <= row; col++)
			A(row, col) = A(col, row) = R(p[row], col);
	for (size_t x = 0; x < n; x++) {
		A(x, n) = A(n, x) = p[x].rx();
		A(x, n + 1) = A(n + 1, x) = p[x].ry();
		A(x, n + 2) = A(n + 2, x) = 1;
	}
	for (size_t x = 0; x < 3; ++x)
		for (size_t y = 0; y < 3; ++y)
			A(n + x, n + y) = 0;

	MatrixXd B(n + 3, 2);
	for (size_t i = 0; i < n; ++i) {
		B(i, 0) = q[i].rx();
		B(i, 1) = q[i].ry();
	}
	for (size_t x = 0; x < 3; ++x)
		B(n + x, 0) = B(n + x, 1) = 0;
	MatrixXd x = A.colPivHouseholderQr().solve(B), cal(1, n + 3), Target(1, 2);
	QImage img_tmp(*ptr_image);
	int width = img_tmp.width();
	int height = img_tmp.height();
	for (int i = 0; i < width; ++i)
		for (int j = 0; j < height; ++j)
		{
			for (size_t k = 0; k < n; ++k)
				cal(0, k) = R(QPoint(i, j), k);
			cal(0, n) = i;
			cal(0, n + 1) = j;
			cal(0, n + 2) = 1;
			Target = cal * x;
			auto bound = [](int x, int L, int R) {return x < L ? L : (x > R ? R : x); };
			int X_target = bound(round(Target(0, 0)), 0, width - 1);
			int Y_target = bound(round(Target(0, 1)), 0, height - 1);
			ptr_image->setPixel(i, j, img_tmp.pixel(X_target, Y_target));
		}
	matching_list.clear();
}

