#pragma once
#include"SurfMapping.h"
#include<Eigen/SVD>
class ARAPMapping : public SurfMapping
{
public:
	ARAPMapping(PolyMesh* _polymesh = NULL);
	~ARAPMapping();
	void Initialize();
	void Parametrize();
	void LGAdjusting(int iter_num, double diff_therod);
private:
	void Isometric_Mapping();
	double GetCot(MVector3 A, MVector3 b);
	void VertFunction();
	void TriFunction();
	void Function_Build();
	void LocalPhase();
	void GlobalPhase();
	void BuildGlobalA();
	void UpdateEnergy();
	double EvalueDiff();
	void Normalize();
private:
	SparseSolver* ASAPSolver;
	SparseSolver* ARAPSolver;
	MatrixXd* b;
	std::map<MVert*, Vector2d>* Isometric_x;
	int nV, nT;
	Matrix2d* Lt;
	Vector2d* ut;
	double Energy;

	int Y(int u) { return u + nV; }
	int A(int u) { return u + nV * 2; }
	int B(int u) { return u + nV * 2 + nT; }
};