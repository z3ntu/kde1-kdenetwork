/**********************************************************************

	--- Dlgedit generated file ---

	File: PostDialogData.cpp
	Last generated: Fri Jul 18 20:30:46 1997

	DO NOT EDIT!!!  This file will be automatically
	regenerated by dlgedit.  All changes will be lost.

 *********************************************************************/

#include "PostDialogData.h"

#define Inherited QDialog

#include <qlabel.h>
#include <qpushbt.h>
#include <qscrbar.h>

PostDialogData::PostDialogData
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, TRUE )
{
	errorList = new QMultiLineEdit( this, "ErrorList" );
	errorList->setGeometry( 10, 10, 340, 120 );
	errorList->insertLine( "" );
	errorList->setReadOnly( FALSE );
	errorList->setOverwriteMode( FALSE );

	QScrollBar* tmpQScrollBar;
	tmpQScrollBar = new QScrollBar( this, "ErrorScroll" );
	tmpQScrollBar->setGeometry( 350, 10, 20, 120 );

	postNBtn = new QPushButton( this, "PostButton" );
	postNBtn->setGeometry( 10, 140, 100, 30 );
	connect( postNBtn, SIGNAL(clicked()), SLOT(postNow()) );
	postNBtn->setText( "Post now" );
	postNBtn->setAutoRepeat( FALSE );
	postNBtn->setAutoResize( FALSE );

	editBtn = new QPushButton( this, "EditButton" );
	editBtn->setGeometry( 380, 300, 100, 30 );
	connect( editBtn, SIGNAL(clicked()), SLOT(editPart()) );
	editBtn->setText( "Edit part" );
	editBtn->setAutoRepeat( FALSE );
	editBtn->setAutoResize( FALSE );

	cancelBtn = new QPushButton( this, "CancelButton" );
	cancelBtn->setGeometry( 270, 140, 100, 30 );
	connect( cancelBtn, SIGNAL(clicked()), SLOT(cancel()) );
	cancelBtn->setText( "Cancel" );
	cancelBtn->setAutoRepeat( FALSE );
	cancelBtn->setAutoResize( FALSE );

	partList = new QListBox( this, "PartList" );
	partList->setGeometry( 10, 260, 360, 210 );
	connect( partList, SIGNAL(highlighted(int)), SLOT(select(int)) );
	connect( partList, SIGNAL(selected(int)), SLOT(editPart(int)) );
	partList->setFrameStyle( 51 );
	partList->setLineWidth( 2 );

	addBtn = new QPushButton( this, "AddButton" );
	addBtn->setGeometry( 380, 260, 100, 30 );
	connect( addBtn, SIGNAL(clicked()), SLOT(addPart()) );
	addBtn->setText( "Add part" );
	addBtn->setAutoRepeat( FALSE );
	addBtn->setAutoResize( FALSE );

	removeBtn = new QPushButton( this, "RemoveButton" );
	removeBtn->setGeometry( 380, 440, 100, 30 );
	connect( removeBtn, SIGNAL(clicked()), SLOT(removePart()) );
	removeBtn->setText( "Remove part" );
	removeBtn->setAutoRepeat( FALSE );
	removeBtn->setAutoResize( FALSE );

	propsBtn = new QPushButton( this, "PropsButton" );
	propsBtn->setGeometry( 380, 340, 100, 30 );
	connect( propsBtn, SIGNAL(clicked()), SLOT(propsPart()) );
	propsBtn->setText( "Part properties" );
	propsBtn->setAutoRepeat( FALSE );
	propsBtn->setAutoResize( FALSE );

	QPushButton* tmpQPushButton;
	tmpQPushButton = new QPushButton( this, "HelpButton" );
	tmpQPushButton->setGeometry( 380, 10, 100, 30 );
	connect( tmpQPushButton, SIGNAL(clicked()), SLOT(showHelp()) );
	tmpQPushButton->setText( "Help" );
	tmpQPushButton->setAutoRepeat( FALSE );
	tmpQPushButton->setAutoResize( FALSE );

	helpLabel = new QLabel( this, "Long help" );
	helpLabel->setGeometry( 10, 470, 470, 30 );
	helpLabel->setFrameStyle( 17 );
	helpLabel->setText( "" );
	helpLabel->setAlignment( 289 );
	helpLabel->setMargin( -1 );

	postLBtn = new QPushButton( this, "LaterButton" );
	postLBtn->setGeometry( 120, 140, 100, 30 );
	connect( postLBtn, SIGNAL(clicked()), SLOT(postLater()) );
	postLBtn->setText( "Post later" );
	postLBtn->setAutoRepeat( FALSE );
	postLBtn->setAutoResize( FALSE );

	QLabel* tmpQLabel;
	tmpQLabel = new QLabel( this, "Label_2" );
	tmpQLabel->setGeometry( 10, 180, 100, 30 );
	tmpQLabel->setText( "Subject" );
	tmpQLabel->setAlignment( 289 );
	tmpQLabel->setMargin( -1 );

	subject = new QLineEdit( this, "LineEdit_1" );
	subject->setGeometry( 120, 180, 250, 30 );
	subject->setText( "" );
	subject->setMaxLength( 32767 );
	subject->setEchoMode( QLineEdit::Normal );
	subject->setFrame( TRUE );

	tmpQLabel = new QLabel( this, "Label_3" );
	tmpQLabel->setGeometry( 10, 220, 100, 30 );
	tmpQLabel->setText( "Newsgroup" );
	tmpQLabel->setAlignment( 289 );
	tmpQLabel->setMargin( -1 );

	newsgroup = new QLineEdit( this, "LineEdit_2" );
	newsgroup->setGeometry( 120, 220, 250, 30 );
	newsgroup->setText( "" );
	newsgroup->setMaxLength( 32767 );
	newsgroup->setEchoMode( QLineEdit::Normal );
	newsgroup->setFrame( TRUE );

	resize( 490, 510 );
}


PostDialogData::~PostDialogData()
{
}
void PostDialogData::postNow()
{
}
void PostDialogData::editPart()
{
}
void PostDialogData::cancel()
{
}
void PostDialogData::select(int)
{
}
void PostDialogData::editPart(int)
{
}
void PostDialogData::addPart()
{
}
void PostDialogData::removePart()
{
}
void PostDialogData::propsPart()
{
}
void PostDialogData::showHelp()
{
}
void PostDialogData::postLater()
{
}
