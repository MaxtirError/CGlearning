#pragma once
#include <Eigen/Sparse>
#include "../PolyMesh/include/PolyMesh/PolyMesh.h"
#include "../TetGene/Tet_generate.h"
#include "../TetGene/tetgen.h"
using namespace acamcad;
using namespace polymesh;
using namespace Eigen;
#define SPRING_K 5000
#define GRAVITY 10
#define FPS 30
typedef SparseMatrix<double> SpMat;
typedef Triplet<double> T;
class SpringSystem
{
public:
	SpringSystem(){};
	~SpringSystem() {};
	void Initialize_Mesh(PolyMesh* _polymesh, std::vector<int>* _LockPoints);
	void Initialize_Tet(tetgenio* tet, std::vector<int>* _LockPoints);
	virtual void Analyze() {};
	virtual void Simulation() = 0;
	//void Simulation_LG();
	void UpdateMesh(PolyMesh* _polymesh);
	void UpdateTet(tetgenio* t_tetmesh);

protected:
	Vector3d Vec(VectorXd &X, int id) {
		return Vector3d(X(id), X(id + nV), X(id + nV * 2));
	}
	void Set_Vec(VectorXd& X, int id, MVector3 p) {
		X(id) = p.x();
		X(id + nV) = p.y();
		X(id + nV * 2) = p.z();
	}

	void Add_Vec(VectorXd& X, int id, Vector3d p) {
		X(id) += p.x();
		X(id + nV) += p.y();
		X(id + nV * 2) += p.z();
	}
	std::vector<int> LockPoints;
	std::vector<std::pair<int, int>>Edges;
	double *Li; //origin length for every edges;
	int nV, nE;
	double H; //time gap
	VectorXd V, X; //speed, pos£¬Y varaible
	SpMat K, KT; //LockPoint Filter;
};

class EulerSimulator : public SpringSystem
{
public:
	EulerSimulator() {};
	~EulerSimulator() {};
	void Simulation();
};

class EnergySimulator : public SpringSystem
{

public:
	EnergySimulator() {};
	~EnergySimulator() {};
	void Simulation();
private:
	void Analyze();
	void LocalPhase();
	void GlobalPhase();
	double Energy;
	SimplicialCholesky<SpMat> global_solver;
	VectorXd G, Y, Xk, Jd, Lock;
	SpMat A;
};