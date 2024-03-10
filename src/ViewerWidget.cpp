#include   "ViewerWidget.h"

ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent) : QWidget(parent){
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget(){
	delete painter;
	delete img;
}
void ViewerWidget::resizeWidget(QSize size){
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg){
	if (img != nullptr) {
		delete painter;
		delete img;
	}
	img = new QImage(inputImg);
	if (!img) {
		return false;
	}
	resizeWidget(img->size());
	setPainter();
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty(){
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}
bool ViewerWidget::changeSize(int width, int height){
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete painter;
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
		update();
	}

	return true;
}
void ViewerWidget::setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a){
	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = b;
	data[startbyte + 1] = g;
	data[startbyte + 2] = r;
	data[startbyte + 3] = a;
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA){
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(255 * valB);
	data[startbyte + 1] = static_cast<uchar>(255 * valG);
	data[startbyte + 2] = static_cast<uchar>(255 * valR);
	data[startbyte + 3] = static_cast<uchar>(255 * valA);
}
void ViewerWidget::setPixel(int x, int y, const QColor& color){
	if (color.isValid()) {
		size_t startbyte = y * img->bytesPerLine() + x * 4;

		data[startbyte] = color.blue();
		data[startbyte + 1] = color.green();
		data[startbyte + 2] = color.red();
		data[startbyte + 3] = color.alpha();
	}
}

// Algorithms
void ViewerWidget::DDALine(QPoint start, QPoint end, QColor color) {
	float dx = end.x() - start.x();
	float dy = end.y() - start.y();

	// Calculate the number of steps
	float steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

	// Increment for each step
	float xinc = dx / steps;
	float yinc = dy / steps;

	float x = start.x();
	float y = start.y();

	for (int i = 0; i < steps; i++) {
		// Check if coordinates are within the clipping region
		if (isInside(round(x), round(y))) {
			setPixel(round(x), round(y), color);
		}

		x += xinc;
		y += yinc;
	}
}
void ViewerWidget::BresenhamLine(QPoint start, QPoint end, QColor color) {
	int x0 = start.x();
	int y0 = start.y();
	int x1 = end.x();
	int y1 = end.y();

	// výpoèet smernice
	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);
	int step_x = (x0 < x1) ? 1 : -1;
	int step_y = (y0 < y1) ? 1 : -1;

	int error = dx - dy;

	while (true) {
		setPixel(x0, y0, color);
		if (x0 == x1 && y0 == y1)
			break;
		int double_error = 2 * error;

		if (double_error > -dy) {
			error -= dy;
			x0 += step_x;
		}
		if (double_error < dx) {
			error += dx;
			y0 += step_y;
		}
	}
}
void ViewerWidget::BresenhamCircle(QPoint start, QPoint end, QColor color) {
	int r = sqrt(pow(end.x() - start.x(), 2) + pow(end.y() - start.y(), 2)); //polomer kružnice
	int x = 0;
	int y = r;
	int p = 1 - r;

	while (x <= y) {
		// kontrola hraníc kružnice
		if (isInside(start.x() + x, start.y() + y)) setPixel(start.x() + x, start.y() + y, color);
		if (isInside(start.x() - x, start.y() + y)) setPixel(start.x() - x, start.y() + y, color);
		if (isInside(start.x() + x, start.y() - y)) setPixel(start.x() + x, start.y() - y, color);
		if (isInside(start.x() - x, start.y() - y)) setPixel(start.x() - x, start.y() - y, color);
		if (isInside(start.x() + y, start.y() + x)) setPixel(start.x() + y, start.y() + x, color);
		if (isInside(start.x() - y, start.y() + x)) setPixel(start.x() - y, start.y() + x, color);
		if (isInside(start.x() + y, start.y() - x)) setPixel(start.x() + y, start.y() - x, color);
		if (isInside(start.x() - y, start.y() - x)) setPixel(start.x() - y, start.y() - x, color);

		if (p > 0) {
			p += 2 * (x - y) + 5;
			y--;
		}
		else {
			p += 2 * x + 3;
		}
		x++;
	}
}
void ViewerWidget::Cyrus_Beck(QColor color) {
	QPoint D = points[1] - points[0];
	double tl = 0;
	double tu = 1;
	for (int i = 0; i < 1; i++) {
		QPoint E0 = points[0];
		QPoint E1 = points[1];
		QPoint E(E1 - E0);
		QPoint N(-E.y(), E.x());
		double dotDN = dotProduct(D, N);
		qDebug() << dotDN;
		double dotWN = dotProduct((points[0] - points[1]), N);

		if (dotDN != 0) {
			double t = -dotWN / dotDN;
			if (dotDN > 0 && t <= 1) {
				tl = std::max(t, tl);
			}
			if (dotDN < 0 && t >= 0) {
				tu = std::min(t, tu);
			}
		}
	}
	if (tl == 0 && tu == 1) {
		drawLine(points[0], points[1], color, 1);
		return;
	}
	else if (tl < tu) {
		qDebug() << "Here5";
		QPoint newStart = lines[0].p1() + (lines[0].p2() - lines[0].p1()) * tl;
		QPoint newEnd = lines[0].p1() + (lines[0].p2() - lines[0].p1()) * tu;
		drawLine(newStart, newEnd, color, 0);
	}
}

//Draw functions
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType){
	painter->setPen(QPen(color));
	// DDA algoritm
	if (algType == 0) {
		DDALine(start, end, color);
	}
	//Bresenhamov algoritm pre priamku - celoèíselná aritmetika
	else if (algType == 1) {
		BresenhamLine(start, end, color);
	}
	update();
}
void ViewerWidget::drawCircle(QPoint start, QPoint end, QColor color){

	// Bresenhamov algoritmus pre kružnicu
	painter->setPen(QPen(color));
	BresenhamCircle(start, end, color);
	update();
}

//Transformations
void ViewerWidget::Translation(int dx, int dy, QColor color) {
	for (int i = 0; i < points.size(); i++) {
		setPoint(i, points[i].x() + dx, points[i].y() + dy);
	}
	for (int i = 0; i < lines.size(); i++) {
		lines[i].setP1(QPoint(lines[i].p1().x() + dx, lines[i].p1().y() + dy));
		lines[i].setP2(QPoint(lines[i].p2().x() + dx, lines[i].p2().y() + dy));
	}
	qDebug() << points;
	qDebug() << lines;
	clear();
	for (int i = 0; i < points.size() - 1; i++) {
		if (isInside(points[i].x(), points[i].y()) && isInside(points[i + 1].x(), points[i + 1].y())) {
			drawLine(points[i], points[i + 1], color, 0);
		}
		else {
			qDebug() << "Here";
			Cyrus_Beck(color);
		}
	}
	if (isInside(points[points.size() - 1].x(), points[points.size() - 1].y()) && isInside(points[0].x(), points[0].y())) {
		drawLine(points[points.size() - 1], points[0], color, 0);
	}
	update();
}

//Clear
void ViewerWidget::clear(){
	img->fill(Qt::white);
	update();
}
//Slots
void ViewerWidget::paintEvent(QPaintEvent* event){
	QPainter painter(this);
	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}

