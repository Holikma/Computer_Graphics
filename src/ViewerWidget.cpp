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

	// výpočet smernice
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
	QVector<QPoint> plane = { QPoint(0, 0), QPoint(0, img->height()), QPoint(img->width(), img->height()), QPoint(img->width(), 0) };

	for (int i = 0; i < 4; i++) {
		QPoint E0 = plane[i];
		QPoint E1 = plane[(i + 1) % 4];
		QPoint E(E1 - E0);

		QPoint N(E.y(), -E.x());
		double dotDN = dotProduct(D, N);
		double dotWN = dotProduct((points[0] - E0), N);

		if (dotDN != 0) {
			double t = -dotWN / dotDN;
			if (dotDN > 0 && t <= 1) {
				tl = std::max(t, tl);
			}
			else if (dotDN < 0 && t >= 0) {
				tu = std::min(t, tu);
			}
		}
	}
	if (tl == 0 && tu == 1) {
		drawLine(points[0], points[1], color, 0);
	}

	else if (tl < tu) {
		QPoint newStart = points[0] + D * tl;
		QPoint newEnd = points[0] + D * tu;
		drawLine(newStart, newEnd, color, 1);
	}
}
void ViewerWidget::Sutherland_Hodgeman(QColor color) {
	QVector<QPoint> W;
	QVector<QPoint> polygon = points;
	int edges[4] = {0,0,-499,-499};

	for (int j = 0; j < 4; j++) {
		if (polygon.size() == 0) {
			break;
		}
		QPoint S = polygon[polygon.size() -1]; // Initialize S to the last vertex
		double xmin = edges[j];
		for (int i = 0; i < polygon.size(); i++) {
			QPoint Vi = polygon[i];

			if (Vi.x() >= xmin) {
				if (S.x() >= xmin) {
					W.append(Vi);
				}
				else {
					QPoint P = QPoint(xmin, S.y() + (xmin - S.x()) * (Vi.y() - S.y()) / (double)(Vi.x() - S.x()));
					W.append(P);
					W.append(Vi);
				}
			}
			else {
				if (S.x() >= xmin) {
					QPoint P = QPoint(xmin, S.y() + (xmin - S.x()) * (Vi.y() - S.y()) / (double)(Vi.x() - S.x()));
					W.append(P);
				}
			}
			S = Vi;
		}
		polygon = W;
		W.clear();
		for (int k = 0; k < polygon.size(); k++){
			QPoint swap = polygon[k];
			polygon[k].setX(swap.y());
			polygon[k].setY(-swap.x());
		}
	}

	for (int i = 0; i < polygon.size() - 1; i++) {
		drawLine(polygon[i], polygon[i + 1], color, 0);
	}
	if (polygon.size() > 1) {
		drawLine(polygon[polygon.size() - 1], polygon[0], color, 0);
	}

	update();

}

void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType) {
	painter->setPen(QPen(color));
	// DDA algoritm
	if (algType == 0) {
		DDALine(start, end, color);
	}
	//Bresenhamov algoritm pre priamku - celočíselná aritmetika
	else if (algType == 1) {
		BresenhamLine(start, end, color);
	}
	update();
}
void ViewerWidget::drawCircle(QPoint start, QPoint end, QColor color) {

	// Bresenhamov algoritmus pre kružnicu
	painter->setPen(QPen(color));
	BresenhamCircle(start, end, color);
	update();
}
void ViewerWidget::Render(QVector<QPoint> list, QColor color) {
	clear();
	for (int i = 0; i < list.size() - 1; i++) {
		if (isInside(list[i].x(), list[i].y()) && isInside(list[i + 1].x(), list[i + 1].y())) {
			drawLine(list[i], list[i + 1], color, 0);
		}
		else {
			if (list.size() == 2) {
				 
					Cyrus_Beck(color);
				
			}
			else if (list.size() > 2) {
				if (isInside(list[i].x(), list[i].y()) || isInside(list[i + 1].x(), list[i + 1].y())) {
					Sutherland_Hodgeman(color);
				}
			}
		}
	}
	if (isInside(list[list.size() - 1].x(), list[list.size() - 1].y()) && isInside(list[0].x(), list[0].y())) {
		drawLine(list[list.size() - 1], list[0], color, 0);
	}
	else {
		if (list.size() > 2) {
			if (isInside(list[list.size() - 1].x(), list[list.size() - 1].y()) || isInside(list[0].x(), list[0].y())) {
				Sutherland_Hodgeman(color);
			}
		}
	}
	update();
}
//Transformations
void ViewerWidget::Translation(int dx, int dy, QColor color) {
	for (int i = 0; i < points.size(); i++) {
		setPoint(i, points[i].x() + dx, points[i].y() + dy);
	}
	Render(points, color);
	update();
}
void ViewerWidget::Rotation(int Angle, QColor color) {
	// rotation based on first point
	double angle = Angle * M_PI / 180;
	QPoint center = points[0];
	for (int i = 1; i < points.size(); i++) {
		int x = center.x() + (points[i].x() - center.x()) * cos(angle) - (points[i].y() - center.y()) * sin(angle);
		int y = center.y() + (points[i].x() - center.x()) * sin(angle) + (points[i].y() - center.y()) * cos(angle);
		setPoint(i, x, y);
	}
	Render(points, color);
}
void ViewerWidget::Scale(double sx, double sy, QColor color) {
	QPoint center = points[0];
	for (int i = 1; i < points.size(); i++) {
		int x = center.x() + (points[i].x() - center.x()) * sx;
		int y = center.y() + (points[i].y() - center.y()) * sy;
		setPoint(i, x, y);
	}
	Render(points, color);
}
void ViewerWidget::Shear(double shx, QColor color) {
	QPoint center = points[0];
	for (int i = 1; i < points.size(); i++) {
		double x = points[i].x() + shx * points[i].y();
		int y = points[i].y();
		setPoint(i, round(x), y);
	}
	Render(points, color);
}
void ViewerWidget::Flip(QColor color) {
	//flip based on first line
	QPoint start = points[0];
	QPoint end = points[1];
	QPoint center = QPoint((start.x() + end.x()) / 2, (start.y() + end.y()) / 2);
	for (int i = 2; i < points.size(); i++) {
		int x = center.x() - (points[i].x() - center.x());
		int y = center.y() - (points[i].y() - center.y());
		setPoint(i, x, y);
	}
	Render(points, color);
}

bool ViewerWidget::Comp_points(QPoint p1, QPoint p2) {
	if (p1.y() != p2.y())
		return p1.y() < p2.y(); 
	else
		return p1.x() < p2.x(); 
}

void ViewerWidget::Scan_Line(QColor color) {

	struct Edge {
		QPoint start;
		QPoint end;
		double m;
	};

	QVector<QPoint> polygon = points;

	std::sort(polygon.begin(), polygon.end(), [this](QPoint p1, QPoint p2) {return Comp_points(p1, p2);});
	QVector<Edge> edges;

	// Calculate edges from the sorted polygon points
	for (int i = 0; i < polygon.size(); ++i) {
		int next = (i + 1) % polygon.size(); // Get the next point index

		// Skip horizontal edges
		if (polygon[i].y() == polygon[next].y())
			continue;

		// Ensure start.y < end.y
		QPoint start = polygon[i].y() < polygon[next].y() ? polygon[i] : polygon[next];
		QPoint end = polygon[i].y() < polygon[next].y() ? polygon[next] : polygon[i];

		double m = static_cast<double>(end.x() - start.x()) / (end.y() - start.y()); // Calculate slope

		edges.push_back({ start, end, m });
	}

	// Sort the edges based on their y-coordinate
	std::sort(edges.begin(), edges.end(), [](const Edge& e1, const Edge& e2) {
		return e1.start.y() < e2.start.y();
		});

	// Initialize ymin and ymax
	int ymin = edges.empty() ? 0 : edges.front().start.y();
	int ymax = edges.empty() ? 0 : edges.back().start.y();

	// Create an edge table
	QVector<QVector<Edge>> edgeTable(ymax - ymin + 1);

	// Populate the edge table
	for (const auto& edge : edges) {
		int y = edge.start.y();
		edgeTable[y - ymin].push_back(edge);
	}

	// Create a list of active edges
	QVector<Edge> activeEdges;

	// Start scanline algorithm
	for (int y = ymin; y <= ymax; ++y) {
		// Move edges from edge table to active edges
		for (const auto& edge : edgeTable[y - ymin]) {
			activeEdges.push_back(edge);
		}

		// Sort active edges by x-coordinate
		std::sort(activeEdges.begin(), activeEdges.end(), [](const Edge& e1, const Edge& e2) {
			return e1.start.x() < e2.start.x();
			});

		// Process pairs of active edges
		for (int j = 0; j < activeEdges.size()-1; j++) {
			int x1 = activeEdges[j].start.x();
			int x2 = activeEdges[j + 1].start.x();
			// Fill pixels between x1 and x2 at current y
			for (int x = x1; x <= x2; ++x) {
				setPixel(x, y, color);
			}
			// Update edge values
			activeEdges[j].start.setY(activeEdges[j].start.y() + 1);
			activeEdges[j].start.setX(activeEdges[j].start.x() + activeEdges[j].m);

			activeEdges[j + 1].start.setY(activeEdges[j + 1].start.y() + 1);
			activeEdges[j + 1].start.setX(activeEdges[j + 1].start.x() + activeEdges[j + 1].m);
		}

		// Remove edges with Δy = 0
		activeEdges.erase(std::remove_if(activeEdges.begin(), activeEdges.end(), [y](const Edge& edge) {
			return edge.end.y() <= y;
			}), activeEdges.end());
	}
}


//Clear
void ViewerWidget::clear() {
	img->fill(Qt::white);
	update();
}
//Slots
void ViewerWidget::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}
