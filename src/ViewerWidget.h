#pragma once
#include <QtWidgets>

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
		bool isInside(int x, int y) { return (x >= 0 && y >= 0 && x <= img->width() && y <= img->height()) ? true : false; }

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


		uchar* getData() { return data; }
		void setDataPtr() { data = img->bits(); }
		void setPainter() { painter = new QPainter(img); }

		int getImgWidth() { return img->width(); };
		int getImgHeight() { return img->height(); };

		//Helper functions
		double dotProduct(QPoint a, QPoint b) {return a.x() * b.x() + a.y() * b.y(); }
		// Algorithms
		void DDALine(QPoint start, QPoint end, QColor color);
		void BresenhamLine(QPoint start, QPoint end, QColor color);
		void BresenhamCircle(QPoint start, QPoint end, QColor color);
		void Cyrus_Beck(QColor color);
		void Sutherland_Hodgeman(QColor color);
		
		//Transfomations
		void Translation(int dx, int dy, QColor color); //Orez·vanie
		void Rotation(int angle, QColor color); // Ot·Ëanie
		void Render(QVector<QPoint> list, QColor color); //Vykreslenie
		void Scale(double sx, double sy, QColor color); //Zv‰töenie
		void Shear(double shx, QColor color); //Skosenie
		void Flip(QColor color);
		void clear();

	public slots:
		void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};

