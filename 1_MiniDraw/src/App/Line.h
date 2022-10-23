#pragma once

#include "Shape.h"

class CLine : public CFigure {
public:
	CLine();
	~CLine();

	void Draw(QPainter& painter);
};
