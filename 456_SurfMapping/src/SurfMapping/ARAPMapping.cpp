#include "ARAPMapping.h"

ARAPMapping::ARAPMapping(PolyMesh* _polymesh)
{
	m_polymesh = _polymesh;
	ASAPSolver = NULL;
	ARAPSolver = NULL;
	Lt = NULL;
	ut = NULL;
	if (m_polymesh != NULL)
	{
		nV = m_polymesh->numVertices();
		nT = m_polymesh->numPolygons();
		assert(m_polymesh->isTriangleMesh());
	}
	b = NULL;
	Energy = 1e18;
}

ARAPMapping::~ARAPMapping()
{
	if (m_polymesh != NULL)
		delete m_polymesh;
	if (ASAPSolver != NULL)
		delete ASAPSolver;
	if (b != NULL)
		delete b;
	if (Lt != NULL)
		delete[] Lt;
	if (ut != NULL)
		delete[] ut;
}

void ARAPMapping::Initialize()
{
	Isometric_Mapping();
	Function_Build();
}


void ARAPMapping::Parametrize()
{
	auto Answer = ASAPSolver->solve(*b);
	for (auto v : m_polymesh->vertices())
	{
		int vid = v->index();
		v->setTexture(Answer(vid), Answer(Y(vid)));
	}
}

void ARAPMapping::LGAdjusting(int iter_num, double diff_therod)
{
	printf("Start adjusting\n");
	ut = new Vector2d[nV];
	Lt = new Matrix2d[nT];

	for (auto v : m_polymesh->vertices())  // initialize ut
	{
		int vid = v->index();
		ut[vid] = Vector2d(v->getTexture().uv[0], v->getTexture().uv[1]);
	}

	BuildGlobalA();

	int iter = 0;
	for (; iter < iter_num; ++iter)
	{
		printf("iter_num:%d--------------------------\n", iter);
		LocalPhase();
		GlobalPhase();
		double diff = EvalueDiff();
		printf("Energy_info:%.2lf\n", Energy);
		if (diff < diff_therod)
		{
			printf("iteration end up in %d times\n", iter);
			break;
		}
	}
	if (iter == iter_num)
		printf("cannot adjust to diff_therod, try enlarge iter number\n");

	Normalize();

	for (auto v : m_polymesh->vertices())  // load ut
	{
		int vid = v->index();
		v->setTexture(ut[vid][0], ut[vid][1]);
		//printf("%.2lf %.2lf\n", ut[vid][0], ut[vid][1]);
	}
}


void ARAPMapping::Isometric_Mapping() 
//Get an 2D isometric_mapping, default first point of a face's mapping is (0,0)
{
	Isometric_x = new std::map<MVert*, Vector2d> [nT];
	for (MPolyFace* tri : m_polymesh->polyfaces())
		if(tri != NULL)
		{
			int tri_id = tri->index();
			//printf("%d\n", tri_id);
			auto Verts = m_polymesh->polygonVertices(tri);
			MVector3 a(Verts[0]->position()), b(Verts[1]->position()), c(Verts[2]->position());
			double angle = vectorAngle(b - a, c - a);
			double clength = (c - a).norm();
			Isometric_x[tri_id][Verts[0]] = Vector2d(0, 0);
			Isometric_x[tri_id][Verts[1]] = Vector2d((a - b).norm(), 0);
			Isometric_x[tri_id][Verts[2]] = Vector2d(clength * cos(angle), clength * sin(angle));
		}
}

void ARAPMapping::Function_Build()
//variables for every vertices include: ux,id:[0,nV-1], uy,id:[nV, 2nV-1]
//variables for every triangles include: a[2nV,2nV+nT-1], b[2nV+nT,2nV+2nT-1]
//total variable:2(nV + nT)(also Function Scale)
{
	ASAPSolver = new SparseSolver(2 * (nV + nT));
	b = new MatrixXd(2 * (nV + nT), 1);
	b->setZero();

	VertFunction();
	TriFunction();
	ASAPSolver->Analyze();
}

double ARAPMapping::GetCot(MVector3 a, MVector3 b)
{
	double angle = vectorAngle(a, b);
	return cos(angle) / sin(angle);
}

void ARAPMapping::VertFunction()
//function for every vertices
{
	//printf("test cot:%.2lf\n", GetCot(MVector3(-1, 1, 0), MVector3(1, 0, 0)));
	auto boundary = m_polymesh->boundaryVertices();
	//anchor vertices, set v0->(0,0), v1->(1,1)
	MVert* v0 = boundary[0];
	MVert* v1 = boundary[boundary.size() / 2];
	int v0_id = v0->index(), v1_id = v1->index();
	ASAPSolver->SetCoefficient(v0_id, v0_id, 1); (*b)(v0_id) = 0;
	ASAPSolver->SetCoefficient(Y(v0_id), Y(v0_id), 1); (*b)(Y(v0_id)) = 0;
	ASAPSolver->SetCoefficient(v1_id, v1_id, 1); (*b)(v1_id) = 1;
	ASAPSolver->SetCoefficient(Y(v1_id), Y(v1_id), 1); (*b)(Y(v1_id)) = 1;
	for (MVert* u : m_polymesh->vertices())
		if (u->index() != v0_id && u->index() != v1_id) //skipp anchor vertices
		{
			int uid = u->index();
			double cotsum = 0;
			for (MVert* v : m_polymesh->vertAdjacentVertices(u))
			{
				int vid = v->index();
				double cot12 = 0;

				auto buildfunction = [&](MHalfedge* he)
				{
					MPolyFace* tri = he->polygon();
					if (tri != NULL)
					{
						int tri_id = tri->index();
						MVert* w = he->next()->toVertex();
						//printf("%d %d %d %d\n", u->index(), v->index(), w->index(), tri_id);
						double cot = GetCot(u->position() - w->position(), v->position() - w->position());
						//printf("%.2lf\n", cot);
						cot12 += cot;
						auto x = Isometric_x[tri_id];
						//printf("x:%.2lf %.2lf\n", x[u][0] - x[v][0], x[u][1] - x[v][1]);
						ASAPSolver->SetCoefficient(uid, A(tri_id), -cot * (x[u][0] - x[v][0])); //ux and a
						ASAPSolver->SetCoefficient(uid, B(tri_id), -cot * (x[u][1] - x[v][1])); //ux and b
						ASAPSolver->SetCoefficient(Y(uid), B(tri_id), cot * (x[u][0] - x[v][0])); //uy and b
						ASAPSolver->SetCoefficient(Y(uid), A(tri_id), -cot * (x[u][1] - x[v][1])); //uy and b
					}
				};
				MHalfedge* he = m_polymesh->halfedgeBetween(u, v);
				buildfunction(he);
				he = he->pair();
				buildfunction(he);
				//printf("%.2lf\n", cot12);
				ASAPSolver->SetCoefficient(uid, vid, -cot12);
				ASAPSolver->SetCoefficient(Y(uid), Y(vid), -cot12);
				cotsum += cot12;
			}
			ASAPSolver->SetCoefficient(uid, uid, cotsum);
			ASAPSolver->SetCoefficient(Y(uid), Y(uid), cotsum);
		}
}


void ARAPMapping::TriFunction()
//function for every triangle;
{
	for (auto tri : m_polymesh->polyfaces())
		if(tri != NULL)
		{
			int tri_id = tri->index();
			auto V = m_polymesh->polygonVertices(tri);
			int vid[3];
			for (int k = 0; k < 3; ++k)
				vid[k] = V[k]->index();
			double cot[3];
			for (int k = 0; k < 3; ++k)
				cot[k] = GetCot(V[k]->position() - V[(k + 2) % 3]->position(), V[(k + 1) % 3]->position() - V[(k + 2) % 3]->position());
			auto x = Isometric_x[tri_id];
			double dx[3], dy[3];

			double denom = 0; //denominator is cot * |x|^2
			for (int k = 0; k < 3; ++k)
			{
				dx[k] = x[V[k]][0] - x[V[(k + 1) % 3]][0];
				dy[k] = x[V[k]][1] - x[V[(k + 1) % 3]][1];
				denom += cot[k] * (dx[k] * dx[k] + dy[k] * dy[k]);
			}

			ASAPSolver->SetCoefficient(A(tri_id), A(tri_id), denom);
			ASAPSolver->SetCoefficient(B(tri_id), B(tri_id), denom);

			for (int k = 0; k < 3; ++k)
			{
				int pk = (k + 2) % 3;
				ASAPSolver->SetCoefficient(A(tri_id), vid[k], -(cot[k] * dx[k] - cot[pk] * dx[pk])); //a and x
				ASAPSolver->SetCoefficient(A(tri_id), Y(vid[k]), -(cot[k] * dy[k] - cot[pk] * dy[pk])); //a and y
				ASAPSolver->SetCoefficient(B(tri_id), vid[k], -(cot[k] * dy[k] - cot[pk] * dy[pk])); //b and x
				ASAPSolver->SetCoefficient(B(tri_id), Y(vid[k]), (cot[k] * dx[k] - cot[pk] * dx[pk])); //b and y
			}
		}
}

void ARAPMapping::BuildGlobalA()
//build the global function A
{
	ARAPSolver = new SparseSolver(nV);
	for (MVert* u : m_polymesh->vertices())
	{
		int uid = u->index();
		double cotsum = 0;

		for (MVert* v : m_polymesh->vertAdjacentVertices(u))
		{
			int vid = v->index();
			double cot12 = 0;

			auto CaculateCot = [&](MHalfedge* he)
			{
				MPolyFace* tri = he->polygon();
				if (tri != NULL)
				{
					int tri_id = tri->index();
					MVert* w = he->next()->toVertex();
					double cot = GetCot(u->position() - w->position(), v->position() - w->position());
					cot12 += cot;
				}
			};
			MHalfedge* he = m_polymesh->halfedgeBetween(u, v);
			CaculateCot(he);
			he = he->pair();
			CaculateCot(he);
			//printf("%.2lf\n", cot12);
			ARAPSolver->SetCoefficient(uid, vid, -cot12);
			cotsum += cot12;
		}
		ARAPSolver->SetCoefficient(uid, uid, cotsum);
	}
	ARAPSolver->Analyze();
}

void ARAPMapping::LocalPhase()
//fix ut, find best Lt, according to SVD method
{
	Matrix2d St; 
	Vector2d Du, Dx;
	for (auto tri : m_polymesh->polyfaces())
	{
		int tri_id = tri->index();
		auto V = m_polymesh->polygonVertices(tri);
		auto x = Isometric_x[tri_id];
		St.setZero();
		for (int k = 0, nk, pk; k < 3; ++k)
		{
			nk = (k + 1) % 3, pk = (k + 2) % 3;
			double cot = GetCot(V[k]->position() - V[pk]->position(), V[nk]->position() - V[pk]->position());
			Du = ut[V[k]->index()] - ut[V[nk]->index()];
			Dx = x[V[k]] - x[V[nk]];
			St += cot * Du * Dx.transpose();
			//std::cout << Du<< std::endl << Dx << std::endl;
		}
		JacobiSVD<MatrixXd> svd(St, ComputeThinU | ComputeThinV);
		Matrix2d matU = svd.matrixU(), matV = svd.matrixV();
		Lt[tri_id] = matU * matV.transpose();
	}
}

void ARAPMapping::GlobalPhase()
//fix Lt, find best ut, according to sqr function minmize
{
	assert(ARAPSolver->isAnalyzed()); //solver must be built
	MatrixXd b(nV, 2);
	for (MVert* u : m_polymesh->vertices())
	{
		int uid = u->index();
		Vector2d bt;
		bt.setZero();
		for (MVert* v : m_polymesh->vertAdjacentVertices(u))
		{
			int vid = v->index();
			auto CaculateAnswer = [&](MHalfedge* he)
			{
				MPolyFace* tri = he->polygon();
				if (tri != NULL)
				{
					int tri_id = tri->index();
					MVert* w = he->next()->toVertex();
					double cot = GetCot(u->position() - w->position(), v->position() - w->position());
					auto x = Isometric_x[tri_id];
					bt += cot * Lt[tri_id] * (x[u] - x[v]);
				}
			};
			MHalfedge* he = m_polymesh->halfedgeBetween(u, v);
			CaculateAnswer(he);
			he = he->pair();
			CaculateAnswer(he);
		}

		b(uid, 0) = bt[0];
		b(uid, 1) = bt[1];
	}
	auto Answer = ARAPSolver->solve(b);
	for (MVert* u : m_polymesh->vertices())
	{
		int uid = u->index();
		ut[uid] = Vector2d(Answer(uid, 0), Answer(uid, 1));
	}
}

void ARAPMapping::UpdateEnergy()
{
	Energy = 0;
	for (MVert* u : m_polymesh->vertices())
	{
		int uid = u->index();
		for (MVert* v : m_polymesh->vertAdjacentVertices(u))
		{
			int vid = v->index();
			auto CaculateEnergy = [&](MHalfedge* he)
			{
				MPolyFace* tri = he->polygon();
				if (tri != NULL)
				{
					int tri_id = tri->index();
					MVert* w = he->next()->toVertex();
					double cot = GetCot(u->position() - w->position(), v->position() - w->position());
					auto x = Isometric_x[tri_id];
					Energy += cot * (ut[uid] - ut[vid] - Lt[tri_id] * (x[u] - x[v])).squaredNorm();
				}
			};
			MHalfedge* he = m_polymesh->halfedgeBetween(u, v);
			CaculateEnergy(he);
			he = he->pair();
			CaculateEnergy(he);
		}
	}
}


double ARAPMapping::EvalueDiff()
{
	double pre_Energy = Energy;
	UpdateEnergy();
	return (pre_Energy - Energy) / pre_Energy;
}

void ARAPMapping::Normalize()
{
	double minx = 1e18, miny = 1e18;
	for (int i = 0; i < nV; ++i) 
	{
		minx = std::min(minx, ut[i][0]);
		miny = std::min(miny, ut[i][1]);
	}
	for (int i = 0; i < nV; ++i)
	{
		ut[i][0] -= minx;
		ut[i][1] -= miny;
	}
}