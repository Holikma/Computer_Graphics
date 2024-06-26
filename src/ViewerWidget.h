#pragma once
#include <QtWidgets>
#include <stdio.h>
#include <iostream>


class ViewerWidget :public QWidget {
	Q_OBJECT
	private:
		QSize areaSize = QSize(0, 0);
		QImage* img = nullptr;
		QPainter* painter = nullptr;
		uchar* data = nullptr;

		bool drawLineActivated = false;
		bool drawCircleActivated = false;

		bool isDragging = false;
		QVector<QPoint> points;

		QPoint DragStart = QPoint(0, 0);
		QPoint drawLineBegin = QPoint(0, 0);
		QPoint drawCircleBegin = QPoint(0, 0);

	public:
		ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
		~ViewerWidget();
		void resizeWidget(QSize size);

		//Image functions
		bool setImage(const QImage& inputImg);
		QImage* getImage() { return img; };
		bool isEmpty();
		bool changeSize(int width, int height);
		bool isInside(int x, int y) { return (x >= 0 && y >= 0 && x < img->width() && y < img->height()) ? true : false; }

		//Pixel functions
		void setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a = 255);
		void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
		void setPixel(int x, int y, const QColor& color);

		//Draw functions
		void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
		void drawCircle(QPoint start, QPoint end, QColor color);

		//Set functions
		void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
		void setDrawCircleBegin(QPoint begin) { drawCircleBegin = begin; }
		void setDrawLineActivated(bool state) { drawLineActivated = state; }
		void setDrawCircleActivated(bool state) { drawCircleActivated = state; }
		void setDragStart(QPoint start) { DragStart = start; }
		void setDragging(bool state) { isDragging = state; }

		//Get functions
		bool getDrawLineActivated() { return drawLineActivated; }
		bool getDrawCircleActivated() { return drawCircleActivated; }
		QPoint getDrawLineBegin() { return drawLineBegin; }
		QPoint getDrawCircleBegin() { return drawCircleBegin; }
		QPoint getDragStart() { return DragStart; }
		bool getDragging() { return isDragging; }


		//Vector functions
		QVector<QPoint> getPoints() { return points; }
		void AddPoint(QPoint point) { points.append(point); }
		void setPoint(int index, int x, int y) { points[index].setX(x); points[index].setY(y); }
		QPoint getPoint(int index) { return points[index]; }
		void clearPoints() { points.clear(); }
		void SmallCircleAroundPoint(int x, int y, QColor color){ drawCircle(QPoint(x, y), QPoint(x + 3, y + 3), color); }

		uchar* getData() { return data; }
		void setDataPtr() { data = img->bits(); }
		void setPainter() { painter = new QPainter(img); }

		int getImgWidth() { return img->width(); };
		int getImgHeight() { return img->height(); };

		//Helper functions
		double dotProduct(QPoint a, QPoint b) {return a.x() * b.x() + a.y() * b.y(); }
		double distance(QPoint a, QPoint b) { return sqrt(pow(a.x() - b.x(), 2) + pow(a.y() - b.y(), 2)); }
		bool isInsideTriangle(QPoint A, QPoint B, QPoint C, QPoint P);
		double sign(QPoint p1, QPoint p2, QPoint p3) { return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y()); }
		bool Comp_points(QPoint p1, QPoint p2);
		
		// Algorithms
		void DDALine(QPoint start, QPoint end, QColor color);
		void BresenhamLine(QPoint start, QPoint end, QColor color);
		void BresenhamCircle(QPoint start, QPoint end, QColor color);
		void Cyrus_Beck(QColor color);
		QVector<QPoint> Sutherland_Hodgeman(QColor color);
		
		//Transfomations
		void Translation(int dx, int dy, QColor color); //Orez�vanie
		void Rotation(int angle, QColor color); // Ot��anie
		void Render(QVector<QPoint> list, QColor color); //Vykreslenie
		void Scale(double sx, double sy, QColor color); //Zv�t�enie
		void Shear(double shx, QColor color); //Skosenie
		void Flip(QColor color);
		void Scan_Line(QColor color);
		void Triangle_Fill(QVector<QPoint> lists, int algType);
		void Fill(QVector<QPoint> lists, int algType, QColor color);
		void Barycentric(QPoint A, QPoint B, QPoint C, QPoint P, QColor C0, QColor C1, QColor C2);
		void clear();

		void DrawCurves(QVector<QPoint> list, QColor color, int algType);
		void Hermit(QColor color);
		void Bezier(QColor color);


	public slots:
		void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};

