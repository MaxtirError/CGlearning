#pragma once

#include "Shape.h"

class CRect : public CFigure {
public:
	CRect();
	~CRect();

	void Draw(QPainter& painter);
};

