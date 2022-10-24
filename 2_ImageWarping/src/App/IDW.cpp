#include "IDW.h"

CIDW::CIDW()
{
}

CIDW::~CIDW()
{
}

void CIDW::ImageTransfrom(QImage* ptr_image, QPoint start_pos)
{
	if (matching_list.empty()) return;
	QImage img_tmp(*ptr_image);
	int width = img_tmp.width();
	int height = img_tmp.height();
	double weight_sum, dX_target, dY_target;
	bool bis_target;
	for (int i = 0; i < width; ++i)
		for (int j = 0; j < height; ++j)
		{
			weight_sum = dX_target = dY_target = bis_target = 0;
			for (auto mat : matching_list) {
				QPoint q = mat.first - start_pos;
				QPoint p = mat.second - start_pos;
				auto sqr = [](int x) {return double(x * x); };
				if (p.rx() == i && p.ry() == j) {
					bis_target = true;
					ptr_image->setPixel(i, j, img_tmp.pixel(q.rx(), q.ry()));
					break;
				}
				double weight = 1.0 / (sqr(i - p.rx()) + sqr(j - p.ry()));
				weight_sum += weight;
				dX_target += weight * (q.rx() + i - p.rx());
				dY_target += weight * (q.ry() + j - p.ry());
			}
			auto outbound = [](int x, int L, int R) {return x < L || x > R; };
			int X_target = round(dX_target / weight_sum);
			int Y_target = round(dY_target / weight_sum);
			if (outbound(X_target, 0, width - 1) || outbound(Y_target, 0, height - 1))
				ptr_image->setPixel(i, j, qRgb(0, 0, 0));
			else
				ptr_image->setPixel(i, j, img_tmp.pixel(X_target, Y_target));
		}
	matching_list.clear();
}

