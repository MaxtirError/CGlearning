#pragma once
#include<Eigen/Sparse>
using namespace Eigen;
typedef SparseMatrix<double> SpMat; // declares a column-major sparse matrix type of double
typedef Triplet<double> T;

class SparseSolver {
public:
	SparseSolver();
	SparseSolver(int _m);
	~SparseSolver();
	void SetCoefficient(int i, int j, int v);
	void SetAnswer(int i, int v);
	void Analyze();
	VectorXd solve();

private:
	int m;
	std::vector<T> coefficients;            // list of non-zeros coefficients
	VectorXd b;                   // the right hand side-vector resulting from the constraints
	SpMat A;
	SparseLU<SpMat, COLAMDOrdering<int> >   solver;
};
