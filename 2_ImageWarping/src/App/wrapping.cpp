#include "Wrapping.h"

CWrapping::CWrapping()
{
}

CWrapping::~CWrapping()
{
}

void CWrapping::Draw(QPainter& paint, bool isdraw) {
	for (auto Line : matching_list) {
		paint.drawLine(Line.first, Line.second);
	}
	if (isdraw) {
		paint.drawLine(start, end);
	}
}

void CWrapping::set_start(QPoint s)
{
	start = s;
}

void CWrapping::set_end(QPoint e)
{
	end = e;
}

bool bound(int x, int L, int R) {
	return x >= L && x <= R;
}

bool bound(QPoint p, int width, int height) {
	return bound(p.rx(), 0, width - 1) && bound(p.ry(), 0, height - 1);
}

void CWrapping::add_point(QPoint image_s, int width, int height) {
	if (!bound(start - image_s, width, height) || !bound(end - image_s, width, height))
		return;
	matching_list.push_back(std::make_pair(start, end));
}