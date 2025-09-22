#ifndef CPWCPARAMETERS_H
#define CPWCPARAMETERS_H

#include <QDialog>
//#include <QImage>
#include "ui_cpwcparameters.h"

class cpwcParameters : public QDialog, public Ui::cpwcParameters
{
	Q_OBJECT

public:
	cpwcParameters(QWidget *parent = 0);
	~cpwcParameters();

	void loadImage(QString str);

};

#endif // CPWCPARAMETERS_H