//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// This file os part of KRN, a newsreader for the KDE project.              //
// KRN is distributed under the GNU General Public License.                 //
// Read the acompanying file COPYING for more info.                         //
//                                                                          //
// KRN wouldn't be possible without these libraries, whose authors have     //
// made free to use on non-commercial software:                             //
//                                                                          //
// MIME++ by Doug Sauder                                                    //
// Qt     by Troll Tech                                                     //
//                                                                          //
// This file is copyright 1997 by                                           //
// Roberto Alsina <ralsina@unl.edu.ar>                                      //
// Magnus Reftel  <d96reftl@dtek.chalmers.se>                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include <qapp.h>
#include <qfile.h>

#include <kapp.h>
#include <Kconfig.h>
#include <kkeyconf.h>

#include "asker.h"
#include "groupdlg.h" 
#include "NNTP.h"
#include "kdecode.h"
#include "kmsender.h"
#include <mimelib/mimepp.h>

#include <gdbm.h>

// robert's cache stuff
#include <dirent.h>
#include <time.h>
#include <stdlib.h>

#define SECOND 1
#define MINUTE SECOND*60
#define HOUR   MINUTE*60
#define DAY    HOUR*24

Groupdlg  *main_widget;
KConfig *conf;
KLocale *nls;
KMSender *msgSender;

ArticleDict artSpool;

QString krnpath,cachepath,artinfopath,groupinfopath,pixpath,dbasepath,outpath;

KDecode *decoder;

GDBM_FILE artdb;

void checkConf();
void expireCache();

int main( int argc, char **argv )
{
    msgSender=0;
    // Initialize the mime++ library
    DwInitialize();

    KApplication a( argc, argv, "krn" );

    conf=a.getConfig();
    nls=a.getLocale();

    checkConf();

    QString pixdef=a.kdedir() + QString("/lib/pics/krn/");
    conf->setGroup("KDE Setup");
    if(conf->hasKey("pixpath")) pixpath=conf->readEntry("pixpath",pixdef);
    else pixpath=pixdef;

    decoder=new KDecode;
    
    // Create our directory. If it exists, no problem
    // Should do some checking, though
    
    QString c=getenv ("HOME");
    c=c+"/.kde";
    mkdir (c.data(),S_IREAD|S_IWRITE|S_IEXEC);
    krnpath=c+"/krn/";
    mkdir (krnpath.data(),S_IREAD|S_IWRITE|S_IEXEC);
    cachepath=krnpath+"/cache/";
    mkdir (cachepath.data(),S_IREAD|S_IWRITE|S_IEXEC);
    groupinfopath=krnpath+"/groupinfo/";
    mkdir (groupinfopath.data(),S_IREAD|S_IWRITE|S_IEXEC);
    outpath=krnpath+"/outgoing/";
    mkdir (outpath.data(),S_IREAD|S_IWRITE|S_IEXEC);

    // Create the articles database

    artinfopath=krnpath+"/artinfo.db";
    artdb=gdbm_open(artinfopath.data(),0,GDBM_WRCREAT,448,0);
    
    Groupdlg k;
    main_widget = &k;
    
    a.setMainWidget( (QWidget *) &k );
    
    k.setMinimumSize( 250, 250 );
    k.show();
    
    a.exec();
    expireCache();

    gdbm_close(artdb);
}

void checkConf()
// This checks that all necessary data exists and/or asks for it
// Should use Stefan's KIdentity someday
{
    Asker ask;
    QString data;
    conf->setGroup("Identity");
    
    data=conf->readEntry("Address");
    if (data.isEmpty())
    {
        ask.setCaption ("KRN-Missing Configuration info");
        ask.label->setText("Please enter your email adress");
        ask.entry->setText("");
        ask.exec();
        data=ask.entry->text();
        conf->writeEntry("Address",data);
    }


    data=conf->readEntry("RealName");
    if (data.isEmpty())
    {
        ask.setCaption ("KRN-Missing Configuration info");
        ask.label->setText("Please enter your real name");
        ask.entry->setText("");
        ask.exec();
        data=ask.entry->text();
        conf->writeEntry("RealName",data);
    }


    data=conf->readEntry("Organization");
    if (data.isEmpty())
    {
        ask.setCaption ("KRN-Missing Configuration info");
        ask.label->setText("Please enter your organization's name");
        ask.entry->setText("");
        ask.exec();
        data=ask.entry->text();
        conf->writeEntry("Organization",data);
    }

    conf->setGroup("NNTP");
    data=conf->readEntry("NNTPServer");
    if (data.isEmpty())
    {
        ask.setCaption ("KRN-Missing Configuration info");
        ask.label->setText("Please enter your NNTP server name");
        ask.entry->setText(getenv("NNTPSERVER"));
        ask.exec();
        data=ask.entry->text();
        conf->writeEntry("NNTPServer",data);
    }
    conf->sync();
}

void expireCache()   // robert's cache stuff
{

    conf->setGroup("Cache");
    int expireTime=conf->readNumEntry("ExpireDays",5);
    struct dirent **cdir;
    struct stat st;
    int num_files;
    char filename[255];
    time_t currenttime = time(NULL);
    
    num_files = scandir(cachepath.data(), &cdir, 0, alphasort);
    
    // debug("%d: %s", num_files, cachepath.data());
    
    for(int count = 0; count < num_files; count++) {
        sprintf(filename, "%s%s", cachepath.data(), cdir[count]->d_name);
        
        //    debug(filename);
        
        if(stat(filename, &st)) {
            debug("couldn't stat %s", filename);
        } else {
            if(((currenttime-st.st_atime) > DAY*expireTime) && cdir[count]->d_name[0] != '.') {
                Article *art = new Article();
                
                QString id = cdir[count]->d_name;
                
                art->ID = id;
                
                art->load();
                
                if(art->canExpire())
                    //    debug("unlinking %s", filename);
                    unlink(filename);
                
                delete art;
            }
        }
  }
}


