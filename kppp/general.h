/* -*- C++ -*-
 *
 *            kPPP: A pppd front end for the KDE project
 *
 * $Id$
 * 
 *            Copyright (C) 1997 Bernd Johannes Wuebben 
 *                   wuebben@math.cornell.edu
 *
 * based on EzPPP:
 * Copyright (C) 1997  Jay Painter
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef _GENERAL_H_
#define _GENERAL_H_


#include <string.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qcombo.h>
#include <qlined.h>
#include <qlabel.h>
#include <qchkbox.h>
#include <qradiobt.h>
#include "modemcmds.h"
#include "pppdata.h"
#include <qchkbox.h>
#include "modeminfo.h"
#include "miniterm.h"

class GeneralWidget : public QWidget {
  
  Q_OBJECT

public:
  
  GeneralWidget( QWidget *parent=0, const char *name=0 );

private slots:
  
  void pppdpathchanged(const char*);
  void pppdtimeoutchanged(const char *n);
  void logviewerchanged(const char*);
  void setenter(int);
  void caption_toggled(bool);
  void redial_toggled(bool on);
  void xserver_toggled(bool on);


private:

  QGroupBox *box;
  QLabel *label1;
  QLabel *label2;
  QLabel *label3;
  QLabel *label4;
  QLabel *label5;
  QLabel *label6;
  QLabel *labeltmp;

  QCheckBox *chkbox1;
  QCheckBox *chkbox2;
  QCheckBox *chkbox3;
  QCheckBox *chkbox4;

  QLineEdit *pppdtimeout;
  QLineEdit *logviewer;
  QLabel    *logviewerlabel;
  QLineEdit *pppdpath;
  QComboBox *enter;

};


class ModemWidget : public QWidget {

  Q_OBJECT

public:

  ModemWidget( QWidget *parent=0, const char *name=0 );


private slots:

  void setmodemdc(int);
  void setflowcontrol(int);
  void modemcmdsbutton();
  void query_modem();
  void query_done();
  void terminal();
  void modemtimeoutchanged(const char*);
  void busywaitchanged(const char*);
  void modemlockfilechanged(const char*);

private:

  QGroupBox *box;
  QLabel *label1;
  QLabel *label2;
  QLabel *label3;
  QLabel *label4;
  QLabel *labeltmp;
  ModemTransfer *modemtrans;

  QComboBox *modemdevice;
  QComboBox *flowcontrol;

  QPushButton *modemcmds;
  QPushButton *modeminfo_button;
  QPushButton *terminal_button;
  
  QLineEdit *modemtimeout;
  QLineEdit *busywait;
  QLineEdit *modemlockfile;
};

class AboutWidget : public QWidget {

  Q_OBJECT

public:
  
  AboutWidget( QWidget *parent=0, const char *name=0 );



private:
  
  QGroupBox *box;
  QLabel *label1;
  QLabel *label2;
  QLabel *label3;


};


#endif
