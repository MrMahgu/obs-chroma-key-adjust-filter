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
#include <QPoint>

// TODO make pretty someday

#include "ui_ColorSelectWidget.h"

class ColorSelectWidget : public QWidget {
	Q_OBJECT

public:
	ColorSelectWidget(QWidget *parent = nullptr);
	~ColorSelectWidget();

signals:
	void closed();
	void colorChanged(const QColor &color);

private:
	void resizeEvent(QResizeEvent *event);

	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

private:
	QRgb ProcessPixel(int x, int y);
	bool ProcessOutsideRegion(int &aPtr, int &bPtr, bool left, int a, int b,
				  int x, int y, int z);

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
	Ui::ColorSelectWidget *ui;

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

	int _outside_pos_x;
	int _outside_pos_y;

	bool leftMouseDown;

private:
	QRect colorPickerFrameRect;
	QRect baseColorPickerFrameRect;

	QRect selectedColorFrameRect;
	QRect mouseColorFrameRect;
};
