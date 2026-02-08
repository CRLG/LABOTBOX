#include "lidar_data_filter_module_factory.h"

#include "lidar_data_filter_module_example.h"
#include "lidar_data_filter_module_tracker.h"

	   
// _____________________________________________________________________
CLidarDataFilterModuleFactory::CLidarDataFilterModuleFactory()
{
}


// _____________________________________________________________________
CLidarDataFilterModuleFactory::~CLidarDataFilterModuleFactory()
{
}

// _____________________________________________________________________
CLidarDataFilterModuleBase *CLidarDataFilterModuleFactory::createInstance(QString filter_name)
{
    if (filter_name.toLower().trimmed() == QString(CLidarDataFilterModuleExample().get_name()).toLower().trimmed() )
        return new CLidarDataFilterModuleExample();

    if (filter_name.toLower().trimmed() == QString(CLidarDataFilterModuleTracker().get_name()).toLower().trimmed() )
        return new CLidarDataFilterModuleTracker();

    return NULL;
}

// _____________________________________________________________________
QStringList CLidarDataFilterModuleFactory::getExisting()
{
    QStringList lst;

    lst << CLidarDataFilterModuleExample().get_name();
    lst << CLidarDataFilterModuleTracker().get_name();

    return lst;
}

