/*
  main.cpp - The KControl module for ktalkd

  written 1998 by David Faure
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
   
  */


#include "soundpage.h"
#include "answmachpage.h"
#include <kcontrol.h>
#include <ksimpleconfig.h>

KSimpleConfig *config;

class KKTalkdApplication : public KControlApplication
{
public:

    KKTalkdApplication(int &argc, char **argv, const char *name);
    
    virtual void init();
    virtual void apply();
    virtual void defaultValues();
    virtual void help();
    
private:

    KSoundPageConfig *soundpage;
    KAnswmachPageConfig *answmachpage;
};

KKTalkdApplication::KKTalkdApplication(int &argc, char **argv, 
                                       const char *name)
    : KControlApplication(argc, argv, name), soundpage(0), answmachpage(0)
{
    if (runGUI())
	{
	    if (!pages || pages->contains("soundpage"))
		addPage(soundpage = new KSoundPageConfig(dialog, "soundpage"),
			i18n("&Announcement"), "");
	    if (!pages || pages->contains("answmachpage"))
		addPage(answmachpage = new KAnswmachPageConfig(dialog, "answmachpage"),
			i18n("Ans&wering machine"), "");
	    
	    if (soundpage || answmachpage) {
		dialog->show();
	    }
	    else {
		fprintf(stderr, i18n("usage: kcmktalkd [-init | soundpage | answmachpage]\n"));
		justInit = true;
	    }
	    
	}
}

void KKTalkdApplication::help()
{
   invokeHTMLHelp("ktalkd/index.html","");
}

void KKTalkdApplication::init()
{
}

void KKTalkdApplication::defaultValues() 
{
    if (soundpage)
        soundpage->defaultSettings();
    if (answmachpage)
        answmachpage->defaultSettings();    
}

void KKTalkdApplication::apply()
{
    if (soundpage)
        soundpage->saveSettings();
    if (answmachpage)
        answmachpage->saveSettings();
}

int main(int argc, char **argv)
{
    config = new KSimpleConfig(KApplication::localconfigdir() + "/ktalkdrc");
    KKTalkdApplication app(argc, argv, "kcmktalkd");
    app.setTitle(i18n("KTalkd Configuration"));

    int ret;
    if (app.runGUI())	
        ret = app.exec();
    else
        ret = 0;

    delete config;
    return ret;
}