#include <QSettings>
#include "lidar_filter_params.h"
#include "lidar_data_filter_module_base.h"

CLidarFilterParams::CLidarFilterParams(CLidarDataFilterModuleBase *filter, QString eeprom_pathfilename, QWidget *parent)
    : QWidget(parent),
      m_filter(filter),
      m_eeprom_pathfilename(eeprom_pathfilename)
{
    m_ihm.setupUi(this);

    if (filter) {
        QStringList var_lst;
        filter->m_data_manager.getListeVariablesName(var_lst);
        m_ihm.params_list->addItems(var_lst);
        m_ihm.params_list->model()->sort(0, Qt::DescendingOrder);  // trie par ordre alphabetique
        setWindowTitle(QString("Filter Params: %1").arg(filter->get_name()));
        setToolTip(filter->get_description());
    }
    else {  // grise toute l'IHM pour indiquer que quelque chose ne va pas
        setEnabled(false);
    }

    m_ihm.pb_save_eeprom->setToolTip(QString("Save to: %1").arg(m_eeprom_pathfilename));

    connect(m_ihm.params_list, SIGNAL(currentTextChanged(QString)), this, SLOT(selected_param_change(QString)));
    connect(m_ihm.pb_apply, SIGNAL(clicked(bool)), this, SLOT(apply_value()));
    connect(m_ihm.pb_save_eeprom, SIGNAL(clicked(bool)), this, SLOT(save()));

    // charge les parametres depuis le fichier de configuration
    fromFile(m_eeprom_pathfilename);

    selected_param_change(m_ihm.params_list->currentText());  // s'assure que lorsque la fenetre est cree, la valeur correspond au parametre affiche
}

// _____________________________________________________________________
/*!
 * \brief Lit les parametres d'un fichier de configuration et affecte les parametres trouves
 * \param pathfilename le chemin + nom du fichier contenant les parametres
 */
void CLidarFilterParams::fromFile(QString pathfilename)
{
    QSettings settings(pathfilename, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    QStringList lst_var;
    int loaded_count=0;
    m_filter->m_data_manager.getListeVariablesName(lst_var);
    foreach (QString param_name, lst_var) {
        CData *data = m_filter->m_data_manager.getData(param_name);
        if (data) {
            // la section du fichier .ini est contenue dans le nom du parametre
            QString section_and_name = QString("%1/%2").arg(getEEPROMFileSection()).arg(param_name);
            QVariant var_val = settings.value(section_and_name); // lit la valeur du parametre dans le fichier si elle existe
            if (!var_val.isNull()) { // pour tenir compte du fait que le parametre n'est peut etre pas renseigne dans le fichier de parametrage, dans ce cas, laisse la valeur actuelle au parametre
                data->write(var_val);
                loaded_count++;
            }
        }
    }
    m_ihm.eeprom_filename->setText(QString("%1 params loaded from file: %2").arg(loaded_count).arg(m_eeprom_pathfilename));
}

// _____________________________________________________________________
/*!
 * \brief Ecrit les parametres dans un fichier de configuration
 * \param pathfilename le chemin + nom du fichier a ecrire
 */
void CLidarFilterParams::toFile(QString pathfilename)
{
    if (!m_filter) return;

    QSettings settings(pathfilename, QSettings::IniFormat);
    settings.setIniCodec("UTF-8");

    QStringList lst_var;
    m_filter->m_data_manager.getListeVariablesName(lst_var);
    foreach (QString param_name, lst_var) {
        CData *data = m_filter->m_data_manager.getData(param_name);
        if (data) {
            // la section du fichier .ini est contenue dans le nom du parametre
            QString section_and_name = QString("%1/%2").arg(getEEPROMFileSection()).arg(param_name);
            settings.setValue(section_and_name, data->read());
        }
    }
    m_ihm.eeprom_filename->setText(QString("%1 params saved to file: %2").arg(lst_var.size()).arg(m_eeprom_pathfilename));
}

// _____________________________________________________________________
/*!
 * \brief construit la section concernee du fichier EEPROM pour le filtre concerne
 * \return le nom de la section (= base sur le nom du filtre)
 */
QString CLidarFilterParams::getEEPROMFileSection()
{
    if (!m_filter) return "";
    QString name = m_filter->get_name();
    name = name.simplified().replace(" ", "_");
    return name;
}

// _____________________________________________________________________
void CLidarFilterParams::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    emit closed();
}

// _____________________________________________________________________
void CLidarFilterParams::apply_value()
{
    if (!m_filter) return;
    CData *data = m_filter->m_data_manager.getData(m_ihm.params_list->currentText());
    if (!data) return;
    data->write(m_ihm.param_value->value());
}

// _____________________________________________________________________
void CLidarFilterParams::selected_param_change(QString param_name)
{
    if (!m_filter) return;
    CData *data = m_filter->m_data_manager.getData(param_name);
    if (data) m_ihm.param_value->setValue(data->read().toDouble());
}

// _____________________________________________________________________
void CLidarFilterParams::save()
{
    toFile(m_eeprom_pathfilename);
}
