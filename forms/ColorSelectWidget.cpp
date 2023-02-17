#include "ColorSelectWidget.h"
#include "ui_ColorSelectWidget.h"

// TODO make pretty someday

ColorSelectWidget::ColorSelectWidget(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::Widget),
	  selectedColor(Qt::green),
	  baseColor(Qt::green),
	  currentMouseColor(Qt::green),
	  currentBaseMouseColor(Qt::green),
	  colorPickerFrame(nullptr),
	  baseColorPickerFrame(nullptr),
	  _pos_x(0),
	  _pos_y(0),
	  _outside_pos_x(0),
	  _outside_pos_y(0),
	  leftMouseDown(false)
{
	ui->setupUi(this);

	// Setup tracking on this widget
	this->setMouseTracking(true);

	// Grab QFrames
	colorPickerFrame = this->findChild<QFrame *>("colorPickerFrame"),
	baseColorPickerFrame =
		this->findChild<QFrame *>("baseColorPickerFrame");
	selectedColorFrame = this->findChild<QFrame *>("selectedColorFrame");

	// Setup QFrame rects
	colorPickerFrameRect = colorPickerFrame->geometry();
	baseColorPickerFrameRect = baseColorPickerFrame->geometry();
	selectedColorFrameRect = selectedColorFrame->geometry();

	// Setup mouse events
	colorPickerFrame->setMouseTracking(true);
	baseColorPickerFrame->setMouseTracking(true);
	selectedColorFrame->setMouseTracking(true);
}

ColorSelectWidget::~ColorSelectWidget()
{
	delete ui;
}

void ColorSelectWidget::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
	// do nothing
}

void ColorSelectWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->type() != QEvent::MouseButtonPress)
		return;
	if (event->button() != Qt::LeftButton)
		return;

	leftMouseDown = true;
	QPoint pos = event->pos();

	// actualColor
	QRect *pc = &colorPickerFrameRect;
	if (pc->contains(event->pos())) {
		_pos_x = pos.x();
		_pos_y = pos.y();
		QRgb pixel = ProcessPixel(_pos_x, _pos_y);
		if (leftMouseDown) {
			emit colorChanged(QColor(pixel));
			this->selectedColor = QColor(pixel);
			this->update();
		}
		return;
	}

	// baseColor
	QRect *sc = &baseColorPickerFrameRect;
	if (sc->contains(event->pos())) {
		QRgb pixel = ProcessPixel(pos.x(), pos.y());
		emit colorChanged(QColor(pixel));
		this->selectedColor = QColor(pixel);
		this->baseColor = QColor(pixel);
		this->update();
		return;
	}
}

void ColorSelectWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->type() != QEvent::MouseButtonRelease)
		return;
	if (event->button() != Qt::LeftButton)
		return;
	leftMouseDown = false;
}

QRgb ColorSelectWidget::ProcessPixel(int x, int y)
{
	QPixmap pixmap = this->grab(QRect(x, y, 1, 1));
	QImage image = pixmap.toImage();
	QRgb pixel = image.pixel(0, 0);
	return pixel;
}

bool ColorSelectWidget::ProcessOutsideRegion(int &aPtr, int &bPtr, bool left,
					     int a, int b, int x, int y, int z)
{
	bool res = left ? a < x : a > x;
	if (res) {
		aPtr = x;
		if (b < bPtr && b > y) {
			bPtr = b;
		} else if (b < bPtr && b < y) {
			bPtr = y;
		} else if (b > bPtr && b < z) {
			bPtr = b;
		} else if (b > bPtr && b > z) {
			bPtr = z;
		}
		return true;
	}
	return false;
}

void ColorSelectWidget::mouseMoveEvent(QMouseEvent *event)
{
	QRect *pc = &colorPickerFrameRect;
	QPoint pos = event->pos();

	// Primary
	if (pc->contains(event->pos())) {

		QRgb pixel = ProcessPixel(pos.x(), pos.y());
		this->currentMouseColor = QColor(pixel);
		if (leftMouseDown) {
			emit colorChanged(QColor(pixel));
			_pos_x = pos.x();
			_pos_y = pos.y();
			this->selectedColor = this->currentMouseColor;
		}
		this->update();
		return;
	}

	// baseColor
	QRect *sc = &baseColorPickerFrameRect;
	if (sc->contains(event->pos())) {
		QRgb pixel = ProcessPixel(pos.x(), pos.y());
		if (leftMouseDown) {
			emit colorChanged(QColor(pixel));
			this->baseColor = QColor(pixel);
			this->selectedColor = QColor(pixel);
			this->update();
		}
		return;
	}

	// Below will handle outside the bounds of the quad as long as
	// the mouse was pressed while inside and then dragged outside without
	// being released
	// This allows you to capture the colours in the corners/sides/etc

	if (!leftMouseDown)
		return;

	_outside_pos_x = pos.x();
	_outside_pos_y = pos.y();

	QPoint tl = pc->topLeft();
	QPoint bl = pc->bottomLeft();
	QPoint tr = pc->topRight();
	QPoint br = pc->bottomRight();

	bool updated = false;
	int _margin = 1;

	// Process ouside the region of the quad
	
	// Left
	if (!updated)
		updated = ProcessOutsideRegion(
			_pos_x, _pos_y, true, _outside_pos_x, _outside_pos_y,
			tl.x() + _margin, tl.y() + _margin, bl.y() - _margin);
	// Right
	if (!updated)
		updated = ProcessOutsideRegion(
			_pos_x, _pos_y, false, _outside_pos_x, _outside_pos_y,
			tr.x() - _margin, tr.y() + _margin, br.y() - _margin);
	// Top
	if (!updated)
		updated = ProcessOutsideRegion(
			_pos_y, _pos_x, true, _outside_pos_y, _outside_pos_x,
			tl.y() + _margin, tl.x() + _margin, tr.x() - _margin);
	// Bottom
	if (!updated)
		updated = ProcessOutsideRegion(
			_pos_y, _pos_x, false, _outside_pos_y, _outside_pos_x,
			bl.y() - _margin, tl.x() + _margin, tr.x() - _margin);

	// Handle anything we might have done "outside"
	if (updated) {
		QRgb pixel = ProcessPixel(_pos_x, _pos_y);
		this->currentMouseColor = QColor(pixel);
		if (leftMouseDown) {
			this->selectedColor = this->currentMouseColor;
		}
		this->update();
	}
}

void ColorSelectWidget::paintColorPicker(QPainter *painter)
{
	painter->fillRect(colorPickerFrameRect,
			  QBrush(this->baseColor, Qt::SolidPattern));

	QRect *pc = &colorPickerFrameRect;

	QLinearGradient linearGradient(QPointF(pc->left(), pc->top()),
				       QPointF(pc->left() + pc->width(),
					       pc->top()));
	linearGradient.setColorAt(0.01, Qt::white);
	linearGradient.setColorAt(0.99, Qt::transparent);
	painter->fillRect(colorPickerFrameRect, linearGradient);

	QLinearGradient linearGradient2(QPointF(pc->left(), pc->top()),
					QPointF(pc->left(),
						pc->top() + pc->height()));
	linearGradient2.setColorAt(0.01, Qt::transparent);
	linearGradient2.setColorAt(0.99, Qt::black);
	painter->fillRect(colorPickerFrameRect, linearGradient2);
}

void ColorSelectWidget::paintBaseColorPicker(QPainter *painter)
{
	QRect *sc = &baseColorPickerFrameRect;

	QLinearGradient linearGradient(QPointF(sc->left(), 0),
				       QPointF(sc->left() + sc->width(), 0));
	linearGradient.setColorAt(0.01, Qt::red);
	linearGradient.setColorAt(0.15, Qt::magenta);
	linearGradient.setColorAt(0.16, Qt::magenta);
	linearGradient.setColorAt(0.33, Qt::blue);
	linearGradient.setColorAt(0.34, Qt::blue);
	linearGradient.setColorAt(0.49, Qt::cyan);
	linearGradient.setColorAt(0.50, Qt::cyan);
	linearGradient.setColorAt(0.66, Qt::green);
	linearGradient.setColorAt(0.67, Qt::green);
	linearGradient.setColorAt(0.83, Qt::yellow);
	linearGradient.setColorAt(0.84, Qt::yellow);
	linearGradient.setColorAt(0.99, Qt::red);
	painter->fillRect(baseColorPickerFrameRect, linearGradient);
}

void ColorSelectWidget::paintBaseColor(QPainter *painter)
{
	painter->fillRect(selectedColorFrameRect,
			  QBrush(this->selectedColor, Qt::SolidPattern));
}

void ColorSelectWidget::paintMouseColor(QPainter *painter)
{
	painter->fillRect(mouseColorFrameRect,
			  QBrush(this->currentMouseColor, Qt::SolidPattern));
}

void ColorSelectWidget::paintColorNotification(QPainter *painter)
{

	painter->setPen(QPen(QColor(0xFF0000)));

	int w = 4;

	painter->drawPoint(_pos_x, _pos_y - w);
	painter->drawPoint(_pos_x - w, _pos_y);
	painter->drawPoint(_pos_x + w, _pos_y);
	painter->drawPoint(_pos_x, _pos_y + w);
}

void ColorSelectWidget::paintBaseColorNotification(QPainter *painter)
{
	Q_UNUSED(painter);
}

void ColorSelectWidget::closeEvent(QCloseEvent *event)
{
	emit closed();
	QWidget::closeEvent(event);
}

void ColorSelectWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	paintColorPicker(&painter);
	paintBaseColorPicker(&painter);

	paintBaseColor(&painter);
	paintMouseColor(&painter);

	paintColorNotification(&painter);
}
