#ifndef QTTESTE_H
#define QTTESTE_H

#include <QMainWindow>
#include <QtGui>
#include <QtCore>
#include <QDateTime>
#include "ui_gdaq.h"

#include "texo.h"
#include "texo_def.h"

#include "daq.h"
namespace Ui {
class gdaq;
}

class gdaq : public QMainWindow
{
    Q_OBJECT

public:
    explicit gdaq(QWidget *parent = nullptr);
    ~gdaq();

	// prototypes
	//bool selectProbe(int connector); //sele��o de porta do transdutor
	bool createSequence(int sequence); //sele��o da sequ�ncia de transmiss�o
	bool run(); //inicia a aquisi��o de dados pelo DAQ e pelo Texo
	bool stop(); //finaliza a aquisi��o de dados pelo DAQ e pelo Texo
	bool newImage(void*, unsigned char*, int);
	bool sequenceFixedTX(bool custom);//sequ�ncia com transmiss�o fixa
	//bool sequenceMovingTX();//sequ�ncia com transmiss�o m�vel
	//bool sequenceMovingRX();//sequ�ncia com recep��o m�vel
	bool sequencePARX(bool custom);//sequ�ncia sem transmiss�o, somente com recep��o, para aquisi��o de imagens fotoac�sticas 
	bool sequenceCPWC(bool custom);//sequ�ncia de emiss�o com angula��o na onda plana
	bool sequencePAPE(bool custom);//sequ�ncia de PA e PE intercalada

	bool sequenceDAQ();//sequ�ncia para configura��o de par�metros do DAQ
	
	void printStats();//mostra as estat�sticas do Texo: Frame size, Frame rate e Buffer size
	bool saveData(QString inpath);//armazena os dados adquiridos pelo DAQ no disco r�gido
	void wait();//aguarda a digita��o de qualquer tecla toda vez que um item do menu � finalizado
	void frame_counter();//mostra o n�mero de frames adquiridos pelo DAQ
	void trigger_select(); //item do menu para sele��o de trigger
	void divisor_select(); //item do menum para sele��o do divisor- Tamanho do Buffer do DAQ = 16GB/2^divisor (bytes)


	// global variables
	texo tex;
	sonixdaq* daq;
	

	char shape[96]; //vari�vel de forma do pulso de transmiss�o

	bool running;//variavel que mostra a situa��o da aquisi��o de dados pelo DAQ ("rodando"=true, "parado"=false) 
	bool validprobe;//vari�vel que mostra se foi escolhido uma porta da SonixRP em que se encontra um transdutor (transdutor v�lido = true)
	bool validsequence;//vari�vel que mostra se foi escolhida uma sequ�ncia de aquisi��o v�lida (sequ�ncia v�lida = true)
	bool abort;
	bool PA_sequence;
	bool PE_sequence;
	//CPWC
	bool cpwc;
	int cpwc_control;
	int cpwc_vet[24];
	int max_angle;
	//log
	//vetor de parametros
	int out_parameters[7];
	//PAPE
	bool PAPE;

	bool custom_sequence;

	bool sampling_80MHz; //vari�vel para selecionar taxa de amostragem igual a 80MHz
	int depth; //vari�vel de profundidade (mm)
	int divisor_var; //vari�vel de tamanho do buffer - Tamanho do Buffer do DAQ = 16GB/2^divisor_var (bytes) 
	int max_frame_buffer; //numero m�ximo de frames para encher o Buffer do DAQ, lembrando que Tamanho do Buffer do DAQ = 16GB/2^divisor_var (bytes)
	int pwx;
	float gn;
	int trigger_in;
	int trigger_out;
	unsigned int channelsDAQ[4];
	int frameCount;
	int max_frame;
	float buffer_daq;
	QString fileName;
	// DAQ paprams
	daqTGC tgc; //configura��o do TGC
	daqRaylinePrms rlprms; //configura��o da linha de aquisi��o do DAQ
	daqSequencePrms seqprms; // configura��o da sequ�ncia do DAQ

	// PROBE
	char * probeName;
	int probeConn;
	int probeCFreq;
	int probeSize;


	

	

private:
    Ui::gdaq ui;
public slots:
    //ON - OFF
	void on_pb_config_daq_clicked();
    void on_pb_reset_daq_clicked();
	
	//Probe
	void on_pb_probe_clicked();
	void on_sb_probe_valueChanged(int pb);

	//Controls
	void on_sb_Power_valueChanged(int x);
	void on_dsb_Gain_valueChanged(double x);
	void on_sb_Depth_valueChanged(int x);
	void on_sb_divisor_valueChanged(int x);
	//sync
	void on_cb_Trigger_in_currentIndexChanged(int x);
	void on_cb_Trigger_out_currentIndexChanged(int x);
	//sequence
	void on_rb_PE_toggled(bool sel);
	void on_rb_PA_toggled(bool sel);
	void on_rb_CPWC_toggled(bool sel);
	void on_rb_PAPE_toggled(bool sel);
	//Run
	void on_pb_run_clicked();
	//Stop
	void on_pb_stop_clicked(); 
	//Save
	void on_pb_save_clicked();

	//TGC Curve
	void on_cb_fixedTGC_stateChanged(int x);

	//TX parameters
	void on_tb_PE_clicked(); 
	void on_tb_PA_clicked(); 
	//void on_tb_CPWC_clicked();
	void on_tb_PAPE_clicked(); 

	//Delay
	void sDelay(int s);
	void msDelay(int ms);

	//Power Test
	void set_power(int pwx);
	//Sync func
	void set_sync(int in, int out);


	//About
	void on_actionGIIMUS_Laboratory_triggered();
	void on_actionAuthor_triggered();
	
	
};

#endif // QTTESTE_H
//QObject::connect(pb_tx_parameters, SIGNAL(clicked()), TxParameters, SLOT(accept()));