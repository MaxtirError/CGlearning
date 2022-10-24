#include <QtCore>
#include "MeshViewerWidget.h"

MeshViewerWidget::MeshViewerWidget(QWidget* parent)
	: QGLViewerWidget(parent),
	ptMin(0,0,0),
	ptMax(0,0,0),
	isEnableLighting(true),
	isTwoSideLighting(false),
	isDrawBoundingBox(false),
	isDrawBoundary(false),
	ishavetexture(false),
	issimulated(false),
	istetmode(false),
	MappingTool(NULL),
	timer(NULL),
	simulator(NULL),
	tetmesh(NULL),
	mp_status(SurfMapping::MARAP),
	bd_status(SurfMapping::BNormal)
{
}

MeshViewerWidget::~MeshViewerWidget(void)
{
}

bool MeshViewerWidget::LoadMesh(const std::string & filename)
{
	Clear();

	bool read_OK = acamcad::polymesh::loadMesh(filename, polyMesh);
	std::cout << "Load mesh from file " << filename << std::endl;
	if (read_OK)
	{
		strMeshFileName = QString::fromStdString(filename);
		QFileInfo fi(strMeshFileName);
		strMeshPath = fi.path();
		strMeshBaseName = fi.baseName();
		UpdateMesh();
		update();
		return true;
	}
	return false;
}


bool MeshViewerWidget::LoadTet(const std::string& filename)
{
	tetmesh = tetgenerator.poly_in(filename.c_str());
	tetmesh = tetgenerator.tet_out(tetmesh);
	return istetmode = tetmesh->numberofpoints;
}

void MeshViewerWidget::Clear(void)
{
	if (polyMesh != NULL) {
		polyMesh->clear();
		//polyMesh = NULL;
	}
	if (tetmesh != NULL) {
		delete tetmesh;
		tetmesh = NULL;
	}
	if (MappingTool != NULL) {
		delete MappingTool;
		MappingTool = NULL;
	}
	if (timer != NULL) {
		delete timer;
		timer = NULL;
	}
	if (simulator != NULL) {
		delete simulator;
		simulator = NULL;
	}
}

void MeshViewerWidget::UpdateMesh(void)
{
	polyMesh->updateFacesNormal();
	polyMesh->updateMeshNormal();
	polyMesh->updateVerticesNormal();
	if (polyMesh->numVertices() == 0)
	{
		std::cerr << "ERROR: UpdateMesh() No vertices!" << std::endl;
		return;
	}
	ptMin[0] = ptMin[1] = ptMin[2] = DBL_MAX;
	ptMax[0] = ptMax[1] = ptMax[2] = -DBL_MAX;

	for (const auto& vh : polyMesh->vertices())
	{
		auto p = vh->position();
		for (size_t i = 0; i < 3; i++)
		{
			ptMin[i] = ptMin[i] < p[i] ? ptMin[i] : p[i];
			ptMax[i] = ptMax[i] > p[i] ? ptMax[i] : p[i];
		}
	}

	double avelen = 0.0;
	double maxlen = 0.0;
	double minlen = DBL_MAX;
	for (const auto& eh : polyMesh->edges()) {
		double len = eh->length();
		maxlen = len > maxlen ? len : maxlen;
		minlen = len < minlen ? len : minlen;
		avelen += len;
	}

	SetScenePosition((ptMin + ptMax)*0.5, (ptMin - ptMax).norm()*0.5);
	std::cout << "Information of the input mesh:" << std::endl;
	std::cout << "  [V, E, F] = [" << polyMesh->numVertices()<< ", " << polyMesh->numEdges() << ", " << polyMesh->numPolygons() << "]\n";
	std::cout << "  BoundingBox:\n";
	std::cout << "  X: [" << ptMin[0] << ", " << ptMax[0] << "]\n";
	std::cout << "  Y: [" << ptMin[1] << ", " << ptMax[1] << "]\n";
	std::cout << "  Z: [" << ptMin[2] << ", " << ptMax[2] << "]\n";
	std::cout << "  Diag length of BBox: " << (ptMax - ptMin).norm() << std::endl;
	std::cout << "  Edge Length: [" << minlen << ", " << maxlen << "]; AVG: " << avelen / polyMesh->numEdges() << std::endl;
}

bool MeshViewerWidget::SaveMesh(const std::string & filename)
{
	return acamcad::polymesh::writeMesh(filename, polyMesh);
}

bool MeshViewerWidget::ScreenShot()
{
	update();
	QString filename = strMeshPath + "/" + QDateTime::currentDateTime().toString("yyyyMMddHHmmsszzz") + QString(".png");
	QImage image = grabFramebuffer();
	image.save(filename);
	std::cout << "Save screen shot to " << filename.toStdString() << std::endl;
	return true;
}

void MeshViewerWidget::SetDrawBoundingBox(bool b)
{
	isDrawBoundingBox = b;
	update();
}
void MeshViewerWidget::SetDrawBoundary(bool b)
{
	isDrawBoundary = b;
	update();
}
void MeshViewerWidget::EnableLighting(bool b)
{
	isEnableLighting = b;
	update();
}

void MeshViewerWidget::EnableTexture(bool b)
{
	ishavetexture = b;
	update();
}

void MeshViewerWidget::EnableDoubleSide(bool b)
{
	isTwoSideLighting = b;
	update();
}

void MeshViewerWidget::ResetView(void)
{
	ResetModelviewMatrix();
	ViewCenter();
	update();
}

void MeshViewerWidget::ViewCenter(void)
{
	if (polyMesh->numVertices()!=0)
	{
		UpdateMesh();
	}
	update();
}

void MeshViewerWidget::CopyRotation(void)
{
	CopyModelViewMatrix();
}

void MeshViewerWidget::LoadRotation(void)
{
	LoadCopyModelViewMatrix();
	update();
}

void MeshViewerWidget::LoadTexture(const QString& filename)
{
	texture_image = QImage(filename).convertToFormat(QImage::Format_RGB888);
	ishavetexture = true;
	TextureMapping();


	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_image.width(), texture_image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture_image.bits());
	update();
}


void MeshViewerWidget::PrintMeshInfo(void)
{
	std::cout << "Mesh Info:\n";
	std::cout << "  [V, E, F] = [" << polyMesh->numVertices() << ", " << polyMesh->numEdges() << ", " << polyMesh->numPolygons() << "]\n";
	std::cout << "  BoundingBox:\n";
	std::cout << "  X: [" << ptMin[0] << ", " << ptMax[0] << "]\n";
	std::cout << "  Y: [" << ptMin[1] << ", " << ptMax[1] << "]\n";
	std::cout << "  Z: [" << ptMin[2] << ", " << ptMax[2] << "]\n";
	std::cout << "  Diag length of BBox: " << (ptMax - ptMin).norm() << std::endl;
	
}

void MeshViewerWidget::DrawScene(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(&projectionmatrix[0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixd(&modelviewmatrix[0]);
	//DrawAxis();
	if (isDrawBoundingBox) DrawBoundingBox();
	if (isDrawBoundary) DrawBoundary();
	if (isEnableLighting) glEnable(GL_LIGHTING);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, isTwoSideLighting);
	DrawSceneMesh();
	if (isEnableLighting) glDisable(GL_LIGHTING);
	if (ishavetexture) glDisable(GL_TEXTURE_2D);
}

void MeshViewerWidget::DrawSceneMesh(void)
{
	if (istetmode ? tetmesh->numberofpoints == 0 : polyMesh->numVertices() == 0) { return; }
	SetMaterial();
	switch (drawmode)
	{
	case POINTS:
		DrawPoints();
		break;
	case WIREFRAME:
		DrawWireframe();
		break;
	case HIDDENLINES:
		DrawHiddenLines();
		break;
	case FLATLINES:
		DrawFlatLines();
		break;
	case FLAT:
		glColor3d(1.0, 1.0, 1.0);
		DrawFlat();
		break;
	default:
		break;
	}
}

void MeshViewerWidget::DrawPoints(void)
{
	glColor3d(1.0, 0.5, 0.5);
	glPointSize(5);
	glBegin(GL_POINTS);
	if (istetmode) {
		const double* p = tetmesh->pointlist;
		const double normal[3] = { 0, 0, 0 };
		for (int i = 0; i < tetmesh->numberofpoints; ++i) {
			//std::cout << p[i * 3] << " " << p[i * 3 + 1] << " " << p[i * 3 + 2]<<std::endl;
			glNormal3dv(normal);
			glVertex3dv(p + i * 3);
		}
	}
	else {
		for (const auto& vh : polyMesh->vertices()) {
			glNormal3dv(vh->normal().data());
			glVertex3dv(vh->position().data());
		}
	}
	glEnd();
}

void MeshViewerWidget::DrawWireframe(void)
{
	glColor3d(0.2, 0.2, 0.2);
	glBegin(GL_LINES);
	if (istetmode) {
		const double normal[3] = { 0, 0, 0 };
		const double* p = tetmesh->pointlist;
		const int* e = tetmesh->edgelist;
		for (int i = 0; i < tetmesh->numberofedges; ++i) {
			int v0 = e[i << 1], v1 = e[i << 1 | 1];
			glNormal3dv(normal);
			glVertex3dv(p + v0 * 3);
			glVertex3dv(p + v1 * 3);
		}
	}
	else {
		for (const auto& eh : polyMesh->edges()) {
			auto heh = eh->halfEdge();
			auto v0 = heh->fromVertex();
			auto v1 = heh->toVertex();
			glNormal3dv(v0->normal().data());
			glVertex3dv(v0->position().data());
			glNormal3dv(v1->normal().data());
			glVertex3dv(v1->position().data());
		}
		glEnd();
	}
}

void MeshViewerWidget::DrawHiddenLines()
{
	glLineWidth(1.0);
	float backcolor[4];
	glGetFloatv(GL_COLOR_CLEAR_VALUE, backcolor);
	glColor4fv(backcolor);
	glDepthRange(0.01, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if (glIsEnabled(GL_LIGHTING))
	{
		glDisable(GL_LIGHTING);
		DrawFlat();
		glEnable(GL_LIGHTING);
	}
	else
	{
		DrawFlat();
	}
	glDepthRange(0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3d(.3, .3, .3);
	DrawFlat();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void MeshViewerWidget::DrawFlatLines(void)
{
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.5f, 2.0f);
	glShadeModel(GL_FLAT);
	//glColor3d(0.8, 0.8, 0.8);
	glColor3d(1.0, 1.0, 1.0);
	DrawFlat();
	glDisable(GL_POLYGON_OFFSET_FILL);
	if (glIsEnabled(GL_LIGHTING))
	{
		glDisable(GL_LIGHTING);
		DrawWireframe();
		glEnable(GL_LIGHTING);
	}
	else
	{
		DrawWireframe();
	}
}

void MeshViewerWidget::DrawFlat(void)
{
	if (ishavetexture)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texture);
	}
	glBegin(GL_TRIANGLES);
	if (istetmode)
	{
		const double normal[3] = { 0, 0, 0 };
		const double* p = tetmesh->pointlist;
		const int* tri = tetmesh->trifacelist;
		for (int i = 0;i < tetmesh->numberoftrifaces; ++i)
		{
			glNormal3dv(normal);
			//glNormal3dv(fh->normal().data());
			for (int k = 0;k < 3; ++k)
			{
				glVertex3dv(p + tri[i * 3 + k] * 3);
			}
		}
	}
	else {
		for (const auto& fh : polyMesh->polyfaces())
		{
			glNormal3dv(fh->normal().data());
			for (const auto& fvh : polyMesh->polygonVertices(fh))
			{
				if (ishavetexture)
					glTexCoord2f(fvh->getTexture().uv[0], fvh->getTexture().uv[1]);
				glVertex3dv(fvh->position().data());
			}
		}
	}
	glEnd();
	if(ishavetexture)
		glDisable(GL_TEXTURE_2D);

}


void MeshViewerWidget::DrawBoundingBox(void)
{
	float linewidth;
	glGetFloatv(GL_LINE_WIDTH, &linewidth);
	glLineWidth(2.0f);
	glColor3d(.3, .7, .3);
	glBegin(GL_LINES);
	for (const auto& i : { 0, 1 })
	{
		for (const auto& j : { 0, 1 })
		{
			for (const auto& k : { 0, 1 })
			{
				glVertex3d(i ? ptMin[0] : ptMax[0], j ? ptMin[1] : ptMax[1], k ? ptMin[2] : ptMax[2]);
				glVertex3d(~i ? ptMin[0] : ptMax[0], j ? ptMin[1] : ptMax[1], k ? ptMin[2] : ptMax[2]);
				glVertex3d(i ? ptMin[0] : ptMax[0], j ? ptMin[1] : ptMax[1], k ? ptMin[2] : ptMax[2]);
				glVertex3d(i ? ptMin[0] : ptMax[0], ~j ? ptMin[1] : ptMax[1], k ? ptMin[2] : ptMax[2]);
				glVertex3d(i ? ptMin[0] : ptMax[0], j ? ptMin[1] : ptMax[1], k ? ptMin[2] : ptMax[2]);
				glVertex3d(i ? ptMin[0] : ptMax[0], j ? ptMin[1] : ptMax[1], ~k ? ptMin[2] : ptMax[2]);
			}
		}
	}
	glEnd();
	glLineWidth(linewidth);
}

void MeshViewerWidget::DrawBoundary(void)
{
	float linewidth;
	glGetFloatv(GL_LINE_WIDTH, &linewidth);
	glLineWidth(2.0f);
	glColor3d(0.1, 0.1, 0.1);
	glBegin(GL_LINES);

	for (const auto& eh : polyMesh->edges()) {
		if (polyMesh->isBoundary(eh)) {
			auto heh = eh->halfEdge();
			auto v0 = heh->fromVertex();
			auto v1 = heh->toVertex();
			glNormal3dv(v0->normal().data());
			glVertex3dv(v0->position().data());
			glNormal3dv(v1->normal().data());
			glVertex3dv(v1->position().data());
		}
	}
	
	glEnd();
	glLineWidth(linewidth);
}

void MeshViewerWidget::MiniSurf()
{
	TextureMapping();
	for (auto v : polyMesh->vertices())
	{
		Texcoord Position = v->getTextureUVW();
		//printf("%.2lf %.2lf %.2lf\n", Position[0], Position[1], Position[2]);
		v->setPosition(Position[0], Position[1], Position[2]);
	}
	update();
}

void MeshViewerWidget::TextureMapping()
{
	if (MappingTool == NULL)
	{
		if (mp_status != SurfMapping::MARAP && mp_status != SurfMapping::MASAP)
			MappingTool = new BoundarySurfMap(polyMesh);
		else
			MappingTool = new ARAPMapping(polyMesh);
		MappingTool->SetBoundaryMode(bd_status);
		MappingTool->SetMappingMode(mp_status);
		MappingTool->Initialize();
		MappingTool->Boundary_Mapping();
		MappingTool->Parametrize();
		if (mp_status == SurfMapping::MARAP)
			MappingTool->LGAdjusting(10, 0.001);
	}
}

void MeshViewerWidget::SetBdNormal()
{
	bd_status = SurfMapping::BNormal;
	if (MappingTool != NULL)
	{
		MappingTool->SetBoundaryMode(BoundarySurfMap::BNormal);
		MappingTool->Boundary_Mapping();
		MappingTool->Parametrize();
	}
}

void MeshViewerWidget::SetBdSquare()
{
	bd_status = SurfMapping::BSquare;
	if (MappingTool != NULL)
	{
		MappingTool->SetBoundaryMode(BoundarySurfMap::BSquare);
		MappingTool->Boundary_Mapping();
		MappingTool->Parametrize();
	}
}

void MeshViewerWidget::SetBdCircle()
{
	bd_status = SurfMapping::BCircle;
	if (MappingTool != NULL)
	{
		MappingTool->SetBoundaryMode(BoundarySurfMap::BCircle);
		MappingTool->Boundary_Mapping();
		MappingTool->Parametrize();
	}
}


void MeshViewerWidget::SetMpUniform()
{
	mp_status = SurfMapping::MUniform;
	if (MappingTool != NULL)
	{
		MappingTool->SetMappingMode(BoundarySurfMap::MUniform);
		MappingTool->Parametrize();
	}
}


void MeshViewerWidget::SetMpCot()
{
	mp_status = SurfMapping::MCotan;
	if (MappingTool != NULL)
	{
		MappingTool->SetMappingMode(BoundarySurfMap::MCotan);
		MappingTool->Parametrize();
	}
}

void MeshViewerWidget::SetMpASAP()
{
	mp_status = SurfMapping::MASAP;
	if (MappingTool != NULL)
	{
		MappingTool->SetMappingMode(BoundarySurfMap::MASAP);
		MappingTool->Parametrize();
	}
}


void MeshViewerWidget::SetMpARAP()
{
	mp_status = SurfMapping::MARAP;
	if (MappingTool != NULL)
	{
		MappingTool->SetMappingMode(BoundarySurfMap::MARAP);
		MappingTool->Parametrize();
		MappingTool->LGAdjusting(10, 0.001);
	}
}

void MeshViewerWidget::Simulation()
{
	if (istetmode) {
		LockPoints.clear();
		double miny = 1e9;
		const double* P = tetmesh->pointlist;
		for (int i = 0; i < tetmesh->numberofpoints; ++i)
		{
			miny = std::min(miny, P[i * 3]);
		}

		for (int i = 0; i < tetmesh->numberofpoints; ++i)
		if(P[i * 3 + 1] < miny + 1e-9)
		{
			LockPoints.push_back(i);
		}
		simulator = new EnergySimulator();
		simulator->Initialize_Tet(tetmesh, &LockPoints);
		simulator->Analyze();
	}
	else {
		LockPoints.clear();
		for (auto v : polyMesh->vertices()) {
			if (v->x() <= 1e-9 && v->y() <= 1e-9)
				LockPoints.push_back(v->index());
			if (v->x() <= 1e-9 && v->y() >= 1 - 1e-9)
				LockPoints.push_back(v->index());
		}
		puts("LockPoints");
		for (auto x : LockPoints)
			printf("%d ", x);
		puts("");
		simulator = new EnergySimulator();
		simulator->Initialize_Mesh(polyMesh, &LockPoints);
		simulator->Analyze();
	}
	timer = new QTimer();
	timer->setInterval(FPS);

	connect(timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

	timer->start();
}


void MeshViewerWidget::onTimeout()
{
	if (istetmode)
		simulator->UpdateTet(tetmesh);
	else 
		simulator->UpdateMesh(polyMesh);
	update();
	simulator->Simulation();
}