#include "SparseSolver.h"
#include<iostream>
SparseSolver::SparseSolver(int _m)
{
	m = _m;
	if(m)
		A.resize(m, m);
	analyzed_Ok = false;
}

SparseSolver::~SparseSolver()
{

}

void SparseSolver::SetCoefficient(int i, int j, double v)
{
	assert(!(i >= m || j >= m || i < 0 || j < 0));
	assert(!(v >= 1e9 || v <= -1e9));
	//printf("%d %d %.2lf\n", i, j, v);
	coefficients.push_back(T(i, j, v));
}

void SparseSolver::Analyze()
{
	A.setFromTriplets(coefficients.begin(), coefficients.end());

	solver.analyzePattern(A);
	solver.factorize(A);
	//printf("%d\n", solver.info());
	assert(solver.info() == Eigen::Success);
	analyzed_Ok = true;
}


bool SparseSolver::isAnalyzed()
{
	return analyzed_Ok;
}

MatrixXd SparseSolver::solve(MatrixXd b)
{
	return solver.solve(b);
}