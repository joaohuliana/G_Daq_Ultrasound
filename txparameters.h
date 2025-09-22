#ifndef TXPARAMETERS_H
#define TXPARAMETERS_H

#include <QDialog>
//#include <QImage>
#include "ui_txparameters.h"

class TxParameters : public QDialog, public Ui::TxParameters
{
	Q_OBJECT

public:
	TxParameters(QWidget *parent = 0);
	~TxParameters();

	void loadImage(QString str);

};

#endif // TXPARAMETERS_H