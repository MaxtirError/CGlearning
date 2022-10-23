#include "Ellipse.h"

CEllipse::CEllipse()
{
}

CEllipse::~CEllipse()
{
}

void CEllipse::Draw(QPainter& painter)
{
	painter.drawEllipse(start.x(), start.y(),
		end.x() - start.x(), end.y() - start.y());
}
