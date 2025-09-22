#ifndef TXCPWC_H
#define TXCPWC_H

#include <QDialog>
//#include <QImage>
#include "ui_txparameters.h"

class TxCPWC : public QDialog, public Ui::TxCPWC
{
	Q_OBJECT

public:
	TxCPWC(QWidget *parent = 0);
	~TxCPWC();

	void loadImage(QString str);

};

#endif // TXCPWC_H