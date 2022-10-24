#include "BoundarySurfMap.h"

BoundarySurfMap::BoundarySurfMap(PolyMesh* _polymesh)
{
	bd_status = BNormal;
	mp_status = MUniform;
	m_polymesh = _polymesh;
	bd_length = 0;
	UniformSolver = NULL;
	CotSolver = NULL;
	is_boundary = NULL;
	next_id = NULL;
	mat_id = NULL;
	PointList = NULL;
}

BoundarySurfMap::~BoundarySurfMap()
{
	if (UniformSolver != NULL)
		delete UniformSolver;
	if (CotSolver != NULL)
		delete CotSolver;
	if (is_boundary != NULL)
		delete[]is_boundary;
	if (next_id != NULL)
		delete[]next_id;
	if (mat_id != NULL)
		delete[]mat_id;
	if (PointList != NULL)
		delete[]PointList;
}

void BoundarySurfMap::Get_boundary()
{
	int n = m_polymesh->numVertices();
	is_boundary = new bool[n];
	next_id = new int[n];
	mat_id = new int[n];
	PointList = new MPoint3[n];
	for (int i = 0; i < n; ++i)
		is_boundary[i] = next_id[i] = mat_id[i] = 0;

	for (const auto& eh : m_polymesh->edges()) { // find boundary
		if (m_polymesh->isBoundary(eh)) {
			auto heh = eh->halfEdge();
			auto v0 = heh->fromVertex();
			auto v1 = heh->toVertex();
			int v0_id = v0->index();
			int v1_id = v1->index();
			is_boundary[v0_id] = true;
			is_boundary[v1_id] = true;
			next_id[v0_id] = v1_id;
			//printf("link relation:%d %d\n", v0_id, v1_id);
			bd_start = v0_id; //auto start
			bd_length += distance(v0->position(), v1->position());
		}
	}
	mat_size = 0;
	for (auto v : m_polymesh->vertices())
	{
		int v_id = v->index();
		PointList[v_id] = v->position();
		if (!is_boundary[v_id])
			mat_id[v_id] = mat_size++;
	}
}

void BoundarySurfMap::Boundary_Mapping()
{
	double position = 0;
	int p = bd_start;
	Texcoord* F = new Texcoord[m_polymesh->numVertices() + 1];
	switch (bd_status) 
	{
		case BNormal:
			for (auto v : m_polymesh->vertices())
				if (is_boundary[v->index()])
					v->setTexture(Texcoord(v->x(), v->y(), v->z()));
			break;
		case BSquare:
			do { //mapping the boundary
				if (position <= 1)
					F[p] = Texcoord(position, 0);
				else if (position <= 2)
					F[p] = Texcoord(1, (position - 1));
				else if (position <=  3)
					F[p] = Texcoord(1 - (position - 2), 1);
				else
					F[p] = Texcoord(0, 1 - (position - 3));
				double step = distance(PointList[p], PointList[next_id[p]]) / bd_length * 4;
				position += step;
				//printf("boundary vertex:%.2lf %.2lf %.2lf\n", PointList[p][0], PointList[p][1], PointList[p][2]);
				//printf("mapping data:%.2lf %.2lf\n", F[p].uv[0], F[p].uv[1]);
				p = next_id[p];
			} while (p != bd_start);

			for (auto v : m_polymesh->vertices())
				if (is_boundary[v->index()]) {
					v->setTexture(F[v->index()]);
				}
			break;
		case BCircle:
			do { //mapping the boundary
				F[p] = Texcoord(0.5 + 0.5 * sin(2 * pi * position), 0.5 + 0.5 * cos(2 * pi * position));
				double step = distance(PointList[p], PointList[next_id[p]]) / bd_length;
				position += step;
				//printf("boundary vertex:%.2lf %.2lf %.2lf\n", PointList[p][0], PointList[p][1], PointList[p][2]);
				//printf("mapping data:%.2lf %.2lf\n", F[p].uv[0], F[p].uv[1]);
				p = next_id[p];
			} while (p != bd_start);

			for (auto v : m_polymesh->vertices())
				if (is_boundary[v->index()]) {
					v->setTexture(F[v->index()]);
				}
			break;
	};
	delete[]F;
}

void BoundarySurfMap::Initialize()
{
	Get_boundary();
	Boundary_Mapping();
}

void BoundarySurfMap::Parametrize()
{
	bool status = mp_status == MUniform;
	SparseSolver*& Solver = status ? UniformSolver : CotSolver;
	bool isbuilt = true;
	if (Solver == NULL)
	{
		isbuilt = false;
		Solver = new SparseSolver(mat_size);
	}

	auto cottheta = [](MVector3 A, MVector3 B)
	{
		double cos2 = (dot(A, B) * dot(A, B)) / A.normSq() / B.normSq();
		return sqrt(cos2 / (1 - cos2));
	};

	MatrixXd b(mat_size, 3);
	for (const auto& v : m_polymesh->vertices())
	{
		if (!is_boundary[v->index()])
		{
			auto neighbors = m_polymesh->vertAdjacentVertices(v);
			int degree = neighbors.size();
			int cid = mat_id[v->index()];
			double ansx = 0, ansy = 0, ansz = 0;
			double weight_sum = 0;
			for (int i = 0;i < degree; ++i)
			{
				auto nv = neighbors[i];
				int nv_id = nv->index();
				double weight = 1;
				if (!status)
				{
					MVector3 nv0(nv->position());
					MVector3 nv1(neighbors[i ? i - 1 : degree - 1]->position());
					MVector3 nv2(neighbors[i == degree - 1 ? 0 : i + 1]->position());
					weight = cottheta(nv0 - nv1, v->position() - nv1) + cottheta(nv0 - nv2, v->position() - nv2);
					weight_sum += weight;
				}
				if (!is_boundary[nv_id])
				{
					if (!isbuilt) 
					{
						int nid = mat_id[nv_id];
						Solver->SetCoefficient(cid, nid, -weight);
					}
				}
				else
				{
					Texcoord pos = nv->getTextureUVW();
					ansx += pos.uv[0] * weight;
					ansy += pos.uv[1] * weight;
					ansz += pos.uv[2] * weight;
				}
			}
			if (!isbuilt)
				Solver->SetCoefficient(cid, cid, status ? degree : weight_sum);
			b(cid, 0) = ansx;
			b(cid, 1) = ansy;
			b(cid, 2) = ansz;
		}
	}
	if (!isbuilt) 
	{
		Solver->Analyze();
	}
	auto Ans = Solver->solve(b);

	for (const auto& v : m_polymesh->vertices())
	{
		if (!is_boundary[v->index()])
		{
			int cid = mat_id[v->index()];
			v->setTexture(Texcoord(Ans(cid, 0), Ans(cid, 1), Ans(cid, 2)));

			//printf("inside vertex:%.2lf %.2lf %.2lf\n", v->nx(), v->ny(), v->nz());
			//printf("mapping data:%.2lf %.2lf\n", Ans(cid, 0), Ans(cid, 1));
		}
	}
}
