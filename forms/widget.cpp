#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
	: QWidget(parent),
	  ui(new Ui::Widget),
	  selectedColor(Qt::green),
	  baseColor(Qt::green),
	  currentMouseColor(Qt::green),
	  currentBaseMouseColor(Qt::green),
	  colorPickerFrame(nullptr),
	  baseColorPickerFrame(nullptr),
	  selectedColorFrame(nullptr),
	  mouseColorFrame(nullptr),
	  _pos_x(0),
	  _pos_y(0),
	  leftMouseDown(false)
{
	setAttribute(Qt::WA_DeleteOnClose, true);

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

Widget::~Widget()
{
	delete ui;
}

void Widget::test(test_callback func)
{
	func();
}

void Widget::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
	// do nothing
}

void Widget::mousePressEvent(QMouseEvent *event)
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
		QPixmap pixmap = this->grab(QRect(_pos_x, _pos_y, 1, 1));
		QImage image = pixmap.toImage();
		QRgb pixel = image.pixel(0, 0);
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
		QPixmap pixmap = this->grab(QRect(pos.x(), pos.y(), 1, 1));
		QImage image = pixmap.toImage();
		QRgb pixel = image.pixel(0, 0);
		emit colorChanged(QColor(pixel));
		this->selectedColor = QColor(pixel);
		this->baseColor = QColor(pixel);
		this->update();
		return;
	}
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->type() != QEvent::MouseButtonRelease)
		return;
	if (event->button() != Qt::LeftButton)
		return;
	leftMouseDown = false;
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
	QRect *pc = &colorPickerFrameRect;
	QPoint pos = event->pos();

	// Primary
	if (pc->contains(event->pos())) {
		QPixmap pixmap = this->grab(QRect(pos.x(), pos.y(), 1, 1));
		QImage image = pixmap.toImage();
		QRgb pixel = image.pixel(0, 0);
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
		QPixmap pixmap = this->grab(QRect(pos.x(), pos.y(), 1, 1));
		QImage image = pixmap.toImage();
		QRgb pixel = image.pixel(0, 0);
		if (leftMouseDown) {
			emit colorChanged(QColor(pixel));
			this->baseColor = QColor(pixel);
			this->selectedColor = QColor(pixel);
			this->update();
		}
		return;
	}
}

void Widget::paintColorPicker(QPainter *painter)
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

void Widget::paintBaseColorPicker(QPainter *painter)
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

void Widget::paintBaseColor(QPainter *painter)
{
	painter->fillRect(selectedColorFrameRect,
			  QBrush(this->selectedColor, Qt::SolidPattern));
}

void Widget::paintMouseColor(QPainter *painter)
{
	painter->fillRect(mouseColorFrameRect,
			  QBrush(this->currentMouseColor, Qt::SolidPattern));
}

void Widget::paintColorNotification(QPainter *painter)
{

	painter->setPen(QPen(QColor(0xFF0000)));

	int w = 4;

	painter->drawPoint(_pos_x, _pos_y - w);
	painter->drawPoint(_pos_x - w, _pos_y);
	painter->drawPoint(_pos_x + w, _pos_y);
	painter->drawPoint(_pos_x, _pos_y + w);
}

void Widget::paintBaseColorNotification(QPainter *painter)
{
	Q_UNUSED(painter);
}

void Widget::closeEvent(QCloseEvent *event)
{
	emit closed();
	QWidget::closeEvent(event);
}

void Widget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);

	paintColorPicker(&painter);
	paintBaseColorPicker(&painter);

	paintBaseColor(&painter);
	paintMouseColor(&painter);

	paintColorNotification(&painter);
	//paintBaseColorNotification(&painter);
}
