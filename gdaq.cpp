#include "gdaq.h"
#include "ui_gdaq.h"
#include "chooseprobe.h"
#include "txparameters.h"
#include "cpwcparameters.h"
//#include "txcpwc.h"
#include "StdAfx.h"
#include <math.h>
#include <fstream>

#define DAQ_FIRMWARE_PATH "D:\\MAO\\daq\\fw\\"
// constants
#ifdef SONIXTOUCH
#define NUMCHANNELS 64
#else
#define NUMCHANNELS 32
#endif

#define MAXELEMENTS 128 //máximo número de elementos




gdaq::gdaq(QWidget *parent) :
QMainWindow(parent)
	//ui(new Ui::QtTeste)

{
	setWindowIcon(QIcon(":/icon/Icone2.ico"));
	ui.setupUi(this);//linka a tela principal

	ui.pb_config_daq->setEnabled(1);//habilita o botão de configurar e inciciar o DAQ
	ui.pb_reset_daq->setEnabled(0);// desabilita o botão de resetar as configurações do DAQ

	/*Global Variables*/
	PA_sequence = false;//variável da sequencia photoacoustic
	PE_sequence = false;//variável da sequencia pulso eco
	PAPE=false;//variável da sequencia PA-PE
	cpwc = false; // variável da sequencia CPWC Coherent Plane Wave Compounding
	custom_sequence = false;

	running = false;//variável do DAQ/Texo "rodando"
	validprobe = false;//variável que mostra se foi escolhido uma porta da SonixRP em que se encontra um transdutor (transdutor válido = true)
	validsequence = false;//variável que mostra se foi escolhida uma sequência de aquisição válida (sequência válida = true)
	abort = false;//variável de abortar a aquisição
	sampling_80MHz = false; //variável para selecionar taxa de amostragem igual a 80MHz
	divisor_var = 10; //variável de tamanho do buffer - Tamanho do Buffer do DAQ = 16GB/2^divisor_var (bytes) 
	max_frame_buffer = 0; //numero máximo de frames para encher o Buffer do DAQ, lembrando que Tamanho do Buffer do DAQ = 16GB/2^divisor_var (bytes)
	//channelsDAQ[4] = { 0, 0, 0, 0 };
	ui.seqname->setText(QString("G DAQ V1.0"));//texto inicial do system info
	
	
	//variavel de controle do angulo no CPWC
	cpwc_control = 0;
	max_angle = 4;
	
	//Vetor de angulos usados no CPWC
	cpwc_vet[0] = -24000;
    cpwc_vet[1] = -22000;
    cpwc_vet[2] = -20000;
    cpwc_vet[3] = -18000;
    cpwc_vet[4] = -16000;
    cpwc_vet[5] = -14000;
    cpwc_vet[6] = -12000;
    cpwc_vet[7] = -10000;
    cpwc_vet[8] = -8000;
    cpwc_vet[9] = -6000;
    cpwc_vet[10] = -4000;
	cpwc_vet[11] = -2000;
	cpwc_vet[12] = 0;
	cpwc_vet[13] = 2000;
	cpwc_vet[14] = 4000;
	cpwc_vet[15] = 6000;
	cpwc_vet[16] = 8000;
	cpwc_vet[17] = 10000;
	cpwc_vet[18] = 12000;
	cpwc_vet[19] = 14000;
	cpwc_vet[20] = 16000;
	cpwc_vet[21] = 18000;
	cpwc_vet[22] = 20000;
	cpwc_vet[23] = 22000;
	cpwc_vet[24] = 24000;
	
	



}

//TGC curve
void gdaq::on_cb_fixedTGC_stateChanged(int x)
{
	if(x)//abilita ou desabilita a curva de TGC
	{
		ui.TGCbox->setEnabled(0);
	}
	else
	{
		ui.TGCbox->setEnabled(1);
	}

}


//Configure DAQ
void gdaq::on_pb_config_daq_clicked()
{
	// Configure DAQ OFF
	ui.pb_config_daq->setEnabled(0);
	//Reset DAQ ON
	ui.pb_reset_daq->setEnabled(1);
	
	//Boxes Sync and Power OFF
	ui.sb_Power->setEnabled(0);
	ui.boxSync->setEnabled(0);
	ui.sb_cpwc_main_angle->setEnabled(0);


	//char sel;//variável de seleção do menu principal

	#ifdef SONIXTOUCH
		// initialize and set the data file path
		printf("<<Sonix Touch system>> \n\n");
		printf("-Initializing Texo: ... ");
		if (!tex.init(FIRMWARE_PATH, 3, 3, 0, NUMCHANNELS, 0))
		{
			scanf("%c", &sel);
			return -1;
		}

	#else    // RP
		// initialize and set the data file path
		//QMessageBox msgBox;
		//msgBox.setText("SonixRP ultrasound system at 40MHz");
		//msgBox.exec();	

		for (int i = 1; i <= 2; i++)
				{
				ui.seqname->setText(QString("Initializing Texo: ."));
				msDelay(333);
				ui.seqname->setText(QString("Initializing Texo: .."));
				msDelay(333);
				ui.seqname->setText(QString("Initializing Texo: ..."));
				msDelay(333);
				}
		if (!tex.init(FIRMWARE_PATH, 2, 2, 0, NUMCHANNELS, 0)) //definir canais e caminho.
			{
			ui.seqname->setText(QString("Initializing Texo: ... Failed "));
			}
		else
			{
				for (int i = 1; i <= 3; i++)
				{
				ui.seqname->setText(QString("Texo initilized:  %1 secs").arg(3-i));
				sDelay(1);
				}

			}
	#endif

				
			
	daq = sonixdaq::getInstance();
	
    if (!daq->isConnected())//caso DAQ não esteja conectado mostrar mensagem
    {
		ui.seqname->setText(QString("DAQ not connected or off"));
                
    }
    else//senão mostrar que está conectado
    {
        
		for (int i = 1; i <= 2; i++)
				{
				ui.seqname->setText(QString("DAQ conected... Programming."));
				msDelay(333);
				ui.seqname->setText(QString("DAQ conected... Programming.."));
				msDelay(333);
				ui.seqname->setText(QString("DAQ conected... Programming..."));
				msDelay(333);
				}

		if (!daq->isInitialized())//caso DAQ ainda nao esteja inicializado
		{
			daq->setFirmwarePath(DAQ_FIRMWARE_PATH);//carregar arquivos do firmware do DAQ para inicialização do DAQ
			if (!daq->init(sampling_80MHz))
			{
				printf(daq->getLastError().c_str());

				ui.seqname->setText(daq->getLastError().c_str());
				on_pb_reset_daq_clicked();

			}
			else
			{
				for (int i = 1; i <= 3; i++)
				{
					ui.seqname->setText(QString("DAQ initilized: %1 secs").arg(3-i));
					sDelay(1);
				}
				ui.seqname->setText(QString(""));
			}
		}
	
		else//senão informar que o DAQ já está inicializado
		{
			for (int i = 1; i <= 3; i++)
				{
					ui.seqname->setText(QString("DAQ already initilized: %1 secs").arg(3-i));
					sDelay(1);
				}
				ui.seqname->setText(QString(""));
		}
		// Select PROBE
		on_sb_probe_valueChanged(ui.sb_probe->value());
		//Set Depth
		on_sb_Depth_valueChanged(ui.sb_Depth->value());
		//Set Power
		set_power(ui.sb_Power->value());
		max_angle = ui.sb_cpwc_main_angle->value();
		//Set Trigger
		on_cb_Trigger_in_currentIndexChanged(ui.cb_Trigger_in->currentIndex());
		set_sync(trigger_in,trigger_out);
		//Set Fixed Sequence
		on_rb_PE_toggled(true);
		//boxes Probe, Run, Save, Sequence ON
		ui.boxProbe->setEnabled(1);
		ui.pb_run->setEnabled(1);
		ui.pb_save->setEnabled(1);
		ui.dsb_Gain->setEnabled(1);
		ui.sb_Depth->setEnabled(1);
		ui.sb_divisor->setEnabled(1);
		ui.boxSeq->setEnabled(1);
		
		//Boxes Trigger OFF
		ui.cb_Trigger_out->setEnabled(0);
		ui.cb_Trigger_in->setEnabled(0);
		
		tex.forceConnector(3);//forçar o conector 3 para o DAQ
	}
}


//Reset DAQ
void gdaq::on_pb_reset_daq_clicked()
{
	// SHOW RESET DAQ
	ui.pb_config_daq->setEnabled(1);
	ui.pb_reset_daq->setEnabled(0);
	ui.boxProbe->setEnabled(0);
	ui.pb_run->setEnabled(0);
	ui.pb_save->setEnabled(0);
	//ui.boxCtrl->setEnabled(0);
	ui.boxSync->setEnabled(1);
	ui.cb_Trigger_in->setEnabled(1);
	ui.cb_Trigger_out->setEnabled(1);
	ui.sb_Power->setEnabled(1);
	ui.sb_cpwc_main_angle->setEnabled(1);
	ui.sb_Depth->setEnabled(0);
	ui.sb_divisor->setEnabled(0);
	ui.dsb_Gain->setEnabled(0);

	ui.boxSeq->setEnabled(0);

	if (running)
    {
        stop();
    }

	ui.seqname->setText(QString("Daq System Off"));

	tex.shutdown();
	//daq->disconnect();

}


/* Probe*/
void gdaq::on_pb_probe_clicked() 
{
	ChooseProbe dialog(this);

	if(dialog.exec())
	{
		int probe = 0;

		if(dialog.pb1->isChecked()) probe = 0;		
		if(dialog.pb2->isChecked()) probe = 2;	
		if(dialog.pb3->isChecked()) probe = 1;

		ui.sb_probe->setValue(probe);
	}
}
void gdaq::on_sb_probe_valueChanged(int pb) 
{

	if (!tex.activateProbeConnector(pb))
	{
		probeConn = pb;
		probeSize = 128;
		probeCFreq = 7000000;
		probeName = "NO PROBE";
		validprobe=false;
		

	}
	else
	{
		probeSize = tex.getProbeNumElements();
		probeCFreq = tex.getProbeCenterFreq();	

		probeName = new char[32];
		tex.getProbeName(pb, probeName, 32);
		printf(probeName);
		
	}
	
	QString probe = QString(probeName);

	int pbok = QString::compare(probe, "NO PROBE");   // x == 0 for equals

	if (!pbok)
	{
		QMessageBox msgBox;
		msgBox.setText(QString("NOT VALID PROBE AT COONECTOR %1").arg(pb));
		msgBox.exec();
		validprobe=false;
	}
	else
	{
		ui.probename->setText(probe);
		validprobe=true;
	}

}

/*Controls*/
//depth
void gdaq::on_sb_Depth_valueChanged(int x) 
{

	depth = ui.sb_Depth->value();
	gn = ui.dsb_Gain->value();
	divisor_var = ui.sb_divisor->value();

	if(PA_sequence)
	{
		depth = depth/2;
		//out_parameters[0] = depth;
	}
	//TGC
	msDelay(20);
	tex.clearTGCs();
	tex.addTGC(gn);
	
	//Buffer statistiscs
	buffer_daq = ((16.0*1024)/pow(2.0,divisor_var));
	max_frame = ((16.0*1024*1024*1024)/pow(2.0,divisor_var))/(((floor(((2*depth*1000)/1540)+0.5)*1000)/25) * 128 * 2);
	ui.bufferframe->setText(QString("N frames (DAQ): %1").arg(max_frame));
	ui.buffersize->setText(QString("Buffer size (DAQ): %1 MB").arg(buffer_daq));
	//out_parameters[1] = divisor_var;
	//out_parameters[2] = max_frame;
	


}
//divisor
void gdaq::on_sb_divisor_valueChanged(int x) 
{
	depth = ui.sb_Depth->value();
	gn = ui.dsb_Gain->value();
	divisor_var = ui.sb_divisor->value();
	
	if(PA_sequence)
	{
		depth = depth/2;
		//out_parameters[0] = depth;
	
	}
	

	//TGC
	msDelay(20);
	tex.clearTGCs();
	tex.addTGC(gn);
	
	//Buffer statistiscs
	buffer_daq = ((16.0*1024)/pow(2.0,divisor_var));
	max_frame = ((16.0*1024*1024*1024)/pow(2.0,divisor_var))/(((floor(((2*depth*1000)/1540)+0.5)*1000)/25) * 128 * 2);
	ui.bufferframe->setText(QString("N frames (DAQ): %1").arg(max_frame));
	ui.buffersize->setText(QString("Buffer size (DAQ): %1 MB").arg(buffer_daq));
	//out_parameters[0] = depth;
	//out_parameters[1] = divisor_var;
	//out_parameters[2] = max_frame;
	


}
//gain
void gdaq::on_dsb_Gain_valueChanged(double x) 
{
	depth = ui.sb_Depth->value();
	gn = ui.dsb_Gain->value();
	divisor_var = ui.sb_divisor->value();
	
	if(PA_sequence)
	{
		depth = depth/2;
	}
	
	//TGC
	msDelay(20);
	tex.clearTGCs();
	tex.addTGC(gn);
	
	//Buffer statistiscs
	buffer_daq = ((16.0*1024)/pow(2.0,divisor_var));
	max_frame = ((16.0*1024*1024*1024)/pow(2.0,divisor_var))/(((floor(((2*depth*1000)/1540)+0.5)*1000)/25) * 128 * 2);
	ui.bufferframe->setText(QString("N frames (DAQ): %1").arg(max_frame));
	ui.buffersize->setText(QString("Buffer size (DAQ): %1 MB").arg(buffer_daq));
}
//power
void gdaq::on_sb_Power_valueChanged(int x) 
{
	pwx = ui.sb_Power->value();
	
}

/*Sync */

void gdaq::on_cb_Trigger_in_currentIndexChanged(int x) {
	trigger_in = ui.cb_Trigger_in->currentIndex();
	trigger_out = ui.cb_Trigger_out->currentIndex();
	//tex.setSyncSignals(trigger_in, trigger_out, 0);
}
void gdaq::on_cb_Trigger_out_currentIndexChanged(int x) {

	trigger_in = ui.cb_Trigger_in->currentIndex();
	trigger_out = ui.cb_Trigger_out->currentIndex();
	//tex.setSyncSignals(trigger_in, trigger_out, 0);
}


/*Sequence*/

//Fixed Transmit
void gdaq::on_rb_PE_toggled(bool sel)
{
	if (!validprobe)//se não houver um transdutor válido emite uma mensagem de erro
	{
        ui.seqname->setText(QString("Cannot create sequence, no probe selected") );
		validsequence = false;
	}
	else
	{
		// tell program to initialize for new sequence
		if (!tex.beginSequence())//fala pro programa texo começar uma nova sequência
		{
			ui.seqname->setText(QString("Cannot create sequence, tex.beginSequence() error") );
			validsequence = false;
		}
		else
		{
			if (!sequenceFixedTX(custom_sequence))//cria a sequência 1: Fixed Transmit
			{
				ui.seqname->setText(QString("Error Cannot create sequence" ));
				validsequence = false;
			}
			else
			{    
				if (tex.endSequence() == -1)//fala pro Texo terminar a sequência
				{
					ui.seqname->setText(QString("Error Cannot create sequence" ));
					validsequence = false;
				}
				else
				{
					validsequence = true;//configura a variável de sequência válida
					ui.seqname->setText(QString("Fixed Transmit Sequence Loaded" ));
					//PE_sequence = true;
					//PA_sequence = false;
					sDelay(1);
					
				}
			}
		}
	}
}
void gdaq::on_tb_PE_clicked() 
{
	TxParameters dialog(this);
	int angle, aperture, speedOfSound,focusDistance,frequency;
	dialog.sb_tx_angle->setEnabled(1);
	dialog.sb_tx_aperture->setEnabled(1);
	dialog.sb_tx_speedOfSound->setEnabled(1);
	dialog.sb_tx_focusDistance->setEnabled(1);
	if(dialog.exec())
	{
		
		angle = dialog.sb_tx_angle->value();
		aperture = dialog.sb_tx_aperture->value();
		speedOfSound = dialog.sb_tx_speedOfSound->value();
		focusDistance = dialog.sb_tx_focusDistance->value();
		frequency = dialog.sb_tx_frequency->value();

		out_parameters[3] = angle;
		out_parameters[6] = aperture;
		out_parameters[5] = speedOfSound;
		out_parameters[7] = focusDistance;
		out_parameters[4] = frequency;
		custom_sequence = true;
		
		//ui.seqname->setText(QString("focusDistance1: %1 ").arg(focusDistance));
		//sDelay(1);
		//ui.seqname->setText(QString("sequence: %1 ").arg(custom_sequence));
		//sDelay(1);
		sequenceFixedTX(custom_sequence);
		
	}
	
}


//PA sequence
void gdaq::on_rb_PA_toggled(bool sel)
{
	if (!validprobe)//se não houver um transdutor válido emite uma mensagem de erro
	{
        ui.seqname->setText(QString("Cannot create sequence, no probe selected") );
	}
	else
	{
		// tell program to initialize for new sequence
		if (!tex.beginSequence())//fala pro programa texo começar uma nova sequência
		{
			ui.seqname->setText(QString("Cannot create sequence, tex.beginSequence() error") );
			validsequence = false;
		}
		else
		{
			if (!sequencePARX(custom_sequence))//cria a sequência 1: Fixed Transmit
			{
				ui.seqname->setText(QString("Error Cannot create sequence" ));
				validsequence = false;
			}
			else
			{    
				if (tex.endSequence() == -1)//fala pro Texo terminar a sequência
				{
					ui.seqname->setText(QString("Error Cannot create sequence" ));
					validsequence = false;
				}
				else
				{
					validsequence = true;//configura a variável de sequência válida
					ui.seqname->setText(QString("Photoacoustic Sequence Loaded" ));
					//PE_sequence = false;
					//PA_sequence = true;
					on_sb_Depth_valueChanged(ui.sb_Depth->value());
					sDelay(1);
					
				}
			}
		}
	}
}
void gdaq::on_tb_PA_clicked() 
{
	TxParameters dialog(this);
	int angle, aperture, speedOfSound,focusDistance;
	dialog.sb_tx_angle->setEnabled(0);
	dialog.sb_tx_aperture->setEnabled(1);
	dialog.sb_tx_speedOfSound->setEnabled(1);
	dialog.sb_tx_focusDistance->setEnabled(1);
	dialog.sb_tx_frequency->setEnabled(0);
	if(dialog.exec())
	{
		
		aperture = dialog.sb_tx_aperture->value();
		speedOfSound = dialog.sb_tx_speedOfSound->value();
		focusDistance = dialog.sb_tx_focusDistance->value();
		

		out_parameters[6] = aperture;
		out_parameters[5] = speedOfSound;
		out_parameters[7] = focusDistance;
		custom_sequence = true;
		//ui.seqname->setText(QString("focusDistance1: %1 ").arg(focusDistance));
		//sDelay(1);
		//ui.seqname->setText(QString("focusDistance2: %1 ").arg(out_parameters[7]));
		//sDelay(1);

		sequencePARX(custom_sequence);
		
	}
	
}

//Coherent Plane-Wave Compounding
void gdaq::on_rb_CPWC_toggled(bool sel)
	{
	if (!validprobe)//se não houver um transdutor válido emite uma mensagem de erro
	{
        ui.seqname->setText(QString("Cannot create sequence, no probe selected") );
		validsequence = false;
	}
	else
	{
		// tell program to initialize for new sequence
		if (!tex.beginSequence())//fala pro programa texo começar uma nova sequência
		{
			ui.seqname->setText(QString("Cannot create sequence, tex.beginSequence() error") );
			validsequence = false;
		}
		else
		{
			if (!sequenceCPWC(custom_sequence))//cria a sequência 1: Fixed Transmit
			{
				ui.seqname->setText(QString("Error Cannot create sequence" ));
				validsequence = false;
			}
			else
			{    
				if (tex.endSequence() == -1)//fala pro Texo terminar a sequência
				{
					ui.seqname->setText(QString("Error Cannot create sequence" ));
					validsequence = false;
				}
				else
				{
					validsequence = true;//configura a variável de sequência válida
					ui.seqname->setText(QString("Coherent Plane-Wave Compound Loaded" ));
					//PE_sequence = true;
					//PA_sequence = false;
					sDelay(1);
					
				}
			}
		}
	}
}
//void gdaq::on_tb_CPWC_clicked() 
//{
//	TxParameters dialog(this);
//	int aperture, speedOfSound,focusDistance, frequency;
//	dialog.sb_tx_angle->setEnabled(0);
//	dialog.sb_tx_aperture->setEnabled(1);
//	dialog.sb_tx_speedOfSound->setEnabled(1);
//	dialog.sb_tx_focusDistance->setEnabled(1);
//	dialog.sb_tx_frequency->setEnabled(1);
//	if(dialog.exec())
//	{
//		
//		aperture = dialog.sb_tx_aperture->value();
//		speedOfSound = dialog.sb_tx_speedOfSound->value();
//		focusDistance = dialog.sb_tx_focusDistance->value();
//		frequency = dialog.sb_tx_frequency->value();
//
//		out_parameters[6] = aperture;
//		out_parameters[5] = speedOfSound;
//		out_parameters[7] = focusDistance;
//		out_parameters[4] = frequency;
//		custom_sequence = true;
//		
//		if (!sequenceCPWC(custom_sequence))//cria a sequência 1: Fixed Transmit
//			{
//				ui.seqname->setText(QString("Error Cannot create sequence" ));
//				validsequence = false;
//			}
//			else
//			{    
//				if (tex.endSequence() == -1)//fala pro Texo terminar a sequência
//				{
//					ui.seqname->setText(QString("Error Cannot create sequence" ));
//					validsequence = false;
//				}
//				else
//				{
//					validsequence = true;//configura a variável de sequência válida
//					ui.seqname->setText(QString("Coherent Plane-Wave Compound Updated" ));
//					//PE_sequence = true;
//					//PA_sequence = false;
//					sDelay(1);
//					
//				}
//			}
		
//	}
	
	
//}


//PAPE sequence
void gdaq::on_rb_PAPE_toggled(bool sel)
	{
	if (!validprobe)//se não houver um transdutor válido emite uma mensagem de erro
	{
        ui.seqname->setText(QString("Cannot create sequence, no probe selected") );
		validsequence = false;
	}
	else
	{
		// tell program to initialize for new sequence
		if (!tex.beginSequence())//fala pro programa texo começar uma nova sequência
		{
			ui.seqname->setText(QString("Cannot create sequence, tex.beginSequence() error") );
			validsequence = false;
		}
		else
		{
			if (!sequencePAPE(custom_sequence))//cria a sequência 1: Fixed Transmit
			{
				ui.seqname->setText(QString("Error Cannot create sequence" ));
				validsequence = false;
			}
			else
			{    
				if (tex.endSequence() == -1)//fala pro Texo terminar a sequência
				{
					ui.seqname->setText(QString("Error Cannot create sequence" ));
					validsequence = false;
				}
				else
				{
					validsequence = true;//configura a variável de sequência válida
					ui.seqname->setText(QString("Photoacoustic-Fixed Transmit Loaded" ));
					//PE_sequence = true;
					//PA_sequence = false;
					PAPE = true;
					sDelay(1);
					
				}
			}
		}
	}
}
void gdaq::on_tb_PAPE_clicked() 
{
	TxParameters dialog(this);
	int angle, aperture, speedOfSound,focusDistance,frequency;
	dialog.sb_tx_angle->setEnabled(1);
	dialog.sb_tx_aperture->setEnabled(1);
	dialog.sb_tx_speedOfSound->setEnabled(1);
	dialog.sb_tx_focusDistance->setEnabled(1);
	if(dialog.exec())
	{
		
		angle = dialog.sb_tx_angle->value();
		aperture = dialog.sb_tx_aperture->value();
		speedOfSound = dialog.sb_tx_speedOfSound->value();
		focusDistance = dialog.sb_tx_focusDistance->value();
		frequency = dialog.sb_tx_frequency->value();

		out_parameters[3] = angle;
		out_parameters[6] = aperture;
		out_parameters[5] = speedOfSound;
		out_parameters[7] = focusDistance;
		out_parameters[4] = frequency;
		custom_sequence = true;
		//ui.seqname->setText(QString("focusDistance1: %1 ").arg(focusDistance));
		//sDelay(1);
		//ui.seqname->setText(QString("focusDistance2: %1 ").arg(out_parameters[7]));
		//sDelay(1);

		sequencePAPE(custom_sequence);
		
	}
	
}


/*RUN*/
void gdaq::on_pb_run_clicked()
{
	// BUTTON ON/OFF SWITCH
	ui.pb_run->setEnabled(0);
	ui.pb_stop->setEnabled(1);
	ui.pbar_buffer->setEnabled(1);
	
	// CINELOOP
	frameCount = 0;
	
	

	// DAQ
	// TODO: use pulseConfig and iConfig 
	//       information for the next version
	// Disable all boxes
	ui.boxProbe->setEnabled(0);
	ui.boxCtrl->setEnabled(0);
	ui.boxSeq->setEnabled(0);
	ui.boxSync->setEnabled(0);
	//ui.boxRun->setEnabled(0);
	msDelay(10);
	
	if(!run())
	{
		ui.seqname->setText(QString("Cannot run, something get wrong"));
	}
	else
	{
		//ui.seqname->setText(QString("DAQ is runing.. Acquired (%1) frames").arg(frameCount));
		frame_counter();
	}
	
	

}
//STOP
void gdaq::on_pb_stop_clicked() 
{

    // BUTTON ON/OFF SWITCH
	ui.pb_run->setEnabled(1);
	ui.pb_stop->setEnabled(0);
	abort = true;
	stop();
	//abort = true;
	// Enable all boxes
	ui.boxProbe->setEnabled(1);
	ui.boxCtrl->setEnabled(1);
	ui.boxSeq->setEnabled(1);
	ui.boxSync->setEnabled(1);
	//ui.boxRun->setEnabled(1);
	

}
//SAVE
void gdaq::on_pb_save_clicked() 
{
	if(running)
	{
		on_pb_stop_clicked();
	}

	//Qstring fileName;
	fileName = QFileDialog::getSaveFileName(this,"Choose a name for your data.","C:\\Documents and Settings\\Administrator\\Desktop\\DAQ_Data","All Files (*)",0,0);
	if (fileName.isEmpty())
	{
		fileName = "D:\\DAQData\\DAQ_Data\\";
	}
	if (!saveData(fileName))
	{
		ui.seqname->setText(QString("Check DAQ data stored"));
	}
	//ui.seqname->setText(fileName);
	
}





gdaq::~gdaq()
{
	//    delete ui;
}

/* Utils */
void gdaq::sDelay(int time) {
	QTime dieTime= QTime::currentTime().addSecs(time);
	while( QTime::currentTime() < dieTime )
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);    
}
void gdaq::msDelay(int time) {
	QTime dieTime= QTime::currentTime().addMSecs(time);
	while( QTime::currentTime() < dieTime )
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);    
}



//Funções de sequências
bool gdaq::sequenceFixedTX(bool custom)// sequência de transmissão fixa
{
	//Variáveis
    int cnrtEl, lineSize;
    texoTransmitParams tx; //parâmetros de transmissão - tx
    texoReceiveParams rx; //parâmetros de recepção - rx

	//Profundidade e N de frames
	depth = (ui.sb_Depth->value());// atualizar profundidade
	out_parameters[0] = depth;
	max_frame = ((16.0*1024*1024*1024)/pow(2.0,divisor_var))/(((floor(((2*depth*1000)/1540)+0.5)*1000)/25) * 128 * 2);//calcular N para profundidade
	out_parameters[1] = divisor_var;
	out_parameters[2] = max_frame;
	ui.bufferframe->setText(QString("N frames (DAQ): %1").arg(max_frame));//exibir N
	
	//---TRANSMISSÃO---
	//tx.centerElement = 0; //elemento central do transdutor para transmissão - "center element in 1/10th of an element"
    if (custom)
	{
		tx.angle=out_parameters[3];
		tx.aperture=out_parameters[6];
		tx.speedOfSound=out_parameters[5];
		tx.focusDistance = out_parameters[7];
		tx.frequency = out_parameters[4];
		//ui.seqname->setText(QString("Funcionou"));
		custom_sequence = false;
	}
	else
	{
		tx.angle = 0; // anglo da transmissão - "angle in 1/1000th of a degree"
		out_parameters[3] = tx.angle;
		tx.aperture = 128; //aperture size in elements 
		out_parameters[6] = tx.aperture;
		tx.speedOfSound = 1540; //velocidade do som - "speed of sound"
		out_parameters[5] = tx.speedOfSound;
		tx.focusDistance = 300000; // distância focal da transmissão - "focus distance in microns"
		out_parameters[7] = tx.focusDistance;
		tx.frequency = tex.getProbeCenterFreq(); //frequência para transmissão "frequency in Hz"
		out_parameters[4] = tx.frequency;
		custom_sequence = false;
	}

    
   
    strcpy(tx.pulseShape, "+-"); //forma do pulso de transmissão - "pulse shape in codes of '+', '-', or '0'"
    tx.tableIndex = -1; // "transmit table index"
    tx.useDeadElements = false; //"dead elements usage (true = on, false = off)"
    tx.trex = false; //não consta na documentação do texo
    //tx.longPulse = false; //não consta na documentação do texo
	
		//---PLANE WAVE---
    	//Transmit with full aperture
		
		//Plane wave transmit without focusing
		tx.useManualDelays = true; //configurar atrasos na transmissão - "manual time delay usage (true = on, false = off)"
		memset(tx.manualDelays, 0, sizeof(int) * 128); //"time delays for up to 128 elements, with the maximum delay at the end"
		//----------------------------------
	//----------------------------------
	
		
	//---RECEPÇÃO---
	rx.centerElement = 0; //elemento central do transdutor para recepção - "center element in 1/10th of an element"
    rx.aperture = NUMCHANNELS; //aperture size in elements
    rx.angle = 0; // anglo da recepção - "angle in 1/1000th of a degree"
    rx.maxApertureDepth = 30000; //"maximum aperture curve depth in microns"
    rx.acquisitionDepth = depth * 1000; //"acquisition depth in microns"
    rx.saveDelay = 0; //"depth in microns to delay the saving of data" 
    rx.speedOfSound = 1540; //velocidade do som
    rx.channelMask[0] = rx.channelMask[1] = 0xFFFFFFFF;//"channel masking for upto 64 elements (0 = on, 1 = off)"
    rx.applyFocus = true;//"computed time delay usage (true = on, false = off)"
    rx.useManualDelays = false;//configurar atrasos na recepção - "manual time delay usage (true = on, false = off)"
    
		//---adjust the frame duration for DAQ---
		rx.customLineDuration = 200000; //"custom line duration in ns" 
    	//----------------------------------
    
	rx.decimation = 0; //"decimation value for RF or B mode data"
    rx.lgcValue = 0;//"digital gain value applied to entire scanline (0 - 4095)"
    rx.tgcSel = 0;//"tgc selection by index" 
    rx.tableIndex = -1; //"receive table index"
    //rx.saveDaqData = false; //"não consta na documentação do texo"
	//----------------------------------
    
	//---Transmitir fixado no elemento central---
    cnrtEl = tex.getProbeNumElements() / 2;//numero de elementos (N) do transdutor dividido por 2
    tx.centerElement = (cnrtEl * 10) + 5; // ((N/2)*10)+5 na transmissão
    rx.centerElement = (cnrtEl * 10) + 5; // ((N/2)*10)+5 na recepção
    //----------------------------------

    lineSize = tex.addLine(rfData, tx, rx);

    if (lineSize == -1)
    {
        return false;
    }
	//Pulso Eco
	PE_sequence = true;
	//Photoacoustic
	PA_sequence = false;
	//Coherent Plane Wave Compounding
	cpwc = false;
	//Photoacoustic / Pulso-Eco
	PAPE = false;

    return true;
}

bool gdaq::sequencePARX(bool custom)//sequência atual de Photoacoustic imaging modificiada da sequência FixedTX para emitir 0
{
	//Variáveis
    int cnrtEl, lineSize;
    texoTransmitParams tx; //parâmetros de transmissão - tx
    texoReceiveParams rx; //parâmetros de recepção - rx

	//Profundidade e N de frames
	depth = (ui.sb_Depth->value());
	depth = (int)depth/2;//PA tem metade da profundidade pois a onda é gerada no material e não retroespalhada como no PE
	out_parameters[0] = depth;
	max_frame = ((16.0*1024*1024*1024)/pow(2.0,divisor_var))/(((floor(((2*depth*1000)/1540)+0.5)*1000)/25) * 128 * 2);
	out_parameters[1] = divisor_var;
	out_parameters[2] = max_frame;
	ui.bufferframe->setText(QString("N frames (DAQ): %1").arg(max_frame));

	//---TRANSMISSÃO---
    tx.centerElement = 0; //elemento central do transdutor para transmissão - "center element in 1/10th of an element"
    //tx.focusDistance = 300000; // distância focal da transmissão - "focus distance in microns"
	//out_parameters[7] = tx.focusDistance;
	//tx.focusDistance = 0;
    tx.angle = 0; // anglo da transmissão - "angle in 1/1000th of a degree"
    out_parameters[3] = tx.angle;
	tx.frequency = tex.getProbeCenterFreq(); //frequência para transmissão "frequency in Hz"
    out_parameters[4] = tx.frequency;
	strcpy(tx.pulseShape, "0"); //forma do pulso de transmissão - "pulse shape in codes of '+', '-', or '0'"
    if(custom)
	{
		tx.speedOfSound = out_parameters[5];
		tx.aperture = out_parameters[6];
		tx.focusDistance = out_parameters[7];
		custom_sequence = false;
	}
	else
	{
		tx.speedOfSound = 1540; //velocidade do som - "speed of sound"
		out_parameters[5] = tx.speedOfSound;
		tx.aperture = 0; //aperture size in elements 
		out_parameters[6] = tx.aperture;
		tx.focusDistance = 300000; // distância focal da transmissão - "focus distance in microns"
		out_parameters[7] = tx.focusDistance;
		custom_sequence = false;
	}

    
	tx.tableIndex = -1; // "transmit table index"
    tx.useDeadElements = false; //"dead elements usage (true = on, false = off)"
    tx.trex = false; //não consta na documentação do texo
    //tx.longPulse = false; //não consta na documentação do texo
	
		//---PLANE WAVE---
		//transmit with full aperture
		//tx.aperture = 128; //aperture size in elements 
		// plane wave transmit without focusing
		tx.useManualDelays = true; //configurar atrasos na transmissão - "manual time delay usage (true = on, false = off)"
		memset(tx.manualDelays, 0, sizeof(int) * 128); //"time delays for up to 128 elements, with the maximum delay at the end"
		//----------------------------------
	//----------------------------------
	
	//---RECEPÇÃO---
	rx.centerElement = 0; //elemento central do transdutor para recepção - "center element in 1/10th of an element"
    rx.aperture = NUMCHANNELS; //aperture size in elements
    rx.angle = 0; // anglo da recepção - "angle in 1/1000th of a degree"
    //rx.maxApertureDepth = 30000; //"maximum aperture curve depth in microns"
	rx.maxApertureDepth = 0;
    rx.acquisitionDepth = depth * 1000; //"acquisition depth in microns"
    rx.saveDelay = 0; //"depth in microns to delay the saving of data" 
    rx.speedOfSound = 1540; //velocidade do som
    rx.channelMask[0] = rx.channelMask[1] = 0xFFFFFFFF;//"channel masking for upto 64 elements (0 = on, 1 = off)"
    rx.applyFocus = false;//"computed time delay usage (true = on, false = off)"
    rx.useManualDelays = false;//configurar atrasos na recepção - "manual time delay usage (true = on, false = off)"
		
		//---adjust the frame duration for DAQ---
		rx.customLineDuration = 200000; //"custom line duration in ns" 
		//----------------------------------

    rx.decimation = 0; //"decimation value for RF or B mode data"
    rx.lgcValue = 0;//"digital gain value applied to entire scanline (0 - 4095)"
    rx.tgcSel = 0;//"tgc selection by index" 
    rx.tableIndex = -1; //"receive table index"
    //rx.saveDaqData = false; //"não consta na documentação do texo"

    //----------------------------------
    
	//---Fixar transmissão no elemento central---
    cnrtEl = tex.getProbeNumElements() / 2;
    tx.centerElement = (cnrtEl * 10) + 5;
    rx.centerElement = (cnrtEl * 10) + 5;
    //----------------------------------
	
    lineSize = tex.addLine(rfData, tx, rx);

    if (lineSize == -1)
    {
        return false;
    }

	//Pulso Eco
	PE_sequence = false;
	//Photoacoustic
	PA_sequence = true;
	//Coherent Plane Wave Compounding
	cpwc = false;
	//Photoacoustic / Pulso-Eco
	PAPE = false;

    return true;

}

bool gdaq::sequenceCPWC(bool custom)// sequência de transmissão com angulação na onda plana
{
	//Variáveis
    int i,cnrtEl, lineSize, aux_angle;
    texoTransmitParams tx; //parâmetros de transmissão - tx
    texoReceiveParams rx; //parâmetros de recepção - rx

	//Profundidade e N de frames
	depth = (ui.sb_Depth->value());
	out_parameters[0] = depth;
	max_frame = ((16.0*1024*1024*1024)/pow(2.0,divisor_var))/(((floor(((2*depth*1000)/1540)+0.5)*1000)/25) * 128 * 2);
	out_parameters[1] = divisor_var;
	out_parameters[2] = max_frame;
	ui.bufferframe->setText(QString("N frames (DAQ): %1").arg(max_frame));


	//---TRANSMISSÃO---
    if (custom)
	{
		tx.aperture=out_parameters[6];
		tx.speedOfSound=out_parameters[5];
		tx.focusDistance = out_parameters[7];
		tx.frequency = out_parameters[4];
		//ui.seqname->setText(QString("Funcionou"));
		custom_sequence = false;
	}
	else
	{
		tx.aperture = 128; //aperture size in elements 
		out_parameters[6] = tx.aperture;
		tx.speedOfSound = 1540; //velocidade do som - "speed of sound"
		out_parameters[5] = tx.speedOfSound;
		tx.focusDistance = 300000; // distância focal da transmissão - "focus distance in microns"
		out_parameters[7] = tx.focusDistance;
		tx.frequency = tex.getProbeCenterFreq(); //frequência para transmissão "frequency in Hz"
		out_parameters[4] = tx.frequency;
		custom_sequence = false;
		//max_angle = 4;
	}
	//tx.centerElement = 0; //elemento central do transdutor para transmissão - "center element in 1/10th of an element"
    //tx.focusDistance = 0;
	//tx.angle = 0; // anglo da transmissão - "angle in 1/1000th of a degree"
    
	strcpy(tx.pulseShape, "+-"); //forma do pulso de transmissão - "pulse shape in codes of '+', '-', or '0'"
    tx.tableIndex = -1; // "transmit table index"
    tx.useDeadElements = false; //"dead elements usage (true = on, false = off)"
    tx.trex = false; //não consta na documentação do texo
    //tx.longPulse = false; //não consta na documentação do texo
	
		//---transmit with full aperture---
		
		// plane wave transmit without focusing
		tx.useManualDelays = false; //configurar atrasos na transmissão - "manual time delay usage (true = on, false = off)"
		//memset(tx.manualDelays, 0, sizeof(int) * 128); //"time delays for up to 128 elements, with the maximum delay at the end"
		//----------------------------------
	//----------------------------------
	
	//---RECEPÇÃO---
	rx.centerElement = 0; //elemento central do transdutor para recepção - "center element in 1/10th of an element"
    rx.aperture = NUMCHANNELS; //aperture size in elements
    rx.angle = 0; // anglo da recepção - "angle in 1/1000th of a degree"
    rx.maxApertureDepth = 30000; //"maximum aperture curve depth in microns"
    rx.acquisitionDepth = depth * 1000; //"acquisition depth in microns"
    rx.saveDelay = 0; //"depth in microns to delay the saving of data" 
    rx.speedOfSound = 1540; //velocidade do som
    rx.channelMask[0] = rx.channelMask[1] = 0xFFFFFFFF;//"channel masking for upto 64 elements (0 = on, 1 = off)"
    rx.applyFocus = true;//"computed time delay usage (true = on, false = off)"
    rx.useManualDelays = false;//configurar atrasos na recepção - "manual time delay usage (true = on, false = off)"
    
		//---adjust the frame duration for DAQ---
		rx.customLineDuration = 200000; //"custom line duration in ns" 
		//----------------------------------
    
	rx.decimation = 0; //"decimation value for RF or B mode data"
    rx.lgcValue = 0;//"digital gain value applied to entire scanline (0 - 4095)"
    rx.tgcSel = 0;//"tgc selection by index" 
    rx.tableIndex = -1; //"receive table index"
    //rx.saveDaqData = false; //"não consta na documentação do texo"

    //----------------------------------
    
	//---Transmitir fixado no centro---
    cnrtEl = tex.getProbeNumElements() / 2;//numero de elementos (N) do transdurtor dividido por 2
    tx.centerElement = (cnrtEl * 10) + 5; // ((N/2)*10)+5 na transmissão
    rx.centerElement = (cnrtEl * 10) + 5; // ((N/2)*10)+5 na recepção
    //----------------------------------// change angle
    
	//mudar angulação de 24° até -24° com passo de 3° (17 frames)
	
	aux_angle = 12 - (max_angle / 2);
	out_parameters[3] = max_angle;
	for (i = 0; i <= max_angle; i++)
    {
		tx.angle = cpwc_vet[aux_angle+i];
		lineSize = tex.addLine(rfData, tx, rx);

        if (lineSize == -1)
        {
           return false;
        }

    }
	
	//Pulso Eco
	PE_sequence = false;
	//Photoacoustic
	PA_sequence = false;
	//Coherent Plane Wave Compounding
	cpwc = true;
	//Photoacoustic / Pulso-Eco
	PAPE = false;
    
	return true;
}

bool gdaq::sequencePAPE(bool custom)// sequência de transmissão com angulação na onda plana
{
	//Variáveis
    int i,cnrtEl, lineSize;
    texoTransmitParams tx; //parâmetros de transmissão - tx
    texoReceiveParams rx; //parâmetros de recepção - rx

	//Profundidade e N de frames
	depth = (ui.sb_Depth->value());
	out_parameters[0] = depth;
	max_frame = ((16.0*1024*1024*1024)/pow(2.0,divisor_var))/(((floor(((2*depth*1000)/1540)+0.5)*1000)/25) * 128 * 2);
	out_parameters[1] = divisor_var;
	out_parameters[2] = max_frame;
	ui.bufferframe->setText(QString("N frames (DAQ): %1").arg(max_frame));

	if (custom)
	{
		tx.angle=out_parameters[3];
		tx.aperture=out_parameters[6];
		tx.speedOfSound=out_parameters[5];
		tx.focusDistance = out_parameters[7];
		tx.frequency = out_parameters[4];
		//ui.seqname->setText(QString("Funcionou"));
		custom_sequence = false;
	}
	else
	{
		tx.angle = 0; // anglo da transmissão - "angle in 1/1000th of a degree"
		out_parameters[3] = tx.angle;
		tx.aperture = 128; //aperture size in elements 
		out_parameters[6] = tx.aperture;
		tx.speedOfSound = 1540; //velocidade do som - "speed of sound"
		out_parameters[5] = tx.speedOfSound;
		tx.focusDistance = 300000; // distância focal da transmissão - "focus distance in microns"
		out_parameters[7] = tx.focusDistance;
		tx.frequency = tex.getProbeCenterFreq(); //frequência para transmissão "frequency in Hz"
		out_parameters[4] = tx.frequency;
		custom_sequence = false;
	}



	//---TRANSMISSÃO---
    //tx.centerElement = 0; //elemento central do transdutor para transmissão - "center element in 1/10th of an element"
    tx.tableIndex = -1; // "transmit table index"
    tx.useDeadElements = false; //"dead elements usage (true = on, false = off)"
    tx.trex = false; //não consta na documentação do texo
    //tx.longPulse = false; //não consta na documentação do texo
	//---------------------

	//---RECEPÇÃO---
	rx.centerElement = 0; //elemento central do transdutor para recepção - "center element in 1/10th of an element"
    rx.aperture = NUMCHANNELS; //aperture size in elements
    rx.angle = 0; // anglo da recepção - "angle in 1/1000th of a degree"
    rx.maxApertureDepth = 30000; //"maximum aperture curve depth in microns"
    rx.acquisitionDepth = depth * 1000; //"acquisition depth in microns"
    rx.saveDelay = 0; //"depth in microns to delay the saving of data" 
    rx.speedOfSound = 1540; //velocidade do som
    rx.channelMask[0] = rx.channelMask[1] = 0xFFFFFFFF;//"channel masking for upto 64 elements (0 = on, 1 = off)"
    rx.applyFocus = true;//"computed time delay usage (true = on, false = off)"
    rx.useManualDelays = false;//configurar atrasos na recepção - "manual time delay usage (true = on, false = off)"
    
		//---adjust the frame duration for DAQ---
		rx.customLineDuration = 200000; //"custom line duration in ns" 
		//----------------------------------
    rx.decimation = 0; //"decimation value for RF or B mode data"
    rx.lgcValue = 0;//"digital gain value applied to entire scanline (0 - 4095)"
    rx.tgcSel = 0;//"tgc selection by index" 
    rx.tableIndex = -1; //"receive table index"
    //rx.saveDaqData = false; //"não consta na documentação do texo"

    //----------------------------------
    
	//---Transmitir fixado no elemento central---
    cnrtEl = tex.getProbeNumElements() / 2;//numero de elementos (N) do transdurtor dividido por 2
    tx.centerElement = (cnrtEl * 10) + 5; // ((N/2)*10)+5 na transmissão
    rx.centerElement = (cnrtEl * 10) + 5; // ((N/2)*10)+5 na recepção
    //----------------------------------// change angle
    
	//out_parameters[6] = 1280; 
	//---ALTERNAR ENTRE PA E PE---
	for (i = 0; i < 2; i++)
    {
        if (i==0)
		{
			//---Não transmitir---
			tx.aperture = 0; //aperture size in elements 
			strcpy(tx.pulseShape, "0");
			tx.useManualDelays = false; //configurar atrasos na transmissão - "manual time delay usage (true = on, false = off)"
			//---------------------
			
		}
		else
		{
			//----PLANE WAVE---
			tx.aperture=out_parameters[6]; //aperture size in elements 
			strcpy(tx.pulseShape, "+-");
			tx.useManualDelays = true; //configurar atrasos na transmissão - "manual time delay usage (true = on, false = off)"
			memset(tx.manualDelays, 0, sizeof(int) * 128); //"time delays for up to 128 elements, with the maximum delay at the end"
			//------------------
		}
		
		lineSize = tex.addLine(rfData, tx, rx);

        if (lineSize == -1)
        {
            return false;
        }
    }
	
	//Pulso Eco
	PE_sequence = false;
	//Photoacoustic
	PA_sequence = false;
	//Coherent Plane Wave Compounding
	cpwc = false;
	//Photoacoustic / Pulso-Eco
	PAPE = true;

	return true;
}



//DAQ Console run
bool gdaq::run()//"roda o DAQ + Texo"
{
    if (!validsequence)//caso uma sequência ainda nao tenha sido selecionada, emite uma mensagem de erro
    {
        ui.seqname->setText(QString("Cannot run, invalid sequence selected"));
		return false;
    }

    if (running)//caso tente rodar de novo com uma sequência já rodando, emite uma mensagem informando mas não interrompe a sequência ativa 
    {
		ui.seqname->setText(QString("Sequence is already running"));
        return false;
    }

    // first upload the DAQ sequence
    if (!sequenceDAQ())//caso os parâmetros do DAQ não estejam configurados, emite uma mensagem de erro
    {
		ui.seqname->setText(QString("Failed to prepare the DAQ"));
        return false;
    }

    // then run the Texo sequence
    if (tex.runImage()) //roda o texo e consequentemente roda o DAQ
    {
        running = true;// ativa a variável que mostra a situação da aquisição de dados pelo DAQ
		abort = false;
        return true;
		
    }
	
    return false;
}

bool gdaq::sequenceDAQ()//configura os parâmetros do DAQ
{
    int const numChannls = 128;//número de canais

    channelsDAQ[0] = 0xffffffff;//não foi encontrado nada na documentação para esse parâmetro (ainda sob pesquisa)
    channelsDAQ[1] = 0xffffffff;//não foi encontrado nada na documentação para esse parâmetro (ainda sob pesquisa)
    channelsDAQ[2] = 0xffffffff;//não foi encontrado nada na documentação para esse parâmetro (ainda sob pesquisa)
    channelsDAQ[3] = 0xffffffff;//não foi encontrado nada na documentação para esse parâmetro (ainda sob pesquisa)

    rlprms.channels = channelsDAQ;//não foi encontrado nada na documentação para esse parâmetro (ainda sob pesquisa)
    rlprms.gainDelay = 0;//não foi encontrado nada na documentação para esse parâmetro (ainda sob pesquisa)
    rlprms.gainOffset = 0;//não foi encontrado nada na documentação para esse parâmetro (ainda sob pesquisa)
    rlprms.rxDelay = 3;//não foi encontrado nada na documentação para esse parâmetro (ainda sob pesquisa)
    // sampling and decimation
    //if (sampling_80MHz)
    //{
    //    rlprms.sampling = 80;   // DAQ sampling frequency 80 -> 80 [MHz]
    //    rlprms.decimation = 0;  // no decimation for 80MHz sampling
    //}
    //else
    //{
        rlprms.sampling = 40;   // DAQ sampling frequency 40 -> 40 [MHz]
        rlprms.decimation = 0;  // Fs = sampling / (1+decimation) e.g. decimation = 1 -> Fs=20 MHz
    //}
    rlprms.lineDuration = floor((((2*depth)*1000)/1540)+0.5);  // line duration in micro seconds - duração da linha em ms, aqui feita da forma round(2*depth(m)/velocityofsound(m/s))
    rlprms.numSamples = ((rlprms.lineDuration)*1000)/25; // este parâmetro diz quantos pontos uma linha terá, aqui foi feito para 40MHz portanto (Lineduration(ns)/25)

    seqprms.freeRun = false;//configura o DAQ a adquirir continuamente ou parar depois de um dado tamanho de dados.
    seqprms.hpfBypass = false;//filtro digital passa alta.
    seqprms.divisor = divisor_var; // tamanho dos dados a serem adquiridos, isto é, dataSize = 16GB/2^divisor
    seqprms.externalTrigger = true;//define se o DAQ deve “ouvir” um sinal sync externo ou não. Para ter dados sincronizados este parâmetro deve ser “true”. Caso contrário, a aquisição do DAQ não será sincronizada com a transmissão.
	seqprms.externalClock = false; // : define se o DAQ deve usar um clock externo ou usar seu próprio clock interno. Configure isso como “false” se um clock externo não for fornecido.
    if(ui.cb_fixedTGC->isChecked())
	{
		seqprms.fixedTGC = true; //define se o DAQ deve usar o TGC “flat” ou ajustável.
	}
	else
	{
		seqprms.fixedTGC = false; //define se o DAQ deve usar o TGC “flat” ou ajustável.
	}

    seqprms.lnaGain = 1; // Ganho LNA [0:1:2] corresponde a [16dB, 18dB, 21dB].
    seqprms.pgaGain = 1; //Ganho PGA [0:1:3] corresponde a [21dB, 24dB, 27dB, 30dB].
    seqprms.biasCurrent = 1; // troca o ganho [0:1:7] onde 0 desativa completamente.

    if (seqprms.fixedTGC)//se o ganho TGC for fixo
    {
		float teste = gn*100.00;
        seqprms.fixedTGCLevel = (int) teste;//usa o ganho de 100
		//ui.seqname->setText(QString("Tgc gain = %1").arg( seqprms.fixedTGCLevel));
		//sDelay(2);
    }
    else//senão
    {
		float y1 = 0.1*ui.hs_y1->value();
		float y2 = 0.1*ui.hs_y2->value();
		float y3 = 0.1*ui.hs_y3->value();
		float x1 = ui.dsb_x1->value();
		float x2 = ui.dsb_x2->value();
		float x3 = ui.dsb_x3->value();

		//ui.seqname->setText(QString("Y1 = %1").arg(y1));
		//sDelay(1);
		//ui.seqname->setText(QString("Y2 = %1").arg(y2));
		//sDelay(1);
		//ui.seqname->setText(QString("Y3 = %1").arg(y3));
		//sDelay(1);
		//ui.seqname->setText(QString("X1 = %1").arg(x1));
		//sDelay(1);
		//ui.seqname->setText(QString("X2 = %1").arg(x2));
		//sDelay(1);
		//ui.seqname->setText(QString("X3 = %1").arg(x3));
		//sDelay(1);

		// set TGC curve (usa-se um ganho ajustável com a profundidade)

		//se fixedTGC for configurado como “false”, uma curva de TGC precisa ser definida.
		//Atualmente, esta curva é definida por 3 pontos com valores (X, Y) em uma extensão de 0 até 1,
		//em que 1 corresponde ao máximo valor de TGC para Y e máxima profundidade de aquisição para X.  
        
		tgc.setX(0, x1);
        tgc.setX(1, x2);
        tgc.setX(2, x3);

        tgc.setY(0, y1);
        tgc.setY(1, y2);
        tgc.setY(2, y3);
    }
    if (!daq->uploadSequence(rlprms, tgc, seqprms))//Uploads a sequence to the DAQ and starts running the capture.  
    {
		ui.seqname->setText("Could not upload DAQ sequence");
        sDelay(2);
		ui.seqname->setText(daq->getLastError().c_str());
		return false;
    }

    //printf("DAQ statistics:\n\n");
    //printf("Line duration = %d micro sec\n", rlprms.lineDuration);//mostra a duração da linha
    //printf("Number of channels = %d channels\n", numChannls);//numero de canais
    //printf("Samples per channel = %d samples\n", rlprms.numSamples);//numero de pontos por linha
    //printf("Frame size = %d bytes\n", (rlprms.numSamples * numChannls * 2));//tamanho do frame em bytes
	//printf("DAQ buffer size = %.1f Mbytes\n", ((16.0*1024)/pow(2.0,seqprms.divisor)));//tamanho do buffer do daq em MB
	//max_frame_buffer = (int)((16.0*1024*1024*1024)/pow(2.0,seqprms.divisor))/((rlprms.numSamples * numChannls * 2));//configura a variavel de tamanho do buffer em número de frames
	//printf("Number of frames to fill DAQ buffer = %d ", max_frame_buffer);//mostra o número frames que cabem no buffer do DAQ selecionado
	
	return true;
}

void gdaq::frame_counter()//mostrar a contagem de frames adquiridos pelo DAQ
{
	frameCount =0;
	int percent =0;
	float frameCount2=0;
	float max_frame2=0;
	bool cpwc_aux;
	
	
	if(trigger_in ==1)
	{
			if (running)//caso o DAQ esteja rodando
			{
				ui.framebox->setEnabled(1);
				ui.pbar_buffer->setEnabled(1);
			
				ui.lcd_frame->display(frameCount);
				ui.pbar_buffer->setValue(percent);
				ui.seqname->setText(QString("DAQ is running..."));
				msDelay(10);
			
				while (frameCount<max_frame+1&&!abort)
				{
					if (frameCount !=0)//mostrar a contagem dos frames a partir do primeiro frame adquirido
					{
						//ui.seqname->setText(QString("DAQ is running... Acquired (%1) frames\n").arg(tex.getCollectedFrameCount()));
					
						

						
						frameCount2 = frameCount;
						max_frame2 = max_frame;
						
						percent = (int) (100*(frameCount2/max_frame2));
						if(frameCount2>=max_frame2)
						{
							percent = 100;
						}

						ui.lcd_frame->display(frameCount2);
						ui.pbar_buffer->setValue(percent);
					
					}
						if(tex.getCollectedFrameCount()<tex.getMaxFrameCount())//resolver um problema do Texo: caso ainda não houver nenhum frame adquirido, o tex.getCollectedFrameCount() tem valor igual ao número máximo de linhas a serem adquiridas no buffer da SonixRP,em vez de 0. 
						{
							frameCount = tex.getCollectedFrameCount();//se pelo menos um frame for adquirido. o tex.getCollectedFrameCount já representa o real número de frames adquiridos
							if(cpwc)
							{
								frameCount = frameCount*(max_angle+1);
								if(frameCount >= (max_frame+1))
								{
							
									frameCount = (max_frame+1);
								}
							}
							if(PAPE)
							{
								frameCount = frameCount*2;
							}
						}
							msDelay(2);
							
				}
				if (frameCount >= max_frame+1)//caso o numero de frames adquiridos seja igual a quantidade de frames que cabem no Buffer do DAQ finaliza a aquisição.
				{
					ui.seqname->setText(QString("DAQ buffer full: Acquired (%1) frames").arg(frameCount-1));
					stop();
				
					frameCount2 = frameCount;
					max_frame2 = max_frame+1;
					percent = (int) (100*(frameCount2/max_frame2));
					ui.lcd_frame->display(frameCount-1);
					ui.pbar_buffer->setValue(percent);
				
					abort=false;
					ui.boxProbe->setEnabled(1);
					ui.boxCtrl->setEnabled(1);
					ui.boxSeq->setEnabled(1);
					ui.boxSync->setEnabled(1);
					return;
				}
				else
				{
					ui.seqname->setText(QString("Aborted, Acquired (%1) frames: all data will be discarted").arg(frameCount));
					abort = false;
					ui.boxProbe->setEnabled(1);
					ui.boxCtrl->setEnabled(1);
					ui.boxSeq->setEnabled(1);
					ui.boxSync->setEnabled(1);
					return;
				}
			}
		}
	
	else
	{
		if (running)//caso o DAQ esteja rodando
		{
			//ui.framebox->setEnabled(0);
			//ui.pbar_buffer->setEnabled(0);
			//ui.seqname->setText(QString("DAQ buffer full: Acquired (%1) frames").arg(tex.getCollectedFrameCount()));
			while (frameCount<max_frame+1)
			{
				if(tex.getCollectedFrameCount()<tex.getMaxFrameCount())//resolver um problema do Texo: caso ainda não houver nenhum frame adquirido, o tex.getCollectedFrameCount() tem valor igual ao número máximo de linhas a serem adquiridas no buffer da SonixRP,em vez de 0. 
				{
					frameCount = tex.getCollectedFrameCount();//se pelo menos um frame for adquirido. o tex.getCollectedFrameCount já representa o real número de frames adquiridos
				}
				
			}
		}
	}
	if (tex.getCollectedFrameCount() >= max_frame+1)//caso o numero de frames adquiridos seja igual a quantidade de frames que cabem no Buffer do DAQ finaliza a aquisição.
	{
		ui.seqname->setText(QString("DAQ buffer full: Acquired (%1) frames").arg(max_frame));
		stop();
		abort=false;
		ui.boxProbe->setEnabled(1);
		ui.boxCtrl->setEnabled(1);
		ui.boxSeq->setEnabled(1);
		ui.boxSync->setEnabled(1);
	}
	
}

	


//DAQ Console Stop
bool gdaq::stop()//para a sequência rodando
{
	out_parameters[0] = depth;
	out_parameters[1] = divisor_var;
	out_parameters[2] = max_frame;
	ui.pb_run->setEnabled(1);
	ui.pb_stop->setEnabled(0);
    if (!running)//caso tente parar quando não há sequência rodando, emite uma mensagem informando
    {
		ui.seqname->setText(QString("Nothing to stop, sequence is not running"));
        return false;
    }

    if (tex.stopImage())//para o texo e consequentemente para o DAQ
    {
        running = false;//configura a variável que mostra a situação da aquisição de dados pelo DAQ
        //ui.seqname->setText(QString("Finished, Acquired (%1) frames").arg(frameCount));//mostra o número de frames adquiridos contados pelo Texo

        return true;
    }
	

    return false;
}

//DAQ Console Save
bool gdaq::saveData(QString save)//salva os dados do buffer do DAQ para o disco rígido
{
	
    int numFrames, frameSize;
    //FILE* fp;

    numFrames = tex.getCollectedFrameCount();//numero de frames do texo
	if (tex.getCollectedFrameCount()>tex.getMaxFrameCount())//resolver um problema do Texo: caso ainda não houver nenhum frame adquirido, o tex.getCollectedFrameCount() tem valor igual ao número máximo de linhas a serem adquiridas no buffer da SonixRP,em vez de 0. 
	{
		numFrames = 0;//se pelo menos um frame for adquirido. o tex.getCollectedFrameCount já representa o real número de frames adquiridos
	}
    frameSize = tex.getFrameSize();//tamanho do frame do texo

    if (numFrames < 1)//se tenta salvar sem nenhum frame adquirido, não há o que salvar
    {
		ui.seqname->setText(QString( "No frames have been acquired"));
        return false;
    }

    // create sub-folder (cria a pasta no caminho escolhido)
	char inpath[80];
	char inpath2[80];
	strcpy(inpath, save.toAscii().data());
	CreateDirectoryA(inpath, 0);
    strcat(inpath, "\\");

	 ///////////////////// save DAQ data ///////////
	
    if (!daq->isConnected())//se o DAQ desconectar, mostra o erro de desconexão
    {
        ui.seqname->setText(QString("The device is not connected!"));
        return false;
    }
	//função storeData(const char*  path, unsigned int* channelMap, int numSamples, int divisor, DAQ_CALLBACK fn=0, void* prm=0) 

	//Stores channel data to disk. 
	//Parameters:
	//[in]  path  The path to store the channel data to.  
	//[in]  channelMap  The current channel mapping that specifies what channels were active in capture.  
	//[in]  numSamples  The number of samples per channel that were captured.  
	//[in]  divisor  The divisor used.  
	//[in]  fn  An optional callback function that will be called during download to update completion status.  
	//[in]  prm  An optional parameter that will be passed back through the callback.  

	for (int i = 1; i <= 2; i++)
	{
		ui.seqname->setText(QString("Storing DAQ data ."));
		msDelay(333);
		ui.seqname->setText(QString("Storing DAQ data .."));
		msDelay(333);
		ui.seqname->setText(QString("Storing DAQ data ..."));
		msDelay(333);
	}
	if (!daq->storeData(inpath, rlprms.channels, rlprms.numSamples, seqprms.divisor))//se não for possível salvar, mostra o ultimo erro do DAQ
    {
        ui.seqname->setText(QString(daq->getLastError().c_str()));
		///Salvar parâmetros///
		//strcpy(inpath2,inpath);
		strcat(inpath, "parameters.txt");
		FILE * parafp;
		/* open the file for writing*/
		parafp = fopen (inpath,"w");
		if (!parafp)
		{
			ui.seqname->setText(QString("Could not save parameters log to specified path"));
			return false;
		}
		if (PA_sequence)
		{
			fprintf (parafp, "Mode: PA \n");
		}
		if (PE_sequence)
		{
			fprintf (parafp, "Mode: PE (FixedTX) \n");
		}
		if (cpwc)
		{
			fprintf (parafp, "Mode: CPWC \n");
		}
		if (PAPE)
		{
			fprintf (parafp, "Mode: PA/PE \n");
		}
	
		fprintf (parafp, "Depth: %d \n",out_parameters[0]);
		fprintf (parafp, "Divisor: %d \n",out_parameters[1]);
		fprintf (parafp, "N frames: %d \n",out_parameters[2]);
		fprintf (parafp, "tx.angle: %d \n",out_parameters[3]);
		fprintf (parafp, "tx.frequency: %d \n",out_parameters[4]);
		fprintf (parafp, "tx.speedOfSound: %d \n",out_parameters[5]);
		fprintf (parafp, "tx.aperture: %d \n",out_parameters[6]);
		fprintf (parafp, "tx.focalDistance: %d \n",out_parameters[7]);
		//ui.seqname->setText(QString("focusDistance2: %1 ").arg(out_parameters[7]));
	
		/* close the file*/
		fclose (parafp);
        return false;
    }
	ui.seqname->setText(QString("Successfully stored DAQ data"));
    sDelay(1);
    
	///Salvar parâmetros///
	//strcpy(inpath2,inpath);
	strcat(inpath, "parameters.txt");
	FILE * parafp;
	/* open the file for writing*/
	parafp = fopen (inpath,"w");
	if (!parafp)
    {
		ui.seqname->setText(QString("Could not save parameters log to specified path"));
        return false;
    }
	if (PA_sequence)
	{
		fprintf (parafp, "Mode: PA \n");
	}
	if (PE_sequence)
	{
		fprintf (parafp, "Mode: PE (FixedTX) \n");
	}
	if (cpwc)
	{
		fprintf (parafp, "Mode: CPWC \n");
	}
	if (PAPE)
	{
		fprintf (parafp, "Mode: PA/PE \n");
	}

	fprintf (parafp, "Depth: %d \n",out_parameters[0]);
	fprintf (parafp, "Divisor: %d \n",out_parameters[1]);
	fprintf (parafp, "N frames: %d \n",out_parameters[2]);
	fprintf (parafp, "tx.angle: %d \n",out_parameters[3]);
	fprintf (parafp, "tx.frequency: %d \n",out_parameters[4]);
	fprintf (parafp, "tx.speedOfSound: %d \n",out_parameters[5]);
	fprintf (parafp, "tx.aperture: %d \n",out_parameters[6]);
	fprintf (parafp, "tx.focalDistance: %d \n",out_parameters[7]);
	//ui.seqname->setText(QString("focusDistance2: %1 ").arg(out_parameters[7]));
   /* close the file*/
   fclose (parafp);
	
	
	/////////////////// save Texo data ///////////
    //for (int i = 1; i <= 2; i++)
	//{
	//	ui.seqname->setText(QString("Storing Texo data ."));
	//	msDelay(333);
	//	ui.seqname->setText(QString("Storing Texo data .."));
	//	msDelay(333);
	//	ui.seqname->setText(QString("Storing Texo data ..."));
	//	msDelay(333);
	//}
	
   // strcat(inpath, "data.txo");
   // fp = fopen(inpath, "wb+");
  //  if (!fp)
   // {
	//	ui.seqname->setText(QString("Could not store data to specified path"));
    //    return false;
   // }

   // fwrite(tex.getCineStart(0), frameSize, numFrames, fp);//salva os dados do Texo

   // fclose(fp);
	//ui.seqname->setText(QString("Successfully stored Texo data"));
    //sDelay(1);
	for (int i = 1; i <= 1; i++)
	{
		ui.seqname->setText(QString("Wait ."));
		msDelay(333);
		ui.seqname->setText(QString("Wait .."));
		msDelay(333);
		ui.seqname->setText(QString("Wait ..."));
		msDelay(333);
	}
	ui.seqname->setText(QString("Ready"));
    
	return true;

}

//Power func
void gdaq::set_power(int pwx)
{
	if (!tex.setPower(pwx,pwx,pwx))
	{
		ui.seqname->setText(QString("Error in Texo Power"));
	}
}

//Sync func
void gdaq::set_sync(int in, int out)
{
	if(!tex.setSyncSignals(in,out,3))
	{
		ui.seqname->setText(QString("Error in Texo Sync"));
	}
	
	
}



//About
void gdaq::on_actionGIIMUS_Laboratory_triggered()
{
	QMessageBox msgBox(this);
    msgBox.setWindowTitle("About Laboratory");
    msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
    msgBox.setText("<a href='http://www.giimus.usp.br'> GIIMUS Laboratory</a>: Departamento de Física - Universidade de São Paulo Av. Bandeirantes, 3900 – CEP 14040-901, Ribeirão Preto, SP,Brasil. Phone: (+55 16) 3315 3643");
	

    msgBox.exec();
}
void gdaq::on_actionAuthor_triggered()
{
	QMessageBox msgBox(this);
    msgBox.setWindowTitle("About Authors");
    msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
    msgBox.setText(" Developed by <a href='http://lattes.cnpq.br/9007500541726750'> João Henrique Uliana</a> and <a href='http://lattes.cnpq.br/4001522784053517'> Diego R. T. Sampaio</a>");  	

    msgBox.exec();
}
