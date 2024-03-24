#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget* parent) : QMainWindow(parent), ui(new Ui::ImageViewerClass){
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(500, 500));

	ui->scrollArea->setWidget(vW);
	
	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setAlignment(Qt::AlignCenter);
	ui->scrollArea->setWidgetResizable(true);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	DisableTools();


	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	globalColor = Qt::blue;
	globalFillColor = Qt::red;
	QString style_sheet = QString("background-color: #%1;").arg(globalColor.rgba(), 0, 16);
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
	QString Fill_style_sheet = QString("background-color: #%1;").arg(globalFillColor.rgba(), 0, 16);
	ui->pushButtonSetFillColor->setStyleSheet(Fill_style_sheet);
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event){
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return false;
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event){
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event) {
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if (e->button() == Qt::LeftButton && ui->toolButtonDrawLine->isChecked()) {
		if (w->getDrawLineActivated()) {
			w->drawLine(w->getDrawLineBegin(), e->pos(), globalColor, ui->comboBoxLineAlg->currentIndex());
			w->setDrawLineActivated(false);
		}
		else {
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawCircle->isChecked())
	{
		if (w->getDrawCircleActivated()) {
			w->drawCircle(w->getDrawCircleBegin(), e->pos(), globalColor);
			w->setDrawCircleActivated(false);
		}
		else {
			w->setDrawCircleBegin(e->pos());
			w->setDrawCircleActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), globalColor);
			w->update();
		}
	}
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawPolygon->isChecked()) {
		w->AddPoint(e->pos());
		w->setPixel(e->pos().x(), e->pos().y(), globalColor);
		if (w->getPoints().size() > 1) {
			w->drawLine(w->getPoints()[w->getPoints().size() - 2], w->getPoints()[w->getPoints().size() - 1], globalColor, ui->comboBoxLineAlg->currentIndex());
		}
		w->update();
	}
	if (e->button() == Qt::RightButton && ui->toolButtonDrawPolygon->isChecked()) {
		w->drawLine(w->getPoints()[w->getPoints().size() - 1], w->getPoints()[0], globalColor, ui->comboBoxLineAlg->currentIndex());
		w->setDragging(true);
		ui->toolButtonTranslation->setChecked(true);
		ui->toolButtonDrawPolygon->setChecked(false);
		ui->toolButtonDrawPolygon->setEnabled(false);
		EnableTools();
		w->update();
	}
	if (e->button() == Qt::LeftButton && ui->pushButtonCurves->isChecked()) {
		w->AddPoint(e->pos());
		w->SmallCircleAroundPoint(e->pos().x(), e->pos().y(), globalColor);
	}
	if (e->button() == Qt::RightButton && ui->pushButtonCurves->isChecked()) {
		w->DrawCurves(w->getPoints(), globalColor, ui->comboBoxCurves->currentIndex());
		ui->pushButtonCurves->setChecked(false);
		w->update();
	}
}
void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event){
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event) {
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	static QPoint delta;
	if (e->buttons() & Qt::LeftButton && w->getDragging()) {
		setCursor(Qt::ClosedHandCursor);
		delta = e->pos() - w->getDragStart();
		w->Translation(delta.x(), delta.y(), globalColor);
		if (ui->toolButtonFill->isChecked()) {
				w->Fill(vW->getPoints(), ui->FillAlgorithm->currentIndex(), globalFillColor);
		}
		w->setDragStart(e->pos());
		w->update();
	}
	else {
		w->setDragStart(e->pos());
		setCursor(Qt::ArrowCursor); // Change cursor to indicate no dragging
	}
}

void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event){}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event){}
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event){
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
}
//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event){
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}
//Image functions
bool ImageViewer::openImage(QString filename){
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename){
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}
//Slots
void ImageViewer::on_actionOpen_triggered(){
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}
void ImageViewer::on_actionSave_as_triggered(){
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered(){
	vW->clear();
	vW->clearPoints();
	DisableTools();
	ui->toolButtonDrawCircle->setEnabled(true);
	ui->toolButtonDrawCircle->setChecked(false);
	ui->toolButtonDrawLine->setEnabled(true);
	ui->toolButtonDrawLine->setChecked(false);
	ui->toolButtonDrawPolygon->setEnabled(true);
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonTranslation->setChecked(false);
	ui->comboBoxLineAlg->setEnabled(true);
	vW->setDragging(false);
}
void ImageViewer::on_actionExit_triggered(){
	this->close();
}
void ImageViewer::on_pushButtonSetColor_clicked(){
	QColor newColor = QColorDialog::getColor(globalColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		globalColor = newColor;
	}
}
void ImageViewer::on_toolButtonRotation_clicked() {
	vW->Rotation(ui->spinBoxRot->value(), globalColor);	
	if (ui->toolButtonFill->isChecked()) {
		vW->Fill(vW->getPoints(), ui->FillAlgorithm->currentIndex(), globalFillColor);
	}
	vW->update();
}
void ImageViewer::on_toolButtonScale_clicked() {
	vW->Scale(ui->doubleSpinBoxScaleX->value(), ui->doubleSpinBoxScaleY->value(), globalColor);
	if (ui->toolButtonFill->isChecked()) {
		vW->Fill(vW->getPoints(), ui->FillAlgorithm->currentIndex(), globalFillColor);
	}
	vW->update();
}
void ImageViewer::on_toolButtonShear_clicked() {
	vW->Shear(ui->doubleSpinBoxShearX->value(),  globalColor);
	if (ui->toolButtonFill->isChecked()) {
		vW->Fill(vW->getPoints(), ui->FillAlgorithm->currentIndex(), globalFillColor);
	}
	vW->update();
}
void ImageViewer::on_toolButtonFlip_clicked() {
	vW->Flip(globalColor);
	if (ui->toolButtonFill->isChecked()) {
		vW->Fill(vW->getPoints(), ui->FillAlgorithm->currentIndex(), globalFillColor);
	}
	vW->update();
}
void ImageViewer::on_toolButtonDrawLine_clicked() {
	if (ui->toolButtonDrawLine->isChecked()) {
		ui->toolButtonDrawCircle->setEnabled(false);
		ui->toolButtonDrawPolygon->setEnabled(false);
		ui->comboBoxLineAlg->setEnabled(false);
	}
	else {
		ui->toolButtonDrawCircle->setEnabled(true);
		ui->toolButtonDrawPolygon->setEnabled(true);
		ui->comboBoxLineAlg->setEnabled(true);
	}
}
void ImageViewer::on_toolButtonDrawCircle_clicked() {
	if (ui->toolButtonDrawCircle->isChecked()) {
		ui->toolButtonDrawLine->setEnabled(false);
		ui->toolButtonDrawPolygon->setEnabled(false);
		ui->comboBoxLineAlg->setEnabled(false);
	}
	else {
		ui->toolButtonDrawLine->setEnabled(true);
		ui->toolButtonDrawPolygon->setEnabled(true);
		ui->comboBoxLineAlg->setEnabled(true);
	}
}
void ImageViewer::on_toolButtonDrawPolygon_clicked() {
	if (ui->toolButtonDrawPolygon->isChecked()) {
		ui->toolButtonDrawCircle->setEnabled(false);
		ui->toolButtonDrawLine->setEnabled(false);
		ui->comboBoxLineAlg->setEnabled(false);
	}
	else {
		ui->toolButtonDrawCircle->setEnabled(true);
		ui->toolButtonDrawLine->setEnabled(true);
		ui->comboBoxLineAlg->setEnabled(true);
	}
}
void ImageViewer::on_pushButtonSetFillColor_clicked() {
	QColor newColor = QColorDialog::getColor(globalFillColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetFillColor->setStyleSheet(style_sheet);
		globalFillColor = newColor;
	}
}
void ImageViewer::on_toolButtonTranslation_clicked() {
	if (ui->toolButtonTranslation->isChecked()) {		
		vW->setDragging(true);	
	}
	else {
		vW->setDragging(false);
	}
}
void ImageViewer::on_toolButtonFill_clicked() {
	if (ui->toolButtonFill->isChecked()) {
		vW->Fill(vW->getPoints(), ui->FillAlgorithm->currentIndex(), globalFillColor);
	}
	else {
		vW->Render(vW->getPoints(), globalColor);
	}
	vW->update();
}
void ImageViewer::DisableTools() {
	ui->toolButtonTranslation->setEnabled(false);
	ui->toolButtonFill->setEnabled(false);
	ui->toolButtonFlip->setEnabled(false);
	ui->toolButtonRotation->setEnabled(false);
	ui->toolButtonScale->setEnabled(false);
	ui->toolButtonShear->setEnabled(false);
	ui->FillAlgorithm->setEnabled(false);
}
void ImageViewer::EnableTools() {
	ui->toolButtonTranslation->setEnabled(true);
	ui->toolButtonFill->setEnabled(true);
	ui->toolButtonFlip->setEnabled(true);
	ui->toolButtonRotation->setEnabled(true);
	ui->toolButtonScale->setEnabled(true);
	ui->toolButtonShear->setEnabled(true);
	ui->FillAlgorithm->setEnabled(true);
}
void ImageViewer::on_pushButtonCurves_clicked() {
}