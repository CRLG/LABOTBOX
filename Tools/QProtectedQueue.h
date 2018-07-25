/*! \file QProtectedQueue.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _QPROTECTED_QUEUE_H_
#define _QPROTECTED_QUEUE_H_

#include <QList>
#include <QMutex>
#include <QMutexLocker>

 /*! \addtogroup Toold
   * 
   *  @{
   */

template <class T>
/*! @brief class QProtectedQueue
 * Implémente une classe FIFO avec protection contre les accès concurents (par mutex)
 * Cette classe est implémentée sur le modèle de la classe QQueue de Qt en y ajoutant des mutex
 */
class QProtectedQueue : public QList<T>
{
public:
    inline QProtectedQueue() {}
    inline ~QProtectedQueue() {}
    inline void swap(QProtectedQueue<T> &other)	{ QMutexLocker locker(&m_mutex); QList<T>::swap(other); } // prevent QList<->QQueue swaps
    inline void enqueue(const T &t) 			{ QMutexLocker locker(&m_mutex); QList<T>::append(t); }
    inline T dequeue() 							{ QMutexLocker locker(&m_mutex); return QList<T>::takeFirst(); }
    inline T &head() 							{ QMutexLocker locker(&m_mutex); return QList<T>::first(); }
    inline const T &head() const 				{ QMutexLocker locker(&m_mutex); return QList<T>::first(); }
private :
    QMutex m_mutex;
};

#endif // _QPROTECTED_QUEUE_H_

/*! @} */

