#ifndef _LIDAR_FACTORY_H_
#define _LIDAR_FACTORY_H_

#include <QStringList>
#include <QObject>

class LidarBase;

class LidarFactory
{
public:
    LidarFactory();

    static LidarBase* createInstance(QString lidar_name, QObject *parent=Q_NULLPTR);
    static QStringList getExisting();
};

#endif // _LIDAR_FACTORY_H_
