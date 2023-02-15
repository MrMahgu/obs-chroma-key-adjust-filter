#pragma once

#include <QWidget>
#include <QPainter>
#include <QLinearGradient>
#include <QDebug>
#include <QMouseEvent>
#include <QFrame>
#include <QColor>
#include <QPair>
#include <QImage>
#include <QPixmap>
#include <QRgb>

#include "ui_widget.h"

typedef void (*test_callback)(void);

class Widget : public QWidget {
	Q_OBJECT

public:
	Widget(QWidget *parent = nullptr);
	~Widget();

	void test(test_callback func);

signals:
	void closed();
	void colorChanged(const QColor &color);

private:
	void resizeEvent(QResizeEvent *event);

	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

protected:
	void paintColorPicker(QPainter *painter);
	void paintBaseColorPicker(QPainter *painter);

	void paintBaseColor(QPainter *painter);
	void paintMouseColor(QPainter *painter);

	void paintColorNotification(QPainter *painter);
	void paintBaseColorNotification(QPainter *painter);

protected:
	void closeEvent(QCloseEvent *event);
	void paintEvent(QPaintEvent *event);

private:
	Ui::Widget *ui;

	QColor selectedColor;
	QColor baseColor;

	QColor currentMouseColor;
	QColor currentBaseMouseColor;

	QFrame *colorPickerFrame;
	QFrame *baseColorPickerFrame;

	QFrame *selectedColorFrame;
	QFrame *mouseColorFrame;

	int _pos_x;
	int _pos_y;
	bool leftMouseDown;

private:
	QRect colorPickerFrameRect;
	QRect baseColorPickerFrameRect;

	QRect selectedColorFrameRect;
	QRect mouseColorFrameRect;
};
