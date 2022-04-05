#include <QDebug>
#include <QThread>
#include "CKmar_simu.h"

CKmarSimu::CKmarSimu() :
    m_mouvement_init(this),
    m_mouvement_ramasse(this)
{

}

// _____________________________________________________
//!
//! \brief CKmarSimu::setAxisPosition
//! \param axis
//! \param pos
//! \param speed
//! \return
//!
bool CKmarSimu::setAxisPosition(int axis, int pos, int speed)
{
    qDebug() << "CKmarSimu::setAxisPosition" << axis << pos << speed;
    m_axis_goal_position[axis] = pos;
    return true;
}

// _____________________________________________________
//!
//! \brief CKmarSimu::setAxisSpeed
//! \param axis
//! \param speed
//! \return
//!
bool CKmarSimu::setAxisSpeed(int axis, int speed)
{
    return true;
}
// _____________________________________________________
//!
//! \brief CKmarSimu::getAxisCount
//! \return
//!
int CKmarSimu::getAxisCount()
{
    return (4);
}

// _____________________________________________________
//!
//! \brief CKmarSimu::isMoving
//! \param axis
//! \return
//!
bool CKmarSimu::isMoving(int axis)
{
    return (m_axis_goal_position[axis] != m_axis_pos[axis]);
}

// _____________________________________________________
//!
//! \brief CKmarSimu::getPosition
//! \param axis
//! \return
//!
int CKmarSimu::getPosition(int axis)
{
    return m_axis_pos[axis];
}

// _____________________________________________________
//!
//! \brief CKmarSimu::arm
//! \param axis
//!
void CKmarSimu::arm(int axis)
{
    qDebug() << "Arm axis " << axis;
}

// _____________________________________________________
//!
//! \brief CKmarSimu::disarm
//! \param axis
//!
void CKmarSimu::disarm(int axis)
{
    qDebug() << "Disarm axis " << axis;
}

// _____________________________________________________
//!
//! \brief CKmarSimu::getTime
//! \return
//!
int CKmarSimu::getTime()
{
}

// _____________________________________________________
//!
//! \brief CKmarSimu::delay_ms
//! \param delay
//!
void CKmarSimu::delay_ms(int delay)
{
    QThread::msleep(delay);
}

// _____________________________________________________
void CKmarSimu::compute()
{
    // Simule le déplacement des servos vers la consigne
    for (unsigned int i=0; i<getAxisCount(); i++) {
        int err = m_axis_pos[i] - m_axis_goal_position[i];
        if (err > 0) {  // Plus l'erreur est grande, plus le gain est grand
            if (abs(err) > 50) m_axis_pos[i] -= 50;
            else if (abs(err) > 10) m_axis_pos[i] -= 10;
            else if (abs(err) > 5) m_axis_pos[i] -= 5;
            else              m_axis_pos[i]--;
            qDebug() << QString("Position Axis[%1] = %2").arg(i).arg(m_axis_pos[i]);
        }
        else if (err < 0 ) {
            if (abs(err) > 50) m_axis_pos[i] += 50;
            else if (abs(err) > 10) m_axis_pos[i] += 10;
            else if (abs(err) > 5) m_axis_pos[i] += 5;
            else              m_axis_pos[i]++;
            qDebug() << QString("Position Axis[%1] = %2").arg(i).arg(m_axis_pos[i]);
        }
        // else ne rien faire

    }

    CKmarBase::compute();
}

// __________________________________________________
void CKmarSimu::start(int mouvement)
{
    switch(mouvement)
    {
    case MOUVEMENT_INIT :
        m_mouvement_en_cours = &m_mouvement_init;
        break;
    case MOUVEMENT_RAMASSE :
        m_mouvement_en_cours = &m_mouvement_ramasse;
        break;

    default :
        break;

    }
    if (m_mouvement_en_cours) m_mouvement_en_cours->start();
}
