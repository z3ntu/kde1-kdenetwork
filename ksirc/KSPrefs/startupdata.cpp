/**********************************************************************

	--- Qt Architect generated file ---

	File: startupdata.cpp
	Last generated: Sat Jan 17 19:44:50 1998

	DO NOT EDIT!!!  This file will be automatically
	regenerated by qtarch.  All changes will be lost.

 *********************************************************************/

#include "startupdata.h"

#define Inherited QWidget

#include <qlabel.h>
#include <qbttngrp.h>
#include <qpushbt.h>

startupdata::startupdata
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, 0 )
{
	QButtonGroup* dlgedit_ButtonGroup_1;
	dlgedit_ButtonGroup_1 = new QButtonGroup( this, "ButtonGroup_1" );
	dlgedit_ButtonGroup_1->setGeometry( 10, 90, 380, 200 );
	dlgedit_ButtonGroup_1->setMinimumSize( 10, 10 );
	dlgedit_ButtonGroup_1->setMaximumSize( 32767, 32767 );
	dlgedit_ButtonGroup_1->setFrameStyle( 49 );
	dlgedit_ButtonGroup_1->setTitle( "Notify List" );
	dlgedit_ButtonGroup_1->setAlignment( 1 );

	QLabel* dlgedit_Label_1;
	dlgedit_Label_1 = new QLabel( this, "Label_1" );
	dlgedit_Label_1->setGeometry( 10, 10, 100, 30 );
	dlgedit_Label_1->setMinimumSize( 10, 10 );
	dlgedit_Label_1->setMaximumSize( 32767, 32767 );
	dlgedit_Label_1->setText( "Nick" );
	dlgedit_Label_1->setAlignment( 289 );
	dlgedit_Label_1->setMargin( -1 );

	QLabel* dlgedit_Label_2;
	dlgedit_Label_2 = new QLabel( this, "Label_2" );
	dlgedit_Label_2->setGeometry( 10, 50, 100, 30 );
	dlgedit_Label_2->setMinimumSize( 10, 10 );
	dlgedit_Label_2->setMaximumSize( 32767, 32767 );
	dlgedit_Label_2->setText( "Real Name" );
	dlgedit_Label_2->setAlignment( 289 );
	dlgedit_Label_2->setMargin( -1 );

	SLE_Nick = new QLineEdit( this, "LineEdit_1" );
	SLE_Nick->setGeometry( 120, 10, 270, 30 );
	SLE_Nick->setMinimumSize( 10, 10 );
	SLE_Nick->setMaximumSize( 32767, 32767 );
	SLE_Nick->setText( "" );
	SLE_Nick->setMaxLength( 32767 );
	SLE_Nick->setEchoMode( QLineEdit::Normal );
	SLE_Nick->setFrame( TRUE );

	SLE_RealName = new QLineEdit( this, "LineEdit_2" );
	SLE_RealName->setGeometry( 120, 50, 270, 30 );
	SLE_RealName->setMinimumSize( 10, 10 );
	SLE_RealName->setMaximumSize( 32767, 32767 );
	SLE_RealName->setText( "" );
	SLE_RealName->setMaxLength( 32767 );
	SLE_RealName->setEchoMode( QLineEdit::Normal );
	SLE_RealName->setFrame( TRUE );

	LB_Nicks = new QListBox( this, "ListBox_1" );
	LB_Nicks->setGeometry( 20, 110, 170, 170 );
	LB_Nicks->setMinimumSize( 10, 10 );
	LB_Nicks->setMaximumSize( 32767, 32767 );
	LB_Nicks->setFrameStyle( 51 );
	LB_Nicks->setLineWidth( 2 );
	LB_Nicks->setMultiSelection( FALSE );

	QPushButton* dlgedit_PushButton_1;
	dlgedit_PushButton_1 = new QPushButton( this, "PushButton_1" );
	dlgedit_PushButton_1->setGeometry( 200, 110, 180, 30 );
	dlgedit_PushButton_1->setMinimumSize( 10, 10 );
	dlgedit_PushButton_1->setMaximumSize( 32767, 32767 );
	connect( dlgedit_PushButton_1, SIGNAL(clicked()), SLOT(nicks_delete()) );
	dlgedit_PushButton_1->setText( "&Delete" );
	dlgedit_PushButton_1->setAutoRepeat( FALSE );
	dlgedit_PushButton_1->setAutoResize( FALSE );

	B_AddNicks = new QPushButton( this, "PushButton_2" );
	B_AddNicks->setGeometry( 200, 150, 180, 30 );
	B_AddNicks->setMinimumSize( 10, 10 );
	B_AddNicks->setMaximumSize( 32767, 32767 );
	connect( B_AddNicks, SIGNAL(clicked()), SLOT(nicks_add()) );
	B_AddNicks->setText( "&Add" );
	B_AddNicks->setAutoRepeat( FALSE );
	B_AddNicks->setAutoResize( FALSE );

	SLE_Nicks = new QLineEdit( this, "LineEdit_3" );
	SLE_Nicks->setGeometry( 200, 190, 180, 30 );
	SLE_Nicks->setMinimumSize( 10, 10 );
	SLE_Nicks->setMaximumSize( 32767, 32767 );
	connect( SLE_Nicks, SIGNAL(textChanged(const char*)), SLOT(sle_add_update(const char*)) );
	SLE_Nicks->setText( "" );
	SLE_Nicks->setMaxLength( 32767 );
	SLE_Nicks->setEchoMode( QLineEdit::Normal );
	SLE_Nicks->setFrame( TRUE );

	dlgedit_ButtonGroup_1->insert( dlgedit_PushButton_1 );
	dlgedit_ButtonGroup_1->insert( B_AddNicks );

	resize( 400,300 );
	setMinimumSize( 400, 300 );
	setMaximumSize( 400, 300 );
}


startupdata::~startupdata()
{
}
void startupdata::nicks_delete()
{
}
void startupdata::nicks_add()
{
}
void startupdata::sle_add_update(const char*)
{
}
