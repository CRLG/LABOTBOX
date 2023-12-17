#include "sick_tim561.h"

// Document de référence :
//  telegram_listing_telegram_listing_ranging_sensors_lms1xx_lms5xx_tim2xx_tim5xx_tim7xx_lms1000_mrs1000_mrs6000_nav310_ld_oem15xx_ld_lrs36xx_lms4000_lrs4000_multiscan100_en_im0045927

SickTIM651::SickTIM651(QTcpSocket *parent)
    : QTcpSocket(parent),
      m_enable_autoreconnect(true),
      m_cola_protocol_binary(true)
{
    connect(&m_timer_autoreconnect, SIGNAL(timeout()), this, SLOT(on_tick_timer_autoreconnect()));

    connect(this, SIGNAL(connected()), this, SLOT(on_connect()));
    connect(this, SIGNAL(disconnected()), this, SLOT(on_disconnect()));
}

// _________________________________________________________
/*!
 * \brief Ouverture de la communication avec la Lidar
 * \param hostName Adresse IP du Lidar
 * \param port port de communication du Lidar (2111 ou 2112)
 * \param protocol PROTOCOL_COLA_BINARY ou PROTOCOL_COLA_HEXA ou valeur spéciale -1 qui affecte le protocol en fonction du numéro de port de communication
 * \param autoconnect true/false pour que la reconnexion de la socket soit automatique en cas de perte
 * \return true si la socket est ouverte avec succès / false sinon
 */
bool SickTIM651::open(const QString &hostName, quint16 port, int protocol, bool autoconnect)
{
    m_enable_autoreconnect = autoconnect;
    m_hostname = hostName;
    m_port = port;
    // Protocol affecté automatiquement en fonction du numéro de port sur la logique suivante :
    //      -> Port 2111 : protocol Hexa uniquement d'après la doc
    //      -> Port 2112 : protocol possible Hexa ou Binary (forcé à Binary en supposant que le Lidar est configuré comme tel)
    if (protocol < 0) {
        if (port == 2111)   m_cola_protocol_binary = false;
        else                m_cola_protocol_binary = true;
    }
    else {
        m_cola_protocol_binary = (protocol == PROTOCOL_COLA_BINARY);
    }

    QTcpSocket::connectToHost(m_hostname, m_port);
    if (m_enable_autoreconnect) m_timer_autoreconnect.start(AUTORECONNECT_PERIOD);
    return waitForConnected(TIMEOUT_READ_WRITE);
}

// _________________________________________________________
void SickTIM651::close()
{
    m_timer_autoreconnect.stop();
    QTcpSocket::close();
}

// _________________________________________________________
void SickTIM651::disconnection()
{
    QTcpSocket::disconnectFromHost();
    m_enable_autoreconnect = false;
}

// _________________________________________________________
bool SickTIM651::login()
{
    const char data_hex[] = { 0x02, 0x73, 0x4D, 0x4E, 0x20, 0x53, 0x65, 0x74, 0x41, 0x63, 0x63, 0x65, 0x73, 0x73, 0x4D, 0x6F, 0x64, 0x65, 0x20, 0x30, 0x33, 0x20, 0x46, 0x34, 0x37, 0x32, 0x34, 0x37, 0x34, 0x34, 0x03 };
    const char data_binary[] = { 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x17, 0x73, 0x4D, 0x4E, 0x20, 0x53, 0x65, 0x74, 0x41, 0x63, 0x63, 0x65, 0x73, 0x73, 0x4D, 0x6F, 0x64, 0x65, 0x20, 0x03, (char)0xF4, 0x72, 0x47, 0x44, (char)0xB3 };
    const char *data = m_cola_protocol_binary ? data_binary : data_hex;
    int size = m_cola_protocol_binary ? sizeof(data_binary) : sizeof(data_hex);

    int written_size = write(data, size);
    if (written_size != size) return false;
    if (!waitForBytesWritten(TIMEOUT_READ_WRITE)) return false;

    if (!waitForReadyRead(TIMEOUT_READ_WRITE)) return false;
    QByteArray ba = readAll();

    return !isErrorReturned(ba);
}

// _________________________________________________________
bool SickTIM651::start_measurement()
{
    const char data_hex[] = { 0x02, 0x73, 0x4D, 0x4E, 0x20, 0x4C, 0x4D, 0x43, 0x73, 0x74, 0x61, 0x72, 0x74, 0x6D, 0x65, 0x61, 0x73, 0x03 };
    const char data_binary[] = { 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x10, 0x73, 0x4D, 0x4E, 0x20, 0x4C, 0x4D, 0x43, 0x73, 0x74, 0x61, 0x72, 0x74, 0x6D, 0x65, 0x61, 0x73, 0x68 };
    const char *data = m_cola_protocol_binary ? data_binary : data_hex;
    int size = m_cola_protocol_binary ? sizeof(data_binary) : sizeof(data_hex);

    int written_size = write(data, size);
    if (written_size != size) return false;
    if (!waitForBytesWritten(TIMEOUT_READ_WRITE)) return false;

    if (!waitForReadyRead(TIMEOUT_READ_WRITE)) return false;
    QByteArray ba = readAll();

    return !isErrorReturned(ba);
}

// _________________________________________________________
bool SickTIM651::stop_measurement()
{
    const char data_hex[] = { 0x02, 0x73, 0x4D, 0x4E, 0x20, 0x4C, 0x4D, 0x43, 0x73, 0x74, 0x6F, 0x70, 0x6D, 0x65, 0x61, 0x73, 0x03 };
    const char data_binary[] = { 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x0F, 0x73, 0x4D, 0x4E, 0x20, 0x4C, 0x4D, 0x43, 0x73, 0x74, 0x6F, 0x70, 0x6D, 0x65, 0x61, 0x73, 0x10 };
    const char *data = m_cola_protocol_binary ? data_binary : data_hex;
    int size = m_cola_protocol_binary ? sizeof(data_binary) : sizeof(data_hex);

    int written_size = write(data, size);
    if (written_size != size) return false;
    if (!waitForBytesWritten(TIMEOUT_READ_WRITE)) return false;

    if (!waitForReadyRead(TIMEOUT_READ_WRITE)) return false;
    QByteArray ba = readAll();

    return !isErrorReturned(ba);
}

// _________________________________________________________
bool SickTIM651::get_firmware_version(QByteArray &ba)
{
    const char data_hex[] = { 0x02, 0x73, 0x52, 0x4E, 0x20, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74, 0x03 };
    const char data_binary[] = { 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x0F, 0x73, 0x52, 0x4E, 0x20, 0x44, 0x65, 0x76, 0x69, 0x63, 0x65, 0x49, 0x64, 0x65, 0x6E, 0x74, 0x25 };
    const char *data = m_cola_protocol_binary ? data_binary : data_hex;
    int size = m_cola_protocol_binary ? sizeof(data_binary) : sizeof(data_hex);

    int written_size = write(data, size);
    if (written_size != size) return false;
    if (!waitForBytesWritten(TIMEOUT_READ_WRITE)) return false;

    if (!waitForReadyRead(TIMEOUT_READ_WRITE)) return false;
    ba = readAll();
    return  (ba.size() > 0);
}

// _________________________________________________________
bool SickTIM651::run()
{
    const char data_hex[] = { 0x02, 0x73, 0x4D, 0x4E, 0x20, 0x52, 0x75, 0x6E, 03 };
    const char data_binary[] = { 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x07, 0x73, 0x4D, 0x4E, 0x20, 0x52, 0x75, 0x6E, 0x19 };
    const char *data = m_cola_protocol_binary ? data_binary : data_hex;
    int size = m_cola_protocol_binary ? sizeof(data_binary) : sizeof(data_hex);

    int written_size = write(data, size);
    if (written_size != size) return false;
    if (!waitForBytesWritten(TIMEOUT_READ_WRITE)) return false;

    if (!waitForReadyRead(TIMEOUT_READ_WRITE)) return false;
    QByteArray ba = readAll();

    return !isErrorReturned(ba);
}

// _________________________________________________________
bool SickTIM651::poll_one_telegram(CLidarData *scan_data)
{
    const char data_hex[] = { 0x02, 0x73, 0x52, 0x4E, 0x20, 0x4C, 0x4D, 0x44, 0x73, 0x63, 0x61, 0x6E, 0x64, 0x61, 0x74, 0x61, 03 };
    const char data_binary[] = { 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x0F, 0x73, 0x52, 0x4E, 0x20, 0x4C, 0x4D, 0x44, 0x73, 0x63, 0x61, 0x6E, 0x64, 0x61, 0x74, 0x61, 0x05 };
    const char *data = m_cola_protocol_binary ? data_binary : data_hex;
    int size = m_cola_protocol_binary ? sizeof(data_binary) : sizeof(data_hex);

    int written_size = write(data, size);
    if (written_size != size) return false;
    if (!waitForBytesWritten(TIMEOUT_READ_WRITE)) return false;

    // Lecture des données
    // Le buffer reçu peut être :
    //  - Soit la totalité du message à recevoir
    //  - Soit la première partie du message complet
    //  - Soit la nième partie du message complet
    //  - Soit la dernière partie du message complet
    //  Message composé de
    // 0x02 0x02 0x02 0x02 -> 4 octets d'entête
    // 4 octets pour le nombre N d'octet de données suivants
    // Les N octets
    // Le checksum
    int total_to_receive = 9; // au minimum l'entête et la longeur + checksum
    QByteArray message;
    int num_bloc = 0;

    while (message.size() < total_to_receive) {
        num_bloc++;
        if(!waitForReadyRead(TIMEOUT_READ_WRITE)) return false; // erreur de lecture ou plus rien à lire alors qu'on attendait encore des infos -> problème
        //qDebug() << "Num bloc" << num_bloc;
        message.append(readAll());
        if (message.size() >=8) {
            total_to_receive = ((message.at(4) << 24)&0xFF000000) | ((message.at(5) << 16)&0x00FF0000) | ((message.at(6) << 8)&0x0000FF00) | (message.at(7)&0x000000FF);
            total_to_receive += 9; // L'entête + la taille + checksum
            //qDebug() << "Taille totale à lire" << total_to_receive;
        }
        //qDebug() << "Taille deja recue" << message.size();
    }
    // En sortant d'ici, le message est composé de :
    // Entête(4 octets) + Longeur N(4 octets) + N octets de données + 1 octet de checksum
    if (message.size() < 9) return false;

    // Vérifie l'entête
    if ( (message.at(0) != 0x02) || (message.at(1) != 0x02) || (message.at(2) != 0x02) || (message.at(3) != 0x02) ) return false;

    // Vérifie le checksum du message (calculé sur les octets de données utiles (hors entête, hors longueur de message)
    unsigned char message_checksum = message.at(message.size() - 1); // checksum en dernière position du message
    unsigned char computed_checksum = 0;
    for (int i=8; i<(message.size()-1); i++) computed_checksum = computed_checksum ^ message.at(i);
    if (computed_checksum != message_checksum) return false;

    // Vérifie s'il ne s'agit pas d'une réponse négative
    if (isErrorReturned(message)) return false;

    // Interprète le message reçu
    if (!decodeTelegram(message, scan_data)) return false;
    emit newData(*scan_data);

    return true;
}

// _________________________________________________________
bool SickTIM651::isErrorReturned(const QByteArray &ba)
{
    int _first = m_cola_protocol_binary ? 8 : 4;
    if ( (ba[_first]==0x73) && (ba[_first+1]==0x46) && (ba[_first+2]==0x41) ) return true;
    return false;
}

// _________________________________________________________
/*!
 * \brief Décode un buffer binaire reçu en
 * \param[in] telegram le message brut reçu (message valide)
 * \param[out] scan_data la structure de donnée de sortie interprétée en récupérant uniquement les données pertinentes
 */
bool SickTIM651::decodeTelegram(const QByteArray telegram, CLidarData *scan_data)
{
    if (!scan_data) return false;
    // [8;10]       : sRA                       : 3 bytes
    // [11]         : Blank (0x20)              : 1 byte
    // [12;22]      : LDSscandata               : 11 bytes
    // [23]         : Blank (0x20)              : 1 byte
    // [24;25]      : Version number            : 2 bytes
    // [26;27]      : Device number             : 2 bytes
    // [28;31]      : Serial number             : 4 bytes
    // [32;33]      : Device status             : 2 bytes
    // [34;35]      : Telegram counter          : 2 bytes
    // [36;37]      : Scan counter              : 2 bytes
    // [38;41]      : Time since startup        : 4 bytes
    // [42;45]      : Time of transmission      : 4 bytes
    scan_data->m_timestamp = ((telegram[42]<<24)&0xFF000000) | ((telegram[43]<<16)&0x00FF0000) | ((telegram[44]<<8)&0x0000FF00) | (telegram[45]&0x000000FF);
    // [46;47]      : Status digital inputs     : 2 bytes
    // [48;49]      : Status digial outputs     : 2 bytes
    // [50;51]      : Former reserved           : 2 bytes
    // [52;55]      : Scan frequency            : 4 bytes
    scan_data->m_scan_frequency = ( ((telegram[52]<<24)&0xFF000000) | ((telegram[53]<<16)&0x00FF0000) | ((telegram[54]<<8)&0x0000FF00) | (telegram[55]&0x000000FF) )/100; // [Hz]
    // [56;59]      : Measurement frequency     : 4 bytes
    // [60;61]      : Amount of encoder         : 2 bytes
    // [62;63]      : Amount of 16 bits channels: 2 bytes
    // [64;68]      : Content of output channel : 5 bytes (DIST1)
    // [69;72]      : Scale factor              : 4 bytes
    // [73;76]      : Scale factor offset       : 4 bytes
    // [77;80]      : Start angle               : 4 bytes
    scan_data->m_start_angle = ((signed int)( ((telegram[77]<<24)&0xFF000000) | ((telegram[78]<<16)&0x00FF0000) | ((telegram[79]<<8)&0x0000FF00) | (telegram[80]&0x000000FF) ))/10000.; // [°]
    // [81;82]      : Angle step resolution     : 2 bytes
    scan_data->m_angle_step_resolution = ( ((telegram[81]<<8)&0xFF00) | (telegram[82]&0x00FF) )/10000.; // [°]
    // [83;84]      : Amount of data N          : 2 bytes
    scan_data->m_measures_count = ( ((telegram[83]<<8)&0xFF00) | (telegram[84]&0x00FF) );
    if (scan_data->m_measures_count > CLidarData::MAX_MEASURES_COUNT) return false; // problème : impossible de ranger les valeurs dans le tableau de taille insuffisante
    // [85;85+(2*N-1)] : data                   : 2*N bytes
    int start_index_data = 85;
    for (int i=0; i<scan_data->m_measures_count; i++) {
        scan_data->m_dist_measures[i] = ( ((telegram[start_index_data]<<8)&0xFF00) | (telegram[start_index_data+1]&0x00FF) );
        start_index_data+=2;
    }
    return true;
}

// _________________________________________________________
void SickTIM651::on_connect()
{
    m_timer_autoreconnect.stop();
    qDebug() << "Connected to LIDAR";
}

// _________________________________________________________
void SickTIM651::on_disconnect()
{
    qDebug() << "ExchangerClient::on_disconnect : autoreconnect" << m_enable_autoreconnect;
    if (m_enable_autoreconnect) m_timer_autoreconnect.start(AUTORECONNECT_PERIOD);
}

// _________________________________________________________
void SickTIM651::on_tick_timer_autoreconnect()
{
    qDebug() << "Tick SickTIM651";
    QTcpSocket::connectToHost(m_hostname, m_port);
}
