#pragma once

#include "wrapping.h"
#include<Eigen/Dense>

class CRBF : public CWrapping
{
public:
	CRBF();
	virtual ~CRBF();
	virtual void ImageTransfrom(QImage* ptr_image, QPoint start_pos);
};

