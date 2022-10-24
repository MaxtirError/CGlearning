#include "SurfMapping.h"
SurfMapping::SurfMapping()
{

}

SurfMapping::~SurfMapping()
{

}


void SurfMapping::SetBoundaryMode(BoundaryMode _status)
{
	bd_status = _status;
}

void SurfMapping::SetMappingMode(MappingMode _status)
{
	mp_status = _status;
}