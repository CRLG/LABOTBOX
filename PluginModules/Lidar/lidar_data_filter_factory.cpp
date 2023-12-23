#include "lidar_data_filter_factory.h"

#include "lidar_data_filter_example.h"
#include "lidar_data_filter_tracker.h"

	   
// _____________________________________________________________________
CLidarDataFilterFactory::CLidarDataFilterFactory()
{
}


// _____________________________________________________________________
CLidarDataFilterFactory::~CLidarDataFilterFactory()
{
}

// _____________________________________________________________________
CLidarDataFilterBase* CLidarDataFilterFactory::createInstance(QString filter_name)
{
    if (filter_name.toLower().trimmed() == QString(CLidarDataFilterExample().get_name()).toLower().trimmed() )
        return new CLidarDataFilterExample();

    if (filter_name.toLower().trimmed() == QString(CLidarDataFilterTracker().get_name()).toLower().trimmed() )
        return new CLidarDataFilterTracker();

    return NULL;
}

// _____________________________________________________________________
QStringList CLidarDataFilterFactory::getExisting()
{
    QStringList lst;

    lst << CLidarDataFilterExample().get_name();
    lst << CLidarDataFilterTracker().get_name();

    return lst;
}

