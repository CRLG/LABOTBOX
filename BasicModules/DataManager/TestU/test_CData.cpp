/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QDebug>
#include "test.h"
#include "CData.h"


// =======================================================
//      FONCTIONS DE TEST DE LA CLASSE CDdata
// =======================================================


// ________________________________________________________
void Test::test_read_write()
{
  CData myData;

  myData.write("40");
  QCOMPARE(myData.read().toInt(), 40);

  myData.write("12.34");
  QCOMPARE(myData.read().toDouble(), 12.34);

  myData.write("Bonjour");
  QCOMPARE(myData.read().toString(), QString("Bonjour"));
}


// ______________________________________________
void Test::test_connect_write(void)
{
 CTest_connect test_connect;
 unsigned long memo_cpt_passage_slot;

 test_connect.m_data.write("123.456");  // écrit dans la data
 QCOMPARE(test_connect.m_data_variant.toDouble(), 123.456); // vérifie que le slot connecté sur le signal de la data a été appelé

 test_connect.m_data.write("Hello");  // écrit dans la data
 QCOMPARE(test_connect.m_data_variant.toString(), QString("Hello")); // vérifie que le slot connecté sur le signal de la data a été appelé

 test_connect.m_data.write("Nom.champ1.sous_champ1");  // écrit dans la data
 QCOMPARE(test_connect.m_data_variant.toString(), QString("Nom.champ1.sous_champ1")); // vérifie que le slot connecté sur le signal de la data a été appelé

 // Vérifie que si la valeur n'a pas changé, le signal valuechanged n'est pas émis
 test_connect.m_data.write("123.456");
 memo_cpt_passage_slot = test_connect.m_nbre_appel_slot;  // mémorise le nombre de passage dans le slot
 test_connect.m_data.write("123.456");  // écrit la même valeur que précédemment
 QCOMPARE(test_connect.m_nbre_appel_slot, memo_cpt_passage_slot); // vérifie que le signal n'a pas été émis (car la valeur de la data n'a pas changé)
}


// =======================================================
//      CLASSE DE TEST DE CONNECTION
// =======================================================

// ______________________________________________
CTest_connect::CTest_connect()
  :  m_nbre_appel_slot(0)
{
 connect(&m_data, SIGNAL(valueChanged(QVariant)), this, SLOT(test_slot_data_changed(QVariant)));
}

// ______________________________________________
void CTest_connect::test_slot_data_changed(QVariant data)
{
 m_data_variant = data;
 m_nbre_appel_slot++;
}
