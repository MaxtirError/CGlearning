#pragma once
#include <QString>
#include "QGLViewerWidget.h"
#include "../PolyMesh/include/PolyMesh/IOManager.h"
#include "../SurfMapping/BoundarySurfMap.h"
#include "../SurfMapping/ARAPMapping.h"
#include "../SpringSystem/SpringSystem.h"
#include <QTime>
#include "../TetGene/Tet_generate.h"
#include "../TetGene/tetgen.h"
#define MAPPING_SIZE 256
class MeshViewerWidget : public QGLViewerWidget
{
	Q_OBJECT
public:
	MeshViewerWidget(QWidget* parent = 0);
	virtual ~MeshViewerWidget(void);
	bool LoadMesh(const std::string & filename);
	bool LoadTet(const std::string& filename);
	void Clear(void);
	void UpdateMesh(void);
	bool SaveMesh(const std::string & filename);
	bool ScreenShot(void);
	void SetDrawBoundingBox(bool b);
	void SetDrawBoundary(bool b);
	void EnableLighting(bool b);
	void EnableTexture(bool b);
	void EnableDoubleSide(bool b);
	void ResetView(void);
	void ViewCenter(void);
	void CopyRotation(void);
	void LoadRotation(void);
	void LoadTexture(const QString& filename);
	void MiniSurf();
	void SetBdNormal();
	void SetBdCircle();
	void SetBdSquare();
	void SetMpUniform();
	void SetMpCot();
	void SetMpASAP();
	void SetMpARAP();
	void Simulation();
signals:
	void LoadMeshOKSignal(bool, QString);
public slots:
	void PrintMeshInfo(void);
	void onTimeout(void);
protected:
	virtual void DrawScene(void) override;
	void DrawSceneMesh(void);

private:
	void DrawPoints(void);
	void DrawWireframe(void);
	void DrawHiddenLines(void);
	void DrawFlatLines(void);
	void DrawFlat(void);
	void DrawBoundingBox(void);
	void DrawBoundary(void);
	void TextureMapping();
protected:
	acamcad::polymesh::PolyMesh* polyMesh = new acamcad::polymesh::PolyMesh();
	QString strMeshFileName;
	QString strMeshBaseName;
	QString strMeshPath;
	acamcad::MPoint3 ptMin;
	acamcad::MPoint3 ptMax;
	GLuint texture;
	QImage texture_image; 
	SurfMapping *MappingTool;
	SurfMapping::MappingMode mp_status;
	SurfMapping::BoundaryMode bd_status;
	QTimer *timer;
	SpringSystem *simulator;
	std::vector<int>LockPoints;
	CTet_generate tetgenerator;
	tetgenio* tetmesh;
	
	bool isEnableLighting;
	bool isTwoSideLighting;
	bool isDrawBoundingBox;
	bool isDrawBoundary;
	bool ishavetexture;
	bool issimulated;
	bool istetmode;
};
