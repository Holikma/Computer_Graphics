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
	// Define vector D as the vector from the first point to the second point
	QPoint D = points[1] - points[0];
	
	// Initialize tl (t lower bound) to 0 and tu (t upper bound) to 1
	double tl = 0;
	double tu = 1;
	
	// Define the boundary of the clipping region (in this case, a rectangle)
	QVector<QPoint> plane = { QPoint(0, 0), QPoint(0, img->height()), QPoint(img->width(), img->height()), QPoint(img->width(), 0) };
	
	// Iterate over each edge of the clipping rectangle
	for (int i = 0; i < 4; i++) {
		QPoint E0 = plane[i];
		QPoint E1 = plane[(i + 1) % 4];

		// Define the vector representing the current edge (E1 - E0)
		QPoint E(E1 - E0);

		// Calculate the inside normal vector N to the current edge
		QPoint N(E.y(), -E.x());
		double dotDN = dotProduct(D, N);
		double dotWN = dotProduct((points[0] - E0), N);

		// If the dot product of D and N is not 0 (i.e., the line is not parallel to the edge)
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
void ViewerWidget::Translation(int dx, int dy, QColor color) {
	if (points.size() == 0) {
		return;
	}
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
	if (points.size() == 0 || points.size() == 1) {
		return;
	}
	if (points.size() == 2) {
		int new_x = points[0].x() - 2 * (points[1].x() - points[0].x());
		points[1].setX(new_x);
	}
	else {
		double A = points[1].y() - points[0].y();
		double B = points[0].x() - points[1].x();
		double C = -A * points[0].x() - B * points[0].y();
		for (int i = 2; i < points.size(); i++) {
			double d = A * points[i].x() + B * points[i].y() + C;
			double new_x = points[i].x() - 2 * A * d / (A * A + B * B);
			double new_y = points[i].y() - 2 * B * d / (A * A + B * B);
			setPoint(i, new_x, new_y);
			
		}
	}
	Render(points, color);
}

bool ViewerWidget::Comp_points(QPoint p1, QPoint p2) {
	if (p1.y() != p2.y())
		return p1.y() < p2.y();
	else
		return p1.x() < p2.x(); 
}
bool ViewerWidget::isInsideTriangle(QPoint A, QPoint B, QPoint C, QPoint P){
	int as_x = P.x() - A.x();
	int as_y = P.y() - A.y();

	// Určíme, na ktorej strane priamky AB sa bod P nachádza
	bool s_ab = (B.x() - A.x()) * as_y - (B.y() - A.y()) * as_x > 0;

	// Ak sa bod P nachádza na opačnej strane priamky AC, nie je vnútri trojuholníka
	if ((C.x() - A.x()) * as_y - (C.y() - A.y()) * as_x > 0 == s_ab) return false;

	// Ak sa bod P nachádza na opačnej strane priamky BC, nie je vnútri trojuholníka
	if ((C.x() - B.x()) * (P.y() - B.y()) - (C.y() - B.y()) * (P.x() - B.x()) > 0 != s_ab) return false;

	return true;
}
struct Edge {
		QPoint start;
		QPoint end;
		double m;
		int delta_y;
		double x;
		double w;
};
void ViewerWidget::Scan_Line(QColor color) {
	QVector<QPoint> polygon = points;
	QVector<Edge> edges;

	for (int i = 0; i < polygon.size(); i++) {
		int next = (i + 1) % polygon.size();
		if (polygon[i].y() == polygon[next].y())
			continue;
		QPoint start = polygon[i].y() < polygon[next].y() ? polygon[i] : polygon[next];
		QPoint end = polygon[i].y() < polygon[next].y() ? polygon[next] : polygon[i];

		end.setY(end.y() - 1);
		double dx = (end.x() - start.x());
		if (dx == 0) dx = 1/DBL_MAX;

		double m = (double)(end.y() - start.y()) / dx;
		Edge edge = { start, end, m};
		edges.push_back(edge);
	}
	
	std::sort(edges.begin(), edges.end(), [](const Edge& e1, const Edge& e2) {
		return e1.start.y() < e2.start.y();
	});

	int ymin = edges.front().start.y();
	int ymax = edges.front().end.y();

	for (int i = 1; i < edges.size(); i++) {
		if (edges[i].start.y() < ymin)
			ymin = edges[i].start.y();
		if (edges[i].end.y() > ymax)
			ymax = edges[i].end.y();
	}

	QVector<QVector<Edge>> TH(ymax - ymin + 1);

	if (TH.size() < 2) return;

	for (int i = 0; i < edges.size(); i++) {
		int delta_y = edges[i].end.y() - edges[i].start.y();
		double x = edges[i].start.x();
		double w = 0;
		if (edges[i].m == 0) {
			w = 0;
		}
		else {
			w = (double)1.0/edges[i].m;
		}
		edges[i].delta_y = delta_y;
		edges[i].x = x;
		edges[i].w = w;
		
		TH[(edges[i].start.y() - ymin)].push_back(edges[i]);
	}

	QVector<Edge> ZAH;
	int y = ymin;

	for (int i = 0; i < ymax - ymin; i++) {

		if (!TH[i].empty()) {
			for (const Edge& edge : TH[i]) {
				ZAH.push_back(edge);
			}
		}

		std::sort(ZAH.begin(), ZAH.end(), [](const Edge& e1, const Edge& e2) {
			return e1.x < e2.x;
			});

		for (int j = 0; j < ZAH.size(); j += 2) {

			if (j + 1 < ZAH.size() && ZAH[j].x != ZAH[j + 1].x) {

				for (int k = round(ZAH[j].x); k <= round(ZAH[j + 1].x); k++) {
					if (isInside(k,y)) {
						setPixel(k, y, color);
					}
				}
			}
		}
		for (int j = 0; j < ZAH.size(); j++) {
			if (ZAH[j].delta_y == 0) {
				ZAH.erase(ZAH.begin() + j);
				j--;
			}
		}

		for (int j = 0; j < ZAH.size(); j++) {
			ZAH[j].delta_y--;
			ZAH[j].x += ZAH[j].w;
		}
		y++;
	}
}
void ViewerWidget::Triangle_Fill(QVector<QPoint> lists, int algType) {
	QVector<QPoint> sorted = lists;

	std::sort(sorted.begin(), sorted.end(), [](const QPoint& p1, const QPoint& p2) {
		return p1.y() < p2.y() || (p1.y() == p2.y() && p1.x() < p2.x());
		});

	QPoint T0 = sorted[0];
	QPoint T1 = sorted[1];
	QPoint T2 = sorted[2];

	QColor C0 = Qt::blue;
	QColor C1 = Qt::red;
	QColor C2 = Qt::green;

	if (T0.y() == T1.y()) { //lower triangle
		for (int y = T1.y(); y <= T2.y(); y++) {
				int x1 = ((y - T0.y()) * (T2.x() - T0.x()) / (T2.y() - T0.y()) + T0.x());
				int x2 = ((y - T1.y()) * (T2.x() - T1.x()) / (T2.y() - T1.y()) + T1.x());

				for (int x = x1; x <= x2; x++) {
					QPoint K(x, y);
					if (algType == 0) {
						double d0 = distance(T0, K);
						double d1 = distance(T1, K);
						double d2 = distance(T2, K);
						if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
							if (d0 < d1 && d0 < d2) {
								setPixel(x, y, C0);
							}
							else if (d1 < d0 && d1 < d2) {
								setPixel(x, y, C1);
							}
							else {
								setPixel(x, y, C2);
							}
						}
					}
					else {
						if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
							Barycentric(T0, T1, T2, K, C0, C1, C2); 
						}
					}
				}
			}
	}
	else if (T1.y() == T2.y()) { //upper triangle
		for (int y = T0.y(); y <= T1.y(); y++) {
			int x1 = ((y - T0.y()) * (T1.x() - T0.x()) / (T1.y() - T0.y()) + T0.x());
			int x2 = ((y - T0.y()) * (T2.x() - T0.x()) / (T2.y() - T0.y()) + T0.x());
			for (int x = x1; x <= x2; x++) {
				QPoint K(x, y);
				if (algType == 0) {
					double d0 = distance(T0, K);
					double d1 = distance(T1, K);
					double d2 = distance(T2, K);
					if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
						if (d0 < d1 && d0 < d2) {
							setPixel(x, y, C0);
						}
						else if (d1 < d0 && d1 < d2) {
							setPixel(x, y, C1);
						}
						else {
							setPixel(x, y, C2);
						}
					}
				}
				else {
					if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
						Barycentric(T0, T1, T2, K, C0, C1, C2);
					}
				}
			}
		}
	}
	else { //general case
		double m = (double)(T2.y() - T0.y()) / (T2.x() - T0.x());
		QPoint P((T1.y() - T0.y()) / m + T0.x(), T1.y());
		if (T1.x() < P.x()) {
			for (int y = T0.y(); y <= T1.y(); y++) {
				int x1 = ((y - T0.y()) * (T1.x() - T0.x()) / (T1.y() - T0.y()) + T0.x());
				int x2 = ((y - T0.y()) * (P.x() - T0.x()) / (P.y() - T0.y()) + T0.x());
				for (int x = x1; x <= x2; x++) {
					QPoint K(x, y);
					if (algType == 0) {
						double d0 = distance(T0, K);
						double d1 = distance(T1, K);
						double d2 = distance(T2, K);
						if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
							if (d0 < d1 && d0 < d2) {
								setPixel(x, y, C0);
							}
							else if (d1 < d0 && d1 < d2) {
								setPixel(x, y, C1);
							}
							else {
								setPixel(x, y, C2);
							}
						}
					}
					else {
						if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
							Barycentric(T0, T1, T2, K, C0, C1, C2);
						}
					}
				}
			}
			for (int y = T1.y(); y <= T2.y(); y++) {
				int x1 = ((y - T1.y()) * (T2.x() - T1.x()) / (T2.y() - T1.y()) + T1.x());
				int x2 = ((y - P.y()) * (T2.x() - P.x()) / (T2.y() - P.y()) + P.x());

				for (int x = x1; x <= x2; x++) {
					QPoint K(x, y);
					if (algType == 0) {
						double d0 = distance(T0, K);
						double d1 = distance(T1, K);
						double d2 = distance(T2, K);
						if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
							if (d0 < d1 && d0 < d2) {
								setPixel(x, y, C0);
							}
							else if (d1 < d0 && d1 < d2) {
								setPixel(x, y, C1);
							}
							else {
								setPixel(x, y, C2);
							}
						}
					}
					else {
						if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
							Barycentric(T0, T1, T2, K, C0, C1, C2);
						}
					}
				}
			}
		}
		else {
			for (int y = T0.y(); y <= T1.y(); y++) {
				int x1 = ((y - T0.y()) * (P.x() - T0.x()) / (P.y() - T0.y()) + T0.x());
				int x2 = ((y - T0.y()) * (T1.x() - T0.x()) / (T1.y() - T0.y()) + T0.x());

				for (int x = x1; x <= x2; x++) {
					QPoint K(x, y);
					if (algType == 0) {
						double d0 = distance(T0, K);
						double d1 = distance(T1, K);
						double d2 = distance(T2, K);
						if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
							if (d0 < d1 && d0 < d2) {
								setPixel(x, y, C0);
							}
							else if (d1 < d0 && d1 < d2) {
								setPixel(x, y, C1);
							}
							else {
								setPixel(x, y, C2);
							}
						}
					}
					else {
						if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
							Barycentric(T0, T1, T2, K, C0, C1, C2);
						}
					}
				}
			}
			for (int y = T1.y(); y <= T2.y(); y++) {
				int x1 = ((y - P.y()) * (T2.x() - P.x()) / (T2.y() - P.y()) + P.x());
				int x2 = ((y - T1.y()) * (T2.x() - T1.x()) / (T2.y() - T1.y()) + T1.x());
				for (int x = x1; x <= x2; x++) {
					QPoint K(x, y);
					if (algType == 0) {
						double d0 = distance(T0, K);
						double d1 = distance(T1, K);
						double d2 = distance(T2, K);
						if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
							if (d0 < d1 && d0 < d2) {
								setPixel(x, y, C0);
							}
							else if (d1 < d0 && d1 < d2) {
								setPixel(x, y, C1);
							}
							else {
								setPixel(x, y, C2);
							}
						}
					}
					else {
						if (isInside(x, y) && isInsideTriangle(T0, T1, T2, K)) {
							Barycentric(T0, T1, T2, K, C0, C1, C2);
						}
					}
				}
			}
		}
	}
}
void ViewerWidget::Fill(QVector<QPoint> lists, int algType, QColor color) {
	if (lists.size() == 3) {
		Triangle_Fill(lists, algType);
	}
	else if (lists.size() > 3) {
		Scan_Line(color);
	}
}
void ViewerWidget::Barycentric(QPoint A, QPoint B, QPoint C, QPoint P, QColor C0, QColor C1, QColor C2) {
	// Area ABC
	double A_tri = qAbs((A.x() * (B.y() - C.y()) + B.x() * (C.y() - A.y()) + C.x() * (A.y() - B.y())) / 2.0);
	// Area PBC
	double A0_tri = qAbs((P.x() * (B.y() - C.y()) + B.x() * (C.y() - P.y()) + C.x() * (P.y() - B.y())) / 2.0);
	// Area APC
	double A1_tri = qAbs((A.x() * (P.y() - C.y()) + P.x() * (C.y() - A.y()) + C.x() * (A.y() - P.y())) / 2.0);
	// Area ABP
	double A2_tri = qAbs((A.x() * (B.y() - P.y()) + B.x() * (P.y() - A.y()) + P.x() * (A.y() - B.y())) / 2.0);
	// Calculate barycentric coordinates
	double lambda0 = A0_tri / A_tri;
	double lambda1 = A1_tri / A_tri;
	double lambda2 = A2_tri / A_tri;

	// Interpolate color
	double red = lambda0 * C0.redF() + lambda1 * C1.redF() + lambda2 * C2.redF();
	double green = lambda0 * C0.greenF() + lambda1 * C1.greenF() + lambda2 * C2.greenF();
	double blue = lambda0 * C0.blueF() + lambda1 * C1.blueF() + lambda2 * C2.blueF();
	setPixel(P.x(), P.y(), QColor::fromRgbF(red, green, blue));

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
void ViewerWidget::Hermit(QColor color) {
	if (points.size() < 1) {
		return;
	}
	else {
		QVector<QPoint> tangents;
		for (int i = 0; i < points.size(); i++) {
			if (i == 0) tangents.push_back((points[i + 1] - points[i]) / 2);
			else if (i == points.size() - 2) tangents.push_back((points[i] - points[i - 1]) / 2);
			else tangents.push_back(((points[i + 1] - points[i]) / 2) + ((points[i] - points[i - 1])/2));
			
		}
		for (int i = 0; i < tangents.size(); i++) {
			drawLine(points[i], points[i] + tangents[i], color, 0);
		}
		double delta_t = 0.0001;
		for (int i = 1; i < points.size(); i++) {
			QPoint P0 = points[i - 1];
			QPoint P1 = points[i];
			QPoint T0 = tangents[i - 1];
			QPoint T1 = tangents[i];
			double t = delta_t;
			while (t < 1) {
				QPoint Q1 = P0 * (2 * t * t * t - 3 * t * t + 1) + P1 * (-2 * t * t * t + 3 * t * t) +
					T0 * (t * t * t - 2 * t * t + t) + T1 * (t * t * t - t * t);
				drawLine(P0, Q1, color, 0);
				Q1.setX(round(Q1.x()));
				Q1.setY(round(Q1.y()));
				t += delta_t;
				P0 = Q1;
			}
			drawLine(P0, P1, color, 0);
		}
	}
	update();
}
void ViewerWidget::Bezier(QColor color) {
	
}
void ViewerWidget::DrawCurves(QVector<QPoint> list, QColor color, int algType) {
	if (list.size() == 0) {
		return;
	}
	if (list.size() == 1) {
		setPoint(0, list[0].x(), list[0].y());
	}
	else {
		if (algType == 0) {
			Hermit(color);
		}
	}
}
