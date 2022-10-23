#pragma once
#include "../PolyMesh/include/PolyMesh/PolyMesh.h"
#include "../MatrixSolver/SparseSolver.h"
using namespace acamcad;
using namespace polymesh;
class SurfMapping
{
public:
	enum BoundaryMode
	{
		BNormal,
		BSquare,
		BCircle
	};
	enum MappingMode
	{
		MUniform,
		MCotan,
	};
	SurfMapping();
	~SurfMapping();
	virtual void SetBoundaryMode(BoundaryMode _status) {};
	virtual void SetMappingMode(MappingMode _status) {};
	virtual void Boundary_Mapping() {};
	virtual void Initialize() = 0;
	virtual void Parametrize() = 0;
	virtual void LGAdjusting(int iter_num, double erro) {};
protected:
	PolyMesh* m_polymesh;
};

