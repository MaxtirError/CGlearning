#pragma once
#include <QWidget>
#include "Polygon.h"
#include "SparseSolver.h"
class ChildWindow;
QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

enum DrawStatus
{
	kChooseRect, 
	kPaste, 
	kChoosePolygon,
	kMove,
	kFill,
	kNone
};

class ImageWidget :
	public QWidget
{
	Q_OBJECT

public:
	ImageWidget(ChildWindow *relatewindow);
	~ImageWidget(void);

	int ImageWidth();											// Width of image
	int ImageHeight();											// Height of image
	void set_draw_status_to_choose_Rect();
	void set_draw_status_to_paste();
	void set_draw_status_to_choose_Polygon();
	void set_draw_status_to_fill();
	void set_draw_status_to_move();
	QImage* image();
	void set_source_window(ChildWindow* childwindow);

protected:
	void paintEvent(QPaintEvent *paintevent);
	void mousePressEvent(QMouseEvent *mouseevent);
	void mouseMoveEvent(QMouseEvent *mouseevent);
	void mouseReleaseEvent(QMouseEvent *mouseevent);
	void mouseDoubleClickEvent(QMouseEvent* mouseevent);

public slots:
	// File IO
	void Open(QString filename);								// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Invert();												// Invert pixel value in image
	void Mirror(bool horizontal=false, bool vertical=true);		// Mirror image vertically or horizontally
	void TurnGray();											// Turn image to gray-scale map
	void Restore();												// Restore image to origin
	VectorXd Move(int xpos, int ypos, int rgbtype);
	void Move(int xpos, int ypos);

public:
	QPoint						point_start_;					// Left top point of rectangle region
	QPoint						point_end_;						// Right bottom point of rectangle region

private:
	QImage						*image_;						// image 
	QImage						*image_backup_;

	CPolygon					*shape_;
	SparseSolver* Solver;

	// Pointer of child window
	ChildWindow					*source_window_;				// Source child window

	// Signs
	DrawStatus					draw_status_;					// Enum type of draw status
	ScannerRegion pointlist_;
	bool						is_choosing_;
	bool						is_pasting_;
	bool is_moving_;
	int minx_, miny_;

};

