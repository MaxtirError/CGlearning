#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>
#include "ChildWindow.h"
#include "Polygon.h"

using std::cout;
using std::endl;
int g_mask[1024][1024], id[1024][1024], tim;
const int dx[] = { 0, 0, 1, -1 }, dy[] = { 1, -1, 0, 0 };

ImageWidget::ImageWidget(ChildWindow* relatewindow)
{
	image_ = new QImage();
	image_backup_ = new QImage();

	draw_status_ = kNone;
	is_choosing_ = false;
	is_pasting_ = false;

	point_start_ = QPoint(0, 0);
	point_end_ = QPoint(0, 0);


	source_window_ = NULL;
	shape_ = NULL;
}

ImageWidget::~ImageWidget(void)
{
}

int ImageWidget::ImageWidth()
{
	return image_->width();
}

int ImageWidget::ImageHeight()
{
	return image_->height();
}

void ImageWidget::set_draw_status_to_choose_Rect()
{
	draw_status_ = kChooseRect;	
}

void ImageWidget::set_draw_status_to_paste()
{
	draw_status_ = kPaste;
}


void ImageWidget::set_draw_status_to_fill()
{
	draw_status_ = kFill;
}


void ImageWidget::set_draw_status_to_move()
{
	draw_status_ = kMove;
}

void ImageWidget::set_draw_status_to_choose_Polygon()
{
	draw_status_ = kChoosePolygon;
}

QImage* ImageWidget::image()
{
	return image_;
}

void ImageWidget::set_source_window(ChildWindow* childwindow)
{
	source_window_ = childwindow;
}

void ImageWidget::paintEvent(QPaintEvent* paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect(0, 0, image_->width(), image_->height());
	painter.drawImage(rect, *image_);

	// Draw choose region
	painter.setBrush(Qt::NoBrush);
	painter.setPen(Qt::red);
	painter.drawRect(point_start_.x(), point_start_.y(),
		point_end_.x() - point_start_.x(), point_end_.y() - point_start_.y());
	if (shape_ != NULL)
		shape_->Draw(painter);
	painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent* mouseevent)
{
	if (Qt::LeftButton == mouseevent->button())
	{
		switch (draw_status_)
		{
		case kChooseRect:
			is_choosing_ = true;
			point_start_ = point_end_ = mouseevent->pos();
			break;

		case kChoosePolygon:
			is_choosing_ = true;
			if (shape_ != NULL)
				shape_->addPoint();
			else {
				shape_ = new CPolygon(mouseevent->pos());
				setMouseTracking(true);
			}
			break;

		case kFill:
			pointlist_ = shape_->Get_Inside();
			for (auto p : pointlist_)
			{
				image_->setPixel(p.rx(), p.ry(), QRgb(0));
			}
			break;

		case kMove:
		{
			is_moving_ = true;
			auto SrcWindow = source_window_->imagewidget_;
			int srcw = SrcWindow->width(), srch = SrcWindow->height();
			pointlist_ = SrcWindow->shape_->Get_Inside();
			int counter = 0; ++tim;
			minx_ = 1024; miny_ = 1024;
			for (auto p : pointlist_)
			{
				minx_ = std::min(minx_, p.rx());
				miny_ = std::min(miny_, p.ry());
				g_mask[p.rx()][p.ry()] = tim;
				id[p.rx()][p.ry()] = counter++;
			}
			printf("%d %d\n", minx_, miny_);
			Solver = new SparseSolver(counter);
			counter = 0;
			for (auto p : pointlist_)
			{
				int px = p.rx(), py = p.ry();
				int np = 4 - (px == 0) - (px == srcw - 1) - (py == 0) - (py == srch - 1);
				Solver->SetCoefficient(counter, counter, np);
				for (int k = 0; k < 4; ++k)
				{
					int nx = px + dx[k], ny = py + dy[k];
					if (nx < 0 || nx >= srcw || ny < 0 || ny >= srch)
						continue; //not in image
					if (g_mask[nx][ny] != tim)
						continue; //not in region
					Solver->SetCoefficient(counter, id[nx][ny], -1);
				}
				++counter;
			}
			Solver->Analyze();
			Move(mouseevent->pos().rx(), mouseevent->pos().ry());
		}

		case kPaste:
		{
			is_pasting_ = true;

			// Start point in object image
			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();

			// Start point in source image
			int xsourcepos = source_window_->imagewidget_->point_start_.rx();
			int ysourcepos = source_window_->imagewidget_->point_start_.ry();

			// Width and Height of rectangle region
			int w = source_window_->imagewidget_->point_end_.rx()
				- source_window_->imagewidget_->point_start_.rx() + 1;
			int h = source_window_->imagewidget_->point_end_.ry()
				- source_window_->imagewidget_->point_start_.ry() + 1;

			// Paste
			if ((xpos + w < image_->width()) && (ypos + h < image_->height()))
			{
				// Restore image
			//	*(image_) = *(image_backup_);

				// Paste
				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < h; j++)
					{
						image_->setPixel(xpos + i, ypos + j, source_window_->imagewidget_->image()->pixel(xsourcepos + i, ysourcepos + j));
					}
				}
			}
		}

		update();
		break;

		default:
			break;
		}
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
	case kChooseRect:
		// Store point position for rectangle region
		if (is_choosing_)
		{
			point_end_ = mouseevent->pos();
		}
		break;
	case kChoosePolygon:
		if (is_choosing_)
		{
			shape_->set_end(mouseevent->pos());
		}
		break;

	case kMove:
		if (is_moving_) 
		{
			Move(mouseevent->pos().rx(), mouseevent->pos().ry());
		}
		break;

	case kPaste:
		// Paste rectangle region to object image
		if (is_pasting_)
		{
			// Start point in object image
			int xpos = mouseevent->pos().rx();
			int ypos = mouseevent->pos().ry();

			// Start point in source image
			int xsourcepos = source_window_->imagewidget_->point_start_.rx();
			int ysourcepos = source_window_->imagewidget_->point_start_.ry();

			// Width and Height of rectangle region
			int w = source_window_->imagewidget_->point_end_.rx()
				- source_window_->imagewidget_->point_start_.rx() + 1;
			int h = source_window_->imagewidget_->point_end_.ry()
				- source_window_->imagewidget_->point_start_.ry() + 1;

			// Paste
			if ((xpos > 0) && (ypos > 0) && (xpos + w < image_->width()) && (ypos + h < image_->height()))
			{
				// Restore image 
				*(image_) = *(image_backup_);

				// Paste
				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < h; j++)
					{
						image_->setPixel(xpos + i, ypos + j, source_window_->imagewidget_->image()->pixel(xsourcepos + i, ysourcepos + j));
					}
				}
			}
		}
		break;

	default:
		break;
	}

	update();
}

void ImageWidget::Move(int xpos, int ypos)
{
	VectorXd rgbanser[3];
	for (int rgbtype = 0; rgbtype < 3; ++rgbtype)
	{
		rgbanser[rgbtype] = Move(xpos, ypos, rgbtype);
	}
	auto bound = [](double dx) 
	{
		int x = round(dx);
		return x < 0 ? 0 : (x > 255 ? 255 : x); 
	};
	int counter = 0;
	*(image_) = *(image_backup_); //restore
	for (auto p : pointlist_)
	{
		QRgb newpixel = qRgb(bound(rgbanser[0][counter]), bound(rgbanser[1][counter]), bound(rgbanser[2][counter]));
		int current_x = xpos + p.rx() - minx_, current_y = ypos + p.ry() - miny_;
		if ((current_x < image_->width()) && (current_y < image_->height()))
		{
			image_->setPixel(current_x, current_y, newpixel);
		}
		counter++;
	}
}

VectorXd ImageWidget::Move(int xpos, int ypos, int rgbtype)
{
	assert(is_moving_ == true);
	auto SrcImage = source_window_->imagewidget_;
	int srcw = SrcImage->width(), srch = SrcImage->height();
	int counter = 0;
	auto Getrgb = [rgbtype](QRgb pixel)
	{
		return rgbtype == 0 ? qRed(pixel) : (rgbtype == 1 ? qGreen(pixel) : qBlue(pixel));
	};
	for (auto p : pointlist_)
	{
		int px = p.rx(), py = p.ry();
		int ans = 0;
		for (int k = 0; k < 4; ++k)
		{
			int nx = px + dx[k], ny = py + dy[k];
			if (nx < 0 || nx >= srcw || ny < 0 || ny >= srch)
				continue; //not in image
			if (g_mask[nx][ny] == tim) //q in np
			{
				ans += Getrgb(SrcImage->image()->pixel(px, py)) - Getrgb(SrcImage->image()->pixel(nx, ny));
			}
			else // q in boundery
			{
				int current_x = xpos + nx - minx_, current_y = ypos + ny - miny_;
				if ((current_x < image_->width()) && (current_y < image_->height()))
				{
					ans += Getrgb(image_->pixel(current_x, current_y));
				}
			}
		}
		Solver->SetAnswer(counter++, ans);
	}
	return Solver->solve();
}


void ImageWidget::mouseReleaseEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
	case kChooseRect:
		if (is_choosing_)
		{
			point_end_ = mouseevent->pos();
			is_choosing_ = false;
			draw_status_ = kNone;
		}
		break;
	case kChoosePolygon:
		break;

	case kPaste:
		if (is_pasting_)
		{
			is_pasting_ = false;
			draw_status_ = kNone;
		}
		break;

	case kMove:
		if (is_moving_)
		{
			is_moving_ = false;
			draw_status_ = kNone;
		}
		break;

	default:
		break;
	}

	update();
}

void ImageWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (draw_status_ == kChoosePolygon)
	{
		draw_status_ = kNone;
		setMouseTracking(false);
	}
}

void ImageWidget::Open(QString filename)
{
	// Load file
	if (!filename.isEmpty())
	{
		image_->load(filename);
		*(image_backup_) = *(image_);
	}

	//	setFixedSize(image_->width(), image_->height());
	//	relate_window_->setWindowFlags(Qt::Dialog);
	//	relate_window_->setFixedSize(QSize(image_->width(), image_->height()));
	//	relate_window_->setWindowFlags(Qt::SubWindow);

		//image_->invertPixels(QImage::InvertRgb);
		//*(image_) = image_->mirrored(true, true);
		//*(image_) = image_->rgbSwapped();
	cout << "image size: " << image_->width() << ' ' << image_->height() << endl;
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}

	image_->save(filename);
}

void ImageWidget::Invert()
{
	for (int i = 0; i < image_->width(); i++)
	{
		for (int j = 0; j < image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			image_->setPixel(i, j, qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color)));
		}
	}

	// equivalent member function of class QImage
	// image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(image_));
	int width = image_->width();
	int height = image_->height();

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, height - 1 - j));
				}
			}
		}
		else
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(i, height - 1 - j));
				}
			}
		}

	}
	else
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(image_) = image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
	for (int i = 0; i < image_->width(); i++)
	{
		for (int j = 0; j < image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			int gray_value = (qRed(color) + qGreen(color) + qBlue(color)) / 3;
			image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value));
		}
	}

	update();
}

void ImageWidget::Restore()
{
	*(image_) = *(image_backup_);
	point_start_ = point_end_ = QPoint(0, 0);
	update();
}

