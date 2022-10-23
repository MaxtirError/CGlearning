#include "Line.h"

CLine::CLine()
{
}

CLine::~CLine()
{
}

void CLine::Draw(QPainter& painter)
{
	painter.drawLine(start, end);
}
