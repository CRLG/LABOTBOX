#include "lidar_factory.h"
#include "lidar_base.h"

#include "sick_tim561.h"
#include "ydlidar_tminiplus.h"

LidarFactory::LidarFactory()
{
}

// _____________________________________________________________________
LidarBase *LidarFactory::createInstance(QString lidar_name, QObject *parent)
{
    QString lidar_name_simple = lidar_name.toLower().simplified();
    if (lidar_name_simple == QString(SickTIM651().get_name()).toLower().simplified() )
        return new SickTIM651(parent);

    if (lidar_name_simple == QString(YDLIDAR_TminiPlus().get_name()).toLower().simplified() )
        return new YDLIDAR_TminiPlus(parent);

    return Q_NULLPTR;
}

// _____________________________________________________________________
QStringList LidarFactory::getExisting()
{
    QStringList lst;

    lst << SickTIM651().get_name();
    lst << YDLIDAR_TminiPlus().get_name();

    return lst;
}
