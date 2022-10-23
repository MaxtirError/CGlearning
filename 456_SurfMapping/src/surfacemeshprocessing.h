#pragma once
#include <QMainWindow>
#include <QtGui>
#include <QtWidgets>

class MainViewerWidget;

class SurfaceMeshProcessing : public QMainWindow
{
	Q_OBJECT
public:
	SurfaceMeshProcessing(QWidget *parent = 0);
	~SurfaceMeshProcessing(void);

private:
	void CreateActions(void);
	void CreateMenus(void);
	void CreateToolBars(void);
	void CreateStatusBar(void);

	private slots:
	void About(void);
	void on_triggered_act_square();
	void on_triggered_act_circle();
	void on_triggered_act_normal();
	void on_triggered_act_uniform();
	void on_triggered_act_cot();

private:
	// File Actions.
	QAction *actOpen;
	QAction* actOpenTet;
	QAction *actSave;
	QAction *actClearMesh;
	QAction *actScreenshot;
	QAction *actExit;

	// View Actions.
	QAction *actPoints;
	QAction *actWireframe;
	QAction *actHiddenLines;
	QAction *actFlatLines;
	QAction *actFlat;
	QAction *actLighting;
	QAction* actTexture;
	QAction *actDoubleSide;
	QAction *actBoundingBox;
	QAction *actBoundary;
	QAction *actResetView;
	QAction *actViewCenter;
	QAction *actCopyRotation;
	QAction *actLoadRotation;
	QAction *actLoadTexture;
	QAction* actSetBoundaryNormal;
	QAction* actSetBoundarySquare;
	QAction* actSetBoundaryCircle;
	QAction* actSetMappingUniform;
	QAction* actSetMappingCot;
	QAction* actMiniSurf;
	QAction* actSimulation;

	// Help Actions.
	QAction *actAbout;

	MainViewerWidget* viewer;
};
