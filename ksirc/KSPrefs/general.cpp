/**********************************************************************

	--- Qt Architect generated file ---

	File: general.cpp
	Last generated: Sun Jul 26 16:28:55 1998

 *********************************************************************/

#include "general.h"
#include "../config.h"
#include <kconfig.h>
#include <qfiledialog.h>

extern KConfig *kConfig;
extern global_config *kSircConfig;

#define Inherited generalData

general::general
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name )
{
  kConfig->setGroup("General");

  CB_AutoCreateWin->setChecked(kConfig->readNumEntry("AutoCreateWin", FALSE));
  kSircConfig->AutoCreateWin = kConfig->readNumEntry("AutoCreateWin", FALSE);

  CB_BeepNotify->setChecked(kConfig->readNumEntry("BeepNotify", TRUE));
  kSircConfig->BeepNotify = kConfig->readNumEntry("BeepNotify", TRUE);

  CB_NickCompletion->setChecked(kConfig->readNumEntry("NickCompletion", TRUE));
  kSircConfig->NickCompletion = kConfig->readNumEntry("NickCompletion", TRUE);

  CB_ColourPicker->setChecked(kConfig->readNumEntry("ColourPicker", TRUE));
  kSircConfig->ColourPicker = kConfig->readNumEntry("ColourPicker", TRUE);

  CB_AutoRejoin->setChecked(kConfig->readNumEntry("AutoRejoin", TRUE));
  kSircConfig->AutoRejoin = kConfig->readNumEntry("AutoRejoin", TRUE);

  // not yet =P
  CB_BackgroundPix->setChecked(kConfig->readNumEntry("BackgroundPix", FALSE));
  kSircConfig->BackgroundPix = kConfig->readNumEntry("BackgroundPix", FALSE);
  SLE_BackgroundFile->setText(kConfig->readEntry("BackgroundFile"));
  kSircConfig->BackgroundFile = kConfig->readEntry("BackgroundFile");
  connect(PB_BackgroundBrowse, SIGNAL(clicked()), 
          this, SLOT(slot_openBrowser()));

}

general::~general()
{
}

void general::slot_openBrowser()
{
  QFileDialog *FileDialog = new QFileDialog( ".", "*.gif");
  connect(FileDialog, SIGNAL(fileSelected(const char*)),
          this, SLOT(slot_setBackgroundFile(const char*)));
  FileDialog->show();
}

void general::slot_setBackgroundFile(const char* filename)
{ 
  SLE_BackgroundFile->setText( filename );
}

void general::slot_apply()
{
  kSircConfig->AutoCreateWin = CB_AutoCreateWin->isChecked();
  kSircConfig->BeepNotify = CB_BeepNotify->isChecked();
  kSircConfig->NickCompletion = CB_NickCompletion->isChecked();
  kSircConfig->ColourPicker = CB_ColourPicker->isChecked();
  kSircConfig->AutoRejoin = CB_AutoRejoin->isChecked();
  kSircConfig->BackgroundPix = CB_BackgroundPix->isChecked();
  kSircConfig->BackgroundFile = SLE_BackgroundFile->text();

  kConfig->setGroup("General");
  kConfig->writeEntry("AutoCreateWin", kSircConfig->AutoCreateWin);
  kConfig->writeEntry("BeepNotify", kSircConfig->BeepNotify);
  kConfig->writeEntry("NickCompletion", kSircConfig->NickCompletion);
  kConfig->writeEntry("ColourPicker", kSircConfig->ColourPicker);
  kConfig->writeEntry("AutoRejoin", kSircConfig->AutoRejoin);
  kConfig->writeEntry("BackgroundPix", kSircConfig->BackgroundPix);
  kConfig->writeEntry("BackgroundFile", kSircConfig->BackgroundFile);
  kConfig->sync();
}