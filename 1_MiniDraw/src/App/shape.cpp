#include "Shape.h"

CFigure::CFigure()
{
}

CFigure::~CFigure()
{
}

void CFigure::set_start(QPoint s)
{
	start = s;
}

void CFigure::set_end(QPoint e)
{
	end = e;
}
