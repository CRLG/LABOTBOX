#include "lidar_filter_params.h"
#include "lidar_data_filter_base.h"

CLidarFilterParams::CLidarFilterParams(CLidarDataFilterBase *filter, QWidget *parent)
    : QWidget(parent),
      m_filter(filter)
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

    connect(m_ihm.params_list, SIGNAL(currentTextChanged(QString)), this, SLOT(selected_param_change(QString)));
    connect(m_ihm.pb_apply, SIGNAL(clicked(bool)), this, SLOT(apply_value()));

    selected_param_change(m_ihm.params_list->currentText());  // s'assure que lorsaue la fenetre est cree, la valeur correspond au parametre affiche
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
