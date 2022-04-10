/*! \file CTrameBot.h
	\brief Classe virtuelle pour définir une trame
*/

#ifndef _TRAME_BOT_H_
#define _TRAME_BOT_H_

#include <QObject>
#include <QString>
#include <QStringList>


#define C_NOMBRE_DATA_TRAMES_ROBOT    16

typedef struct {
	unsigned int ID;		//!< L'identifiant de la trame
	unsigned char DLC;		//!< La longueur de la trame
    unsigned char Data[C_NOMBRE_DATA_TRAMES_ROBOT];	//!< Les donnees utiles de la trame
}tStructTrameBrute;

class CMessagerieBot;
class CDataManager;

// -----------------------------
//! Classe de base pour les trames CAN
class CTrameBot : public QObject
{
    Q_OBJECT
public :
    //! Nom de la trame
    QString m_name;
    //!	Structure brute
	tStructTrameBrute m_trame_brute;
	//! Memorise le nombre de trame recues
	unsigned int m_nombre_recue;
    //! Memorise le nombre de trame émises
    unsigned int m_nombre_emis;
    //! Identifiant de la trame
    unsigned int m_id;
    //! Longueur de la trame
    unsigned int m_dlc;
    //! Liste des signaux de la trame
    QStringList m_liste_noms_signaux;

public :
    CTrameBot(CMessagerieBot *messagerie_bot, CDataManager *data_manager)
        : m_nombre_recue(0),
          m_nombre_emis(0),
          m_data_manager(data_manager),
          m_messagerie_bot(messagerie_bot)
    {

    }
    ~CTrameBot() { }

    virtual void Decode(tStructTrameBrute *trameRecue) { Q_UNUSED(trameRecue) }
    virtual void Encode() { }

protected :
    //! Gestionnaire de données de l'application
    CDataManager *m_data_manager;

    //! Gestionnaire de messagerie
    CMessagerieBot *m_messagerie_bot;
};


#endif // _TRAME_BOT_H_

