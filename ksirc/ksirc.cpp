/*************************************************************************

 Main KSirc start 

 $$Id$$

 Main start file that defines 3 global vars, etc

 Does nothing to special, start create new KApplcaiton (incorrectly,
 btw, should use the code Kalle posted so it can be session managment
 restarted, etc) loads colours then fires off the main widget.  On
 exit it calls a sync so the kConfig get written.  (Is this a bug?)

*************************************************************************/

/*
 * Needed items
 * 1. Move the fork and dsirc out of the main, and into the ksirc object
 * 2. Multiple toplevel widgets.
 * 3. Handle sig chils and sig pipes to control dsircc
 * 4. Send a /quit and/or kill dsirc on exit
 * */


//#include "toplevel.h"
//#include "ksircprocess.h"
//#include "iocontroller.h"
//#include "ioBroadcast.h"
#include "servercontroller.h"
#include <iostream.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <qfont.h>

#include <kapp.h>
#include <kconfig.h>

#include "config.h"

KApplication *kApp;
KConfig *kConfig;
global_config *kSircConfig;
//QDict<KSircTopLevel> TopList;
//QDict<KSircMessageReceiver> TopList;

int main( int argc, char ** argv )
{
  // Start the KDE application
  kApp = new KApplication( argc, argv, QString("ksirc") );

  // Starts the toplevel and give it sirc pipe items.


  // Get config, and setup internal structure.

  kConfig = kApp->getConfig();

  kSircConfig = new global_config;

  kConfig->setGroup("Colours");
  kSircConfig->colour_text = new QColor(kConfig->readColorEntry("text", &black));
  kSircConfig->colour_info = new QColor(kConfig->readColorEntry("info", &blue));
  kSircConfig->colour_chan = new QColor(kConfig->readColorEntry("chan", &green));
  kSircConfig->colour_error = new QColor(kConfig->readColorEntry("error", &red));

  kSircConfig->colour_background = 0;

  kSircConfig->kdedir = getenv("KDEDIR");
  if(kSircConfig->kdedir.isEmpty()){
    kSircConfig->kdedir = "/usr/local/kde";
  }

  kConfig->setGroup("GlobalOptions");
  kSircConfig->defaultfont = kConfig->readFontEntry("MainFont", new QFont("fixed"));

  //  KSircProcess proc("opus.dal.net");
  servercontroller *control = new servercontroller();
  control->show();

  //  KSircTopLevel *toplevel = new KSircTopLevel();
  //  TopList.insert("default", toplevel);
  //  TopList.insert("!all", new KSircIOBroadcast());

  //  ioc = new KSircIOController(sirc_stdin, sirc_stdout, sirc_stderr);

  //  toplevel->show();
  
  kApp->setMainWidget(control);

  //  kApp->connect(kApp, SIGNAL(lastWindowClosed()), kApp, SLOT(quit()));
  
  kApp->exec();

  kConfig->sync();

  //  kill(sirc_pid, 15);  // on normal exit, do try and kill dsirc...
}















