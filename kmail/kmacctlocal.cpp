// kmacctlocal.cpp

#include "kmacctlocal.h"
#include "kmfolder.h"
#include "kmmessage.h"
#include "kmacctfolder.h"
#include "kmglobal.h"

#include <kapp.h>
#include <kconfig.h>
#include <kmsgbox.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifdef __FreeBSD__
#include <paths.h>	/* defines _PATH_MAILDIR */
#endif
#ifndef _PATH_MAILDIR
#define _PATH_MAILDIR "/var/spool/mail"
#endif


//-----------------------------------------------------------------------------
KMAcctLocal::KMAcctLocal(KMAcctMgr* aOwner, const char* aAccountName):
  KMAcctLocalInherited(aOwner, aAccountName)
{
  initMetaObject();
}


//-----------------------------------------------------------------------------
KMAcctLocal::~KMAcctLocal()
{
  mLocation = "";
}


//-----------------------------------------------------------------------------
const char* KMAcctLocal::type(void) const
{
  return "local";
}


//-----------------------------------------------------------------------------
void KMAcctLocal::init(void)
{
  mLocation = _PATH_MAILDIR;
  mLocation.detach();
  mLocation += "/";
  mLocation += getenv("USER");
}


//-----------------------------------------------------------------------------
bool KMAcctLocal::processNewMail(KMIOStatus *statusWdg)
{
  KMFolder mailFolder(NULL, location());
  long num = 0;
  long i;
  int rc;
  KMMessage* msg;
  bool addedOk;
  
  if (mFolder==NULL) return FALSE;

  debug("processNewMail: %s", (const char*)location());

  statusWdg->prepareTransmission(location(), KMIOStatus::RETRIEVE);
  app->processEvents();
  mailFolder.setAutoCreateIndex(FALSE);
  rc = mailFolder.open();
  if (rc)
  {
    QString aStr;
    aStr = i18n("Cannot open file:");
    aStr += mailFolder.path()+"/"+mailFolder.name();
    KMsgBox::message(0,"KMail notification",aStr);
    perror("cannot open file "+mailFolder.path()+"/"+mailFolder.name());
    return FALSE;
  }

  mFolder->quiet(TRUE);
  mFolder->open();
		       

  num = mailFolder.count();

  addedOk = true;
  
  for (i=0; i<num; i++)
  {
    //if(statusWdg->abortRequested())
    //break;

    if (!addedOk) break;
    
    statusWdg->updateProgressBar(i,num);
    msg = mailFolder.take(0);
    if (msg)
    {
      msg->setStatus(msg->headerField("Status"), msg->headerField("X-Status"));
      addedOk = processNewMsg(msg);
    }
    app->processEvents();
  }
  if (addedOk)
  {
    rc = mailFolder.expunge();
  
    if (rc)
      warning(i18n("Cannot remove mail from\nmailbox `%s':\n%s"),
              (const char*)mailFolder.location(), strerror(rc));
  }
  // else warning is written already

  mailFolder.close();
  mFolder->close();
  mFolder->quiet(FALSE);
  statusWdg->transmissionCompleted();

  return (num > 0);
}


//-----------------------------------------------------------------------------
void KMAcctLocal::readConfig(KConfig& config)
{
  QString defaultPath(_PATH_MAILDIR);
  defaultPath += "/";
  defaultPath += getenv("USER");

  KMAcctLocalInherited::readConfig(config);
  mLocation = config.readEntry("Location", defaultPath);
}


//-----------------------------------------------------------------------------
void KMAcctLocal::writeConfig(KConfig& config)
{
  KMAcctLocalInherited::writeConfig(config);

  config.writeEntry("Location", mLocation);
}


//-----------------------------------------------------------------------------
void KMAcctLocal::setLocation(const QString& aLocation)
{
  mLocation = aLocation;
}
