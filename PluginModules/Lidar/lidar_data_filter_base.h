#ifndef _LIDARDATA_FILTER_BASE_H_
#define _LIDARDATA_FILTER_BASE_H_

// Clase de base pour tous les filtres Lidar
class CLidarData;
class CLidarDataFilterBase
{
public:
    CLidarDataFilterBase() { }
    virtual ~CLidarDataFilterBase() { }

    virtual void filter(const CLidarData *data_in, CLidarData *data_out) = 0;  // virtuelle pure : à implémenter dans les classes dérivées pour implémenter le filtre
    virtual const char *get_name()=0;
    virtual const char *get_description() { return ""; } // pas obligatoire

    const int POINT_IGNORED = 99999999;
};

#endif // _LIDARDATA_FILTER_BASE_H_
