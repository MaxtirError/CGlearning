#pragma once
#include "SurfMapping.h"
const double pi = acos(-1.0);
class BoundarySurfMap : public SurfMapping
{
public:
	BoundarySurfMap(PolyMesh* _polymesh = NULL);
	~BoundarySurfMap();
	void Initialize();
	void Boundary_Mapping();
	void Parametrize();
private:
	void Get_boundary();
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