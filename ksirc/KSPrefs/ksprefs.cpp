#include "ksprefs.h"

#include <qlayout.h>

KSPrefs::KSPrefs(QWidget * parent=0, const char * name=0)
  : QDialog(parent, name)
{
  pTab = new QTabDialog(this, "prefstabs");

  // Start Sub Dialog items.
  pStart = new StartUp(pTab);
  pServerChannel = new ServerChannel(pTab);

  pTab->addTab(pStart, "&StartUp");
  pTab->addTab(pServerChannel, "Servers/&Channels");

  connect(pTab, SIGNAL(applyButtonPressed()),
	  pStart, SLOT(slot_apply()));
  connect(pTab, SIGNAL(applyButtonPressed()),
	  pServerChannel, SLOT(slot_apply()));

  resize(420, 400);

  // Connect this one last since it deletes the widget.
  connect(pTab, SIGNAL(applyButtonPressed()),
	  this, SLOT(slot_apply()));
  connect(pTab, SIGNAL(cancelButtonPressed()),
	  this, SLOT(slot_cancel()));
  connect(pTab, SIGNAL(defaultButtonPressed()),
	  this, SLOT(slot_cancel()));

}

KSPrefs::~KSPrefs(){
  delete pTab;
  pTab = 0;
}

void KSPrefs::resizeEvent ( QResizeEvent * )
{
  pTab->setGeometry(0, 0, width(), height());
}

void KSPrefs::slot_apply()
{
  delete this;
}

void KSPrefs::slot_cancel()
{
  delete this;
}

void KSPrefs::hide()
{
  delete this;
}

