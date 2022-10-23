#include "SparseSolver.h"
SparseSolver::SparseSolver()
{
	m = 0;
}
SparseSolver::SparseSolver(int _m)
{
	m = _m;
	b.resize(m);
	A.resize(m, m);
}

SparseSolver::~SparseSolver()
{

}

void SparseSolver::SetCoefficient(int i, int j, int v)
{
	coefficients.push_back(T(i, j, v));
}

void SparseSolver::SetAnswer(int i, int v)
{
	b[i] = v;
}

void SparseSolver::Analyze()
{
	A.setFromTriplets(coefficients.begin(), coefficients.end());
	solver.analyzePattern(A);
	solver.factorize(A);
}

VectorXd SparseSolver::solve()
{
	return solver.solve(b);
}