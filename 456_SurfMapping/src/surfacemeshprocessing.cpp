#include "surfacemeshprocessing.h"
#include "MeshViewer/MainViewerWidget.h"

SurfaceMeshProcessing::SurfaceMeshProcessing(QWidget *parent)
	: QMainWindow(parent)
{
	setWindowTitle(tr("Surface Mesh Processing"));
	viewer = new MainViewerWidget(this);
	setCentralWidget(viewer);
	CreateActions();
	CreateMenus();
	CreateToolBars();
	CreateStatusBar();
}

SurfaceMeshProcessing::~SurfaceMeshProcessing(void)
{
}

void SurfaceMeshProcessing::CreateActions(void)
{
	actOpen = new QAction(tr("&Open"), this);
	actOpen->setIcon(QIcon(":/SurfaceMeshProcessing/Images/Open.png"));
	actOpen->setShortcut(QKeySequence::Open);
	actOpen->setStatusTip(tr("Open a mesh file"));
	connect(actOpen, SIGNAL(triggered()), viewer, SLOT(Open()));

	actOpenTet = new QAction(tr("&OpenTet"), this);
	actOpenTet->setIcon(QIcon(":/SurfaceMeshProcessing/Images/Open.png"));
	actOpenTet->setShortcut(QKeySequence::Open);
	actOpenTet->setStatusTip(tr("Open a tet file"));
	connect(actOpenTet, SIGNAL(triggered()), viewer, SLOT(OpenTet()));

	actSave = new QAction(tr("&Save"), this);
	actSave->setIcon(QIcon(":/SurfaceMeshProcessing/Images/Save.png"));
	actSave->setShortcut(QKeySequence::Save);
	actSave->setStatusTip(tr("Save the mesh to file"));
	connect(actSave, SIGNAL(triggered()), viewer, SLOT(Save()));

	actClearMesh = new QAction(("Clear Mesh"), this);
	actClearMesh->setIcon(QIcon(":/SurfaceMeshProcessing/Images/ClearMesh.png"));
	actClearMesh->setStatusTip(tr("Clear the Current Mesh"));
	connect(actClearMesh, SIGNAL(triggered()), viewer, SLOT(ClearMesh()));

	actScreenshot = new QAction("Screenshot", this);
	actScreenshot->setIcon(QIcon(":/SurfaceMeshProcessing/Images/saveScreen.png"));
	actScreenshot->setShortcut(tr("F2"));
	actScreenshot->setStatusTip(tr("Save Screenshot"));
	connect(actScreenshot, SIGNAL(triggered()), viewer, SLOT(Screenshot()));

	actExit = new QAction(tr("E&xit"), this);
	actExit->setShortcut(QKeySequence::Quit);
	actExit->setStatusTip(tr("Exit the application"));
	connect(actExit, SIGNAL(triggered()), SLOT(close()));

	actPoints = new QAction(tr("Points"), this);
	actPoints->setIcon(QIcon(":/SurfaceMeshProcessing/Images/points.png"));
	actPoints->setStatusTip(tr("Show points"));
	actPoints->setCheckable(true);
	connect(actPoints, SIGNAL(triggered()), viewer, SLOT(ShowPoints()));

	actWireframe = new QAction(tr("Wireframe"), this);
	actWireframe->setIcon(QIcon(":/SurfaceMeshProcessing/Images/wire.png"));
	actWireframe->setStatusTip(tr("Show wireframe"));
	actWireframe->setCheckable(true);
	connect(actWireframe, SIGNAL(triggered()), viewer, SLOT(ShowWireframe()));

	actHiddenLines = new QAction(tr("Hidden Lines"), this);
	actHiddenLines->setIcon(QIcon(":/SurfaceMeshProcessing/Images/hiddenlines.png"));
	actHiddenLines->setStatusTip(tr("Show hidden lines"));
	actHiddenLines->setCheckable(true);
	connect(actHiddenLines, SIGNAL(triggered()), viewer, SLOT(ShowHiddenLines()));

	actFlatLines = new QAction(tr("Flat Lines"), this);
	actFlatLines->setIcon(QIcon(":/SurfaceMeshProcessing/Images/flatlines.png"));
	actFlatLines->setStatusTip(tr("Show flat lines"));
	actFlatLines->setCheckable(true);
	connect(actFlatLines, SIGNAL(triggered()), viewer, SLOT(ShowFlatLines()));

	actFlat = new QAction(tr("Flat"), this);
	actFlat->setIcon(QIcon(":/SurfaceMeshProcessing/Images/flat.png"));
	actFlat->setStatusTip(tr("Show flat"));
	actFlat->setCheckable(true);
	connect(actFlat, SIGNAL(triggered()), viewer, SLOT(ShowFlat()));


	QActionGroup *agViewGroup = new QActionGroup(this);
	agViewGroup->addAction(actPoints);
	agViewGroup->addAction(actWireframe);
	agViewGroup->addAction(actHiddenLines);
	agViewGroup->addAction(actFlatLines);
	agViewGroup->addAction(actFlat);
	actFlatLines->setChecked(true);

	actLighting = new QAction(tr("Light on/off"), this);
	actLighting->setIcon(QIcon(":/SurfaceMeshProcessing/Images/lighton.png"));
	actLighting->setStatusTip(tr("Turn light on/off"));
	actLighting->setCheckable(true);
	actLighting->setChecked(true);
	connect(actLighting, SIGNAL(toggled(bool)), viewer, SLOT(Lighting(bool)));

	actTexture = new QAction(tr("Texture"), this);
	actTexture->setIcon(QIcon(":/SurfaceMeshProcessing/Images/tt.png"));
	actTexture->setStatusTip(tr("Turn texture on/off"));
	actTexture->setCheckable(true);
	connect(actTexture, SIGNAL(toggled(bool)), viewer, SLOT(Texture(bool)));

	actMiniSurf = new QAction(tr("Minisurf"), this);
	actMiniSurf->setIcon(QIcon(":/SurfaceMeshProcessing/Images/tt.png"));
	actMiniSurf->setStatusTip(tr("transform to minisurf"));
	connect(actMiniSurf, SIGNAL(triggered()), viewer, SLOT(MiniSurf()));

	actDoubleSide = new QAction(tr("Double Side Lighting"), this);
	actDoubleSide->setStatusTip(tr("Double side lighting"));
	actDoubleSide->setCheckable(true);
	connect(actDoubleSide, SIGNAL(toggled(bool)), viewer, SLOT(DoubleSideLighting(bool)));

	actBoundingBox = new QAction("Bounding Box", this);
	actBoundingBox->setIcon(QIcon(":/SurfaceMeshProcessing/Images/bbox.png"));
	actBoundingBox->setStatusTip(tr("Show bounding box"));
	actBoundingBox->setCheckable(true);
	connect(actBoundingBox, SIGNAL(toggled(bool)), viewer, SLOT(ShowBoundingBox(bool)));

	actBoundary = new QAction("Boundary", this);
	actBoundary->setIcon(QIcon(":/SurfaceMeshProcessing/Images/boundary.png"));
	actBoundary->setStatusTip(tr("Show mesh boundary"));
	actBoundary->setCheckable(true);
	connect(actBoundary, SIGNAL(toggled(bool)), viewer, SLOT(ShowBoundary(bool)));

	actResetView = new QAction("Reset View", this);
	actResetView->setStatusTip(tr("Reset view"));
	connect(actResetView, SIGNAL(triggered()), viewer, SLOT(ResetView()));

	actViewCenter = new QAction("View Center", this);
	actViewCenter->setStatusTip(tr("View center"));
	connect(actViewCenter, SIGNAL(triggered()), viewer, SLOT(ViewCenter()));

	actCopyRotation = new QAction("Copy Rotation", this);
	actCopyRotation->setStatusTip(tr("Copy Rotation"));
	connect(actCopyRotation, SIGNAL(triggered()), viewer, SLOT(CopyRotation()));

	actLoadRotation = new QAction("Load Rotation", this);
	actLoadRotation->setStatusTip(tr("Load Rotation"));
	connect(actLoadRotation, SIGNAL(triggered()), viewer, SLOT(LoadRotation()));

	actLoadTexture = new QAction("Load Texture", this);
	actLoadTexture->setStatusTip(tr("Load Texture"));
	connect(actLoadTexture, SIGNAL(triggered()), viewer, SLOT(LoadTexture()));


	actSetBoundaryNormal = new QAction("Normal", this);
	actSetBoundaryNormal->setStatusTip(tr("set texture boundary Normal"));
	actSetBoundaryNormal->setCheckable(true);
	connect(actSetBoundaryNormal, SIGNAL(triggered()), SLOT(on_triggered_act_normal()));
	connect(actSetBoundaryNormal, SIGNAL(triggered()), viewer, SLOT(SetBdNormal()));

	actSetBoundarySquare = new QAction("Square", this);
	actSetBoundarySquare->setStatusTip(tr("set texture boundary Square"));
	actSetBoundarySquare->setCheckable(true);
	actSetBoundaryNormal->setChecked(true);
	connect(actSetBoundarySquare, SIGNAL(triggered()), SLOT(on_triggered_act_square()));
	connect(actSetBoundarySquare, SIGNAL(triggered()), viewer, SLOT(SetBdSquare()));

	actSetBoundaryCircle = new QAction("Circle", this);
	actSetBoundaryCircle->setStatusTip(tr("set texture boundary Circle"));
	actSetBoundaryCircle->setCheckable(true);
	connect(actSetBoundaryCircle, SIGNAL(triggered()), SLOT(on_triggered_act_circle()));
	connect(actSetBoundaryCircle, SIGNAL(triggered()), viewer, SLOT(SetBdCircle()));

	actSetMappingUniform = new QAction("Unfiorm", this);
	actSetMappingUniform->setStatusTip(tr("set texture mapping Unfiorm"));
	actSetMappingUniform->setCheckable(true);
	actSetMappingUniform->setChecked(true);
	connect(actSetMappingUniform, SIGNAL(triggered()), SLOT(on_triggered_act_uniform()));
	connect(actSetMappingUniform, SIGNAL(triggered()), viewer, SLOT(SetMpUniform()));

	actSetMappingCot = new QAction("Cot", this);
	actSetMappingCot->setStatusTip(tr("set texture mapping Cot"));
	actSetMappingCot->setCheckable(true);
	connect(actSetMappingCot, SIGNAL(triggered()), SLOT(on_triggered_act_cot()));
	connect(actSetMappingCot, SIGNAL(triggered()), viewer, SLOT(SetMpCot()));

	actSimulation = new QAction("Simulation", this);
	actSimulation->setIcon(QIcon(":/SurfaceMeshProcessing/Images/boundary.png"));
	actSimulation->setStatusTip(tr("start Simulation"));
	connect(actSimulation, SIGNAL(triggered()), viewer, SLOT(Simulation()));

	actAbout = new QAction(tr("About"), this);
	connect(actAbout, SIGNAL(triggered()), SLOT(About()));
}

void SurfaceMeshProcessing::CreateMenus(void)
{
	QMenu *menuFile = menuBar()->addMenu(tr("&File"));
	menuFile->addAction(actOpen);
	menuFile->addAction(actOpenTet);
	menuFile->addAction(actSave);
	menuFile->addAction(actClearMesh);
	menuFile->addSeparator()->setEnabled(false);
	menuFile->addAction(actScreenshot);
	menuFile->addSeparator()->setEnabled(false);
	menuFile->addAction(actExit);

	QMenu *menuView = menuBar()->addMenu(tr("&View"));
	QMenu *menuRenderMode = menuView->addMenu(tr("Render Mode"));
	menuRenderMode->addAction(actPoints);
	menuRenderMode->addAction(actWireframe);
	menuRenderMode->addAction(actHiddenLines);
	menuRenderMode->addAction(actFlatLines);
	menuRenderMode->addAction(actFlat);
	menuRenderMode->addAction(actTexture);
	menuRenderMode->addAction(actMiniSurf);

	QMenu* menuTexture = menuView->addMenu(tr("Texture Mode"));
	QMenu* menuBdMode = menuTexture->addMenu(tr("Boundary Mode"));
	menuBdMode->addAction(actSetBoundaryNormal);
	menuBdMode->addAction(actSetBoundarySquare);
	menuBdMode->addAction(actSetBoundaryCircle);
	QMenu* menuMpMode = menuTexture->addMenu(tr("Mapping Mode"));
	menuMpMode->addAction(actSetMappingUniform);
	menuMpMode->addAction(actSetMappingCot);
	menuTexture->addAction(actLoadTexture);


	QMenu *menuLighting = menuView->addMenu(tr("Lighting"));
	menuLighting->addAction(actLighting);
	menuLighting->addAction(actDoubleSide);
	menuView->addSeparator()->setEnabled(false);
	menuView->addAction(actBoundingBox);
	menuView->addAction(actBoundary);
	menuView->addAction(actSimulation);
	menuView->addSeparator()->setEnabled(false);
	QMenu *menuRotation = menuView->addMenu(tr("Rotation"));
	menuRotation->addAction(actResetView);
	menuRotation->addAction(actViewCenter);
	menuRotation->addAction(actCopyRotation);
	menuRotation->addAction(actLoadRotation);

	QMenu *menuHelp = menuBar()->addMenu(tr("&Help"));
	menuHelp->addAction(actAbout);
}

void SurfaceMeshProcessing::CreateToolBars(void)
{
	QToolBar *tbStandard = addToolBar(tr("Standard"));
	tbStandard->addAction(actOpen);
	tbStandard->addAction(actOpenTet);
	tbStandard->addAction(actSave);
	tbStandard->addAction(actClearMesh);
	tbStandard->addSeparator()->setEnabled(false);
	tbStandard->addAction(actScreenshot);
	tbStandard->addSeparator()->setEnabled(false);

	QToolBar *tbView = addToolBar(tr("&View"));
	tbView->addAction(actPoints);
	tbView->addAction(actWireframe);
	tbView->addAction(actHiddenLines);
	tbView->addAction(actFlatLines);
	tbView->addAction(actFlat);
	tbView->addSeparator()->setEnabled(false);
	tbView->addAction(actLighting);
	tbView->addAction(actBoundingBox);
	tbView->addAction(actBoundary);
	tbView->addAction(actTexture);
	tbView->addAction(actMiniSurf);
	tbView->addAction(actSimulation);
}

void SurfaceMeshProcessing::CreateStatusBar(void)
{
	connect(viewer, SIGNAL(haveLoadMesh(QString)), statusBar(), SLOT(showMessage(QString)));
}

void SurfaceMeshProcessing::About(void)
{
	QMessageBox::about(this, QString("About ") + this->windowTitle(),
		QString("<div><center><h1>") + this->windowTitle() + " v0.1</h1>" +
		"<div>Copyright &#169; 2019 Shuangming Chai</div>"
		"<div>This program can load and process surface meshes.</div>"
		"<div>If you have any problems, feel free to send emails to "
		"<a href=\"mailto:kfckfckf@mail.ustc.edu.cn\">kfckfckf@mail.ustc.edu.cn</a></div>");
}


void SurfaceMeshProcessing::on_triggered_act_square(void)
{
	actSetBoundaryCircle->setChecked(false);
	actSetBoundaryNormal->setChecked(false);
}

void SurfaceMeshProcessing::on_triggered_act_circle(void)
{
	actSetBoundarySquare->setChecked(false);
	actSetBoundaryNormal->setChecked(false);
}

void SurfaceMeshProcessing::on_triggered_act_normal(void)
{
	actSetBoundarySquare->setChecked(false);
	actSetBoundaryCircle->setChecked(false);
}

void SurfaceMeshProcessing::on_triggered_act_uniform(void)
{
	actSetMappingCot->setChecked(false);
}

void SurfaceMeshProcessing::on_triggered_act_cot(void)
{
	actSetMappingUniform->setChecked(false);
}