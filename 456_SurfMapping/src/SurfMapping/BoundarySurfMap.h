#pragma once
#include "SurfMapping.h"
const double pi = acos(-1.0);
class BoundarySurfMap : public SurfMapping
{
public:
	BoundarySurfMap(PolyMesh* _polymesh = NULL);
	~BoundarySurfMap();
	void SetBoundaryMode(BoundaryMode _status);
	void SetMappingMode(MappingMode _status);
	void Initialize();
	void Boundary_Mapping();
	void Parametrize();
private:
	void Get_boundary();
	BoundaryMode bd_status;
	MappingMode mp_status;
	SparseSolver* UniformSolver;
	SparseSolver* CotSolver;
	bool* is_boundary;
	int* next_id;
	int* mat_id;
	int mat_size;
	MPoint3* PointList;
	double bd_length;
	int bd_start;
	bool isbuilt;
};