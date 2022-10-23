#pragma once
#include <QWidget>
#include<vector>
#include<list>
typedef std::vector<QPoint> ScannerRegion;
typedef class XET {
public:
	XET();
	XET(QPoint s, QPoint e);
	void Move();
	void Insert(XET Edge);
	bool CheckNext(int current_y);
	XET* next;
	double x;
	bool operator < (XET b);
private:
	int ymax;
	double dx;
}AET, NET;
class LineScanner
{
public:
	LineScanner();
	~LineScanner();
	void add_point(QPoint point);
	ScannerRegion Scanning();
private:
	void Get_NET();
	void Insert_Edge(QPoint s, QPoint e);
	int m_miny, m_maxy;
	QPolygon m_Polygon;
	NET* pnet[1024];
};