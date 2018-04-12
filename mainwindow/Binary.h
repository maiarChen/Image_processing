#pragma once

#include <QWidget>
#include "ui_Binary.h"

class Binary : public QWidget
{
	Q_OBJECT

public:
	Binary(QWidget *parent = Q_NULLPTR);
	~Binary();
public slots:
	void send();
signals:
	void BinaryThres(int);
	void closeAndPush();
private:
	Ui::Binary *ui;
	void closeEvent(QCloseEvent *event);
};
