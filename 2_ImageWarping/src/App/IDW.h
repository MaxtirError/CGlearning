#pragma once

#include "wrapping.h"

class CIDW : public CWrapping
{
public:
	CIDW();
	virtual ~CIDW();
	virtual void ImageTransfrom(QImage* ptr_image, QPoint start_pos);
};

