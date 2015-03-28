/*! \file CRS232.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_RS232_H_
#define _CBASIC_MODULE_RS232_H_

#include <QMainWindow>
#include <QTimer>

#include "CBasicModule.h"
#include "ui_ihm_RS232.h"
#include "CRS232Listener.h"


class Console;


 class Cihm_RS232 : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_RS232(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_RS232() { }

    Ui::ihm_RS232 ui;

    CLaBotBox *m_application;
 };



 /*! \addtogroup RS232
   * 
   *  @{
   */

	   
/*! @brief class CRS232
 */	   
class CRS232 : public CBasicModule
{
    Q_OBJECT
#define     VERSION_RS232   "1.0"
#define     AUTEUR_RS232    "Nico"
#define     INFO_RS232      "Gestion de la liaison serie"

public:
    CRS232(const char *plugin_name);
    ~CRS232();

    Cihm_RS232 *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/terminal.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("RS232"); }                  // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_RS232 m_ihm;


// =======================================================
//          MANIPULATION DE LA LIAISON SERIE
// =======================================================
private :
    QList<QSerialPortInfo>  m_list;


// =======================================================
//      RECEPTION ET EMISSION SUR LA LIAISON SERIE
// =======================================================
private :
    CRS232Listener          m_rs232_listener;

public slots:
    void write(const QByteArray &data);
    void write(const char *data, qint64 size);
    void write(char data);

public slots :
    void reveive_rx_data(QByteArray data);

signals :
    void dataReceived(QString data);
    // Signaux ré-émis de CRS232Listener
    void connected(void);
    void disconnected(void);
    void error(void);
    void readyBytes(QByteArray rcv_data);


// =======================================================
//       IHM DE CONFIGURATION DES PARAMETRES RS232
// =======================================================
private :
    void procFillingOptions(void);
    void serialConfigToIHM(QString portname, tConfigRS232 config);
    void serialIHMToConfig(QString &portname, tConfigRS232 &config);

private slots :
    void openButtonClick(void);
    void closeButtonClick(void);
    void portCOMChanged(int index);


public slots :
    void serialConnected(void);
    void serialDisconnected(void);

// =======================================================
//                      ONGLET TERMINAL
// =======================================================
private :
    bool    m_terminal_ON;
    Console *m_terminal;

    void terminalInit(void);

public slots :
    // Menu
    void Terminal_OnOff(bool state);
    void Terminal_Clear(void);
    void Terminal_EchoLocal(bool state);
    void Terminal_Sauvegarder(void);
    void Terminal_EnvoyerFichierTexte(void);

// =======================================================
//                      ONGLET HEX EDIT
// =======================================================
private :
    bool                m_hex_edit_ON;
    QHexEditData        *m_hex_edit_data;
    QHexEditDataWriter  *m_hex_edit_writer;
    QByteArray           m_hex_edit_byte_array;

private slots :
    // Menu
    void HexEdit_OnOff(bool state);
    void HexEdit_Clear(void);
    void HexEdit_SauvegarderBrut(void);
    void HexEdit_SauvegarderHexASCII(void);
    void HexEdit_CopyBrut(void);
    void HexEdit_CopyHexASCII(void);
    void HexEdit_EnvoyerFichierFormatBrut(void);
    void HexEdit_EnvoyerFichierFormatHexASCII(void);

    // Onglet
    void hexEditInit(void);
    void TX_send(void);


// =======================================================
//                      ONGLET SIMU RX/TX
// =======================================================
private :
    bool                m_simu_rx_tx_ON;

    QHexEditData        *m_simuTx_data;
    QHexEditDataWriter  *m_simuTx_writer;
    QByteArray           m_simuTx_byte_array;

private :
    void simuRxTxInit(void);

private slots :
    // Menu
    void SimuRxTx_OnOff(bool state);
    void SimuRx_DonneesEntrantesFichierBrut(void);
    void SimuRx_DonneesEntrantesFichierHexASCII(void);
    void VisuTx_Clear(void);
    void VisuTx_SauvegarderBrut(void);
    void VisuTx_SauvegarderHexASCII(void);

    // Bouton
    void SimuRX_send(void); // Simule une donnée entrante vers l'applicatif

};


#endif // _CBASIC_MODULE_RS232_H_

/*! @} */

