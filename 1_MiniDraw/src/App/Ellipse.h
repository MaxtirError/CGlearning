#pragma once

#include "Shape.h"

class CEllipse : public CFigure {
public:
	CEllipse();
	~CEllipse();

	void Draw(QPainter& painter);
};
