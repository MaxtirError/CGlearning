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
		MASAP,
		MARAP
	};
	SurfMapping();
	~SurfMapping();
	void SetBoundaryMode(BoundaryMode _status);
	void SetMappingMode(MappingMode _status);
	virtual void Boundary_Mapping() {};
	virtual void Initialize() = 0;
	virtual void Parametrize() = 0;
	virtual void LGAdjusting(int iter_num, double erro) {};
	BoundaryMode bd_status;
	MappingMode mp_status;
protected:
	PolyMesh* m_polymesh;
};

