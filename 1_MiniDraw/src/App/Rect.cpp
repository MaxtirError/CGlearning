#include "Rect.h"

CRect::CRect()
{
}

CRect::~CRect()
{
}

void CRect::Draw(QPainter& painter)
{
	painter.drawRect(start.x(), start.y(),
		end.x() - start.x(), end.y() - start.y());
}
