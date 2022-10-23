#pragma once
#include<Eigen/Sparse>
using namespace Eigen;
typedef SparseMatrix<double> SpMat; // declares a column-major sparse matrix type of double
typedef Triplet<double> T;

class SparseSolver {
public:
	SparseSolver(int _m = 0);
	~SparseSolver();
	void SetCoefficient(int i, int j, double v);
	bool isAnalyzed();
	void Analyze();
	MatrixXd solve(MatrixXd);

private:
	int m;
	bool analyzed_Ok;
	std::vector<T> coefficients;            // list of non-zeros coefficients
	SpMat A;
	SparseLU<SpMat, COLAMDOrdering<int> >   solver;
};
