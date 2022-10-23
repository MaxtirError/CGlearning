#include "SpringSystem.h"
void SpringSystem::Initialize_Mesh(PolyMesh* t_polymesh, std::vector<int>* _LockPoints)
{
	LockPoints = *_LockPoints;

	nV = t_polymesh->numVertices();
	H = FPS / 1000.0;
	V.resize(nV * 3); X.resize(nV * 3);
	int m = nV - LockPoints.size();
	printf("%d\n", m);
	K.resize(m * 3, nV * 3);
	
	for (int i = 0; i < nV; ++i) {
		Set_Vec(V, i, MVector3(0, 0, 0));
		Set_Vec(X, i, t_polymesh->vertices()[i]->position());
	}
	//transform LockPoint filter K
	bool* islock = new bool[nV]{};
	for (int x : LockPoints)
		islock[x] = true;
	int cur = 0;
	for (int i = 0; i < nV; ++i)
		if (!islock[i]) {
			K.coeffRef(cur, i) = 1;
			K.coeffRef(cur + m, i + nV) = 1;
			K.coeffRef(cur + 2 * m, i + 2 * nV) = 1;
			++cur;
		}
	assert(cur == m);
	delete[] islock;
	KT = K.transpose();

	nE = t_polymesh->numEdges();

	Li = new double[nE];

	for (auto e : t_polymesh->edges())
	{
		MVert* v1 = e->getVert(0), *v2 = e->getVert(1);
		Li[e->index()] = (v1->position() - v2->position()).norm();
		Edges.push_back(std::make_pair(v1->index(), v2->index()));
	}
}

void SpringSystem::Initialize_Tet(tetgenio* t_tetmesh, std::vector<int>* _LockPoints)
{
	LockPoints = *_LockPoints;

	nV = t_tetmesh->numberofpoints;
	H = FPS / 1000.0;
	V.resize(nV * 3); X.resize(nV * 3);
	int m = nV - LockPoints.size();
	printf("%d\n", m);
	K.resize(m * 3, nV * 3);

	const double* P = t_tetmesh->pointlist;

	auto GetVec = [&P](int id) {
		return MVector3(P[id * 3], P[id * 3 + 1], P[id * 3 + 2]);
	};

	for (int i = 0; i < nV; ++i) {
		Set_Vec(V, i, MVector3(0, 0, 0));
		Set_Vec(X, i, GetVec(i));
	}
	//transform LockPoint filter K
	bool* islock = new bool[nV] {};
	for (int x : LockPoints)
		islock[x] = true;
	int cur = 0;
	for (int i = 0; i < nV; ++i)
		if (!islock[i]) {
			K.coeffRef(cur, i) = 1;
			K.coeffRef(cur + m, i + nV) = 1;
			K.coeffRef(cur + 2 * m, i + 2 * nV) = 1;
			++cur;
		}
	assert(cur == m);
	delete[] islock;
	KT = K.transpose();

	nE = t_tetmesh->numberofedges;

	Li = new double[nE];

	const int* Edge = t_tetmesh->edgelist;

	for (int e = 0;e < nE; ++e)
	{
		int v1 = Edge[e << 1], v2 = Edge[e << 1 | 1];
		Li[e] = (GetVec(v1) - GetVec(v2)).norm();
		Edges.push_back(std::make_pair(v1, v2));
		//printf("%.2lf\n", Li[e]);
		//printf("%d %d\n", v1, v2);
	}
}

void SpringSystem::UpdateMesh(PolyMesh* m_polymesh)
{
	for (auto v : m_polymesh->vertices()) {
		int vid = v->index();
		v->setPosition(MPoint3(X(vid), X(vid + nV), X(vid + nV * 2)));
		//std::cout << X(vid) << " " << X(vid + nV) << " " << X(vid + nV * 2) << std::endl;
	}
}

void SpringSystem::UpdateTet(tetgenio *t_tetmesh)
{
	double*& P = t_tetmesh->pointlist;
	for (int i = 0; i < nV; ++i) {
		P[i * 3] = X(i);
		P[i * 3 + 1] = X(i + nV);
		P[i * 3 + 2] = X(i + nV * 2);
		//std::cout << X(vid) << " " << X(vid + nV) << " " << X(vid + nV * 2) << std::endl;
	}
}

void EulerSimulator::Simulation()
{
	VectorXd fex(nV * 3); fex.setZero();
	for (int i = 0; i < nV; ++i)
		fex(i + 2 * nV) = -GRAVITY; //default gravity to Z-axis,M to Identity
	VectorXd Y = X + H * V + H * H * fex;
	VectorXd Xk = Y;
	for (auto x : LockPoints)
	{
		Xk(x) = X(x);
		Xk(x + nV) = X(x + nV);
		Xk(x + nV * 2) = X(x + nV * 2);
	}
	//std::cout<<y<<std::endl;

	SpMat dfin, dgx, kdgx, Ig;
	dfin.resize(nV * 3, nV * 3);
	Ig.resize(nV * 3, nV * 3);
	Ig.setIdentity();
	Matrix3d I;  I.setIdentity();
	while (1) //newtown iteration
	{
		Vector3d dir, f; double length, r;

		VectorXd fin(nV * 3); fin.setZero();
		for (int e = 0; e < nE; ++e) //v1所受弹力f1的贡献
		{
			int x1 = Edges[e].first, x2 = Edges[e].second;
			length = Li[e];
			dir = (Vec(Xk, x2) - Vec(Xk, x1));
			r = dir.norm();
			assert(r >= 1e-9);
			double k = SPRING_K * length;
			f = k * (r - length) * dir / r;
			Add_Vec(fin, x1, f);
			Add_Vec(fin, x2, -f);
		}

		VectorXd gx = K * (Xk - Y - H * H * fin);

		if (gx.norm() <= 1e-9) //GET a result
			break;

		std::vector<T>coeff;

		Matrix3d df;
		for (int e = 0; e < nE; ++e)//v1所受弹力f1的梯度贡献
		{
			auto Fill = [&](int id1, int id2, Matrix3d f)
			{
				for (int i = 0; i < 3; ++i)
					for (int j = 0; j < 3; ++j)
						coeff.push_back(T(id1 + i * nV, id2 + j * nV, f(i, j)));
			};
			int x1 = Edges[e].first, x2 = Edges[e].second;
			length = Li[e];
			//std::cout << "half edge" << " " << x1 << " " << x2 << " " << length << std::endl;
			dir = (Vec(Xk, x2) - Vec(Xk, x1));
			r = dir.norm();
			assert(r >= 1e-9);
			double k = SPRING_K * length;
			df = k * ((length / r - 1) * I - length / (r * r * r) * dir * dir.transpose());
			//std::cout << df << std::endl;
			Fill(x1, x1, df);
			//std::cout << df << std::endl;
			Fill(x1, x2, -df);
			Fill(x2, x1, -df);
			Fill(x2, x2, df);
		}
		dfin.setFromTriplets(coeff.begin(), coeff.end());
		dgx = Ig - H * H * dfin;
		kdgx = K * dgx * KT;
		SimplicialCholesky<SpMat> solver(kdgx);
		assert(solver.info() == Eigen::Success);
		VectorXd dxf = solver.solve(gx);
		Xk = Xk - KT * dxf;
	}
	//printf("total iter_time:%d\n", it_time);
	V = (Xk - X) / H;
	X = Xk;
	//puts("XK");
	//std::cout << Xk << std::endl;
}

void EnergySimulator::Analyze()
{
	SpMat M, L;
	M.resize(nV * 3, nV * 3);
	L.resize(nV * 3, nV * 3);
	std::vector<T>L_coeff;
	M.setIdentity();
	for (int e = 0; e < nE; ++e)
	{
		auto SetCoeff = [&](int x, int y, double v)
		{
			for (int i = 0; i < 3; ++i)
				L_coeff.push_back(T(x + nV * i, y + nV * i, v));
		};
		int x1 = Edges[e].first, x2 = Edges[e].second;
		double length = Li[e];
		double k = SPRING_K * length;
		SetCoeff(x1, x1, k);
		SetCoeff(x1, x2, -k);
		SetCoeff(x2, x1, -k);
		SetCoeff(x2, x2, k);
	}
	L.setFromTriplets(L_coeff.begin(), L_coeff.end());
	//std::cout << L << std::endl;
	A = M + H * H * L;
	//std::cout << A << std::endl;
	global_solver.compute(K * A * KT);
	assert(global_solver.info() == Success);

	Jd.resize(nV * 3);
	G.resize(nV * 3); G.setZero();
	for (int i = 0; i < nV; ++i)
		G(i + 2 * nV) = GRAVITY; //default gravity to Z-axis,M to Identity

	Lock = X - KT * K * X;
}

void EnergySimulator::Simulation()
{
	VectorXd pXk;
	Y = X + H * V - H * H * G;
	Xk = KT * K * Y + Lock;
	for (;;)
	{
		pXk = Xk;
		LocalPhase();
		GlobalPhase();
		double diff = (pXk - Xk).norm();
		//std::cout << X << std::endl;
		if (diff < 1e-4)
			break;
	}
	V = (Xk - X) / H;
	X = Xk;
}

void EnergySimulator::LocalPhase()
{
	Jd.setZero();
	for (int e = 0; e < nE; ++e)
	{
		int x1 = Edges[e].first, x2 = Edges[e].second;
		double length = Li[e];
		double k = SPRING_K * length;
		Vector3d di = Vec(Xk, x1) - Vec(Xk, x2);
		di *= k * length / di.norm();
		Add_Vec(Jd, x1, di);
		Add_Vec(Jd, x2, -di);
	}
}

void EnergySimulator::GlobalPhase()
{
	VectorXd b = K * (H * H * (Jd - G) + Y - A * Lock);
	Xk = KT * global_solver.solve(b) + Lock;
}