/*
 * kbiff.cpp
 * Copyright (C) 1998 Kurt Granroth <granroth@kde.org>
 *
 * This file contains the implementation of the main KBiff
 * widget
 *
 * $Id$
 */
#include "kbiff.h"
#include "kbiff.moc"

#include "Trace.h"

#include <kiconloader.h>
#include <kprocess.h>
#include <kwm.h>

#include "setupdlg.h"

KBiff::KBiff(QWidget *parent)
	: QLabel(parent)
{
TRACEINIT("KBiff::KBiff()");
	setAutoResize(true);
	setMargin(0);
	setAlignment(AlignLeft | AlignTop);

	// Init the audio server.  serverStatus return 0 when it is ok
	hasAudio = (audioServer.serverStatus() == 0) ? true : false;

	reset();
}

KBiff::~KBiff()
{
}

void KBiff::processSetup(const KBiffSetup* setup)
{
TRACEINIT("KBiff::processSetup()");
	// General settings
	mailClient  = setup->getMailClient();
	docked      = setup->getDock();
	sessions    = setup->getSessionManagement();
	noMailIcon  = setup->getNoMailIcon();
	newMailIcon = setup->getNewMailIcon();
	oldMailIcon = setup->getOldMailIcon();

	// New mail
	systemBeep     = setup->getSystemBeep();
	runCommand     = setup->getRunCommand();
	runCommandPath = setup->getRunCommandPath();
	playSound      = setup->getPlaySound();
	playSoundPath  = setup->getPlaySoundPath();
	notify         = setup->getNotify();

	setMailboxList(setup->getMailboxList(), setup->getPoll());

	delete setup;
}

void KBiff::setMailboxList(const QList<KURL>& mailbox_list, unsigned int poll)
{
TRACEINIT("KBiff::setMailboxList");
	QList<KURL> tmp_list = mailbox_list;

	monitorList.clear();
	
	myMUTEX = true;
	KURL *url;
	for (url = tmp_list.first(); url != 0; url = tmp_list.next())
	{
		TRACEF("Now adding %s", url->url().data());
		KBiffMonitor *monitor = new KBiffMonitor();
		monitor->setMailbox(*url);
		monitor->setPollInterval(poll);
		connect(monitor, SIGNAL(signal_newMail()), this, SLOT(haveNewMail()));
		connect(monitor, SIGNAL(signal_noMail()), this, SLOT(displayPixmap()));
		connect(monitor, SIGNAL(signal_oldMail()), this, SLOT(displayPixmap()));
		monitorList.append(monitor);
	}
	myMUTEX = false;
}

inline const bool KBiff::isDocked() const
{
	return docked;
}

void KBiff::readSessionConfig()
{
TRACEINIT("KBiff::readSesionConfig()");
	KConfig *config = kapp->getSessionConfig();

	config->setGroup("KBiff");

	processSetup(new KBiffSetup(config->readEntry("Profile")));
	docked = config->readBoolEntry("IsDocked");
}

///////////////////////////////////////////////////////////////////////////
// Protected Virtuals
///////////////////////////////////////////////////////////////////////////
void KBiff::mousePressEvent(QMouseEvent *event)
{
TRACEINIT("KBiff::mousePressEvent()");
	// check if this is a right click
	if(event->button() == RightButton)
	{
		// popup the context menu
		popupMenu();
	}
	else
	{
		// force a "read"
		KBiffMonitor *monitor;
		for (unsigned int i = 0; i < monitorList.count(); i++)
		{
			monitor = monitorList.at(i);
			monitor->setMailboxIsRead();
		}

		// execute the command
		if (!mailClient.isEmpty())
		{
			KProcess client;
			client << mailClient;
			client.start(KProcess::DontCare);
		}
	}
}

///////////////////////////////////////////////////////////////////////////
// Protected Slots
///////////////////////////////////////////////////////////////////////////
void KBiff::invokeHelp()
{
	kapp->invokeHTMLHelp("kbiff/kbiff.html", "");
}

void KBiff::displayPixmap()
{
TRACEINIT("KBiff::displayPixmap()");
	if (myMUTEX)
		return;

	// we will try to deduce the pixmap (or gif) name now.  it will
	// vary depending on the dock and mail state
	QString pixmap_name, mini_pixmap_name;
	bool has_new = false, has_old = false, has_no = true;
	KBiffMonitor *monitor;
	for (monitor = monitorList.first();
	     monitor != 0 && has_new == false;
		  monitor = monitorList.next())
	{
		TRACE("Checking mailbox");
		switch (monitor->getMailState())
		{
			case NoMail:
				has_no = true;
				break;
			case NewMail:
				has_new = true;
				break;
			case OldMail:
				has_old = true;
				break;
			default:
				has_no = true;
				break;
		}
	}
	TRACE("Done checking mailboxes");

	if (has_new)
		pixmap_name = newMailIcon;
	else if (has_old)
		pixmap_name = oldMailIcon;
	else
		pixmap_name = noMailIcon;
	mini_pixmap_name = "mini-" + pixmap_name;

	// Get a list of all the pixmap paths.  This is needed since the
	// icon loader only returns the name of the pixmap -- NOT it's path!
	QStrList *dir_list = kapp->getIconLoader()->getDirList();
	QFileInfo file, mini_file;
	for (unsigned int i = 0; i < dir_list->count(); i++)
	{
		QString here = dir_list->at(i);

		// check if this file exists.  If so, we have our path
		file.setFile(here + '/' + pixmap_name);
		mini_file.setFile(here + '/' + mini_pixmap_name);

		// if we are docked, check if the mini pixmap exists FIRST.
		// if it does, we go with it.  if not, we look for the
		// the regular size one
		if (docked && mini_file.exists())
		{
			file = mini_file;
			break;
		}

		if (file.exists())
			break;
	}

	TRACEF("Displaying %s", file.absFilePath().data());
	// at this point, we have the file to display.  so display it
	QPixmap pixmap(file.absFilePath());
	setPixmap(pixmap);
	adjustSize();
}

void KBiff::haveNewMail()
{
TRACEINIT("KBiff::haveNewMail()");
	displayPixmap();
	// notify if we must
	if (notify)
	{
		// do notify
	}

	// beep if we are allowed to
	if (systemBeep)
	{
		kapp->beep();
	}

	// run a command if we have to
	if (runCommand)
	{
		// make sure the command exists
		if (!runCommandPath.isEmpty())
		{
			KProcess command;
			command << runCommandPath;
			command.start(KProcess::DontCare);
		}
	}

	// play a sound if we have to
	if (playSound && hasAudio)
	{
		// make sure something is specified
		if (!playSoundPath.isEmpty())
		{
			audioServer.play(playSoundPath);
			audioServer.sync();
		}
	}
}

void KBiff::dock()
{
TRACEINIT("KBiff::dock()");
	// destroy the old window
	if (this->isVisible())
	{
		this->hide();
		this->destroy(true, true);
		this->create(0, true, false);
		kapp->setMainWidget(this);

		// we don't want a "real" top widget if we are _going_ to
		// be docked.
		if (docked)
			kapp->setTopWidget(this);
		else
			kapp->setTopWidget(new QWidget);
	}

	if (docked == false)
	{
		docked = true;

		// enable docking
		KWM::setDockWindow(this->winId());
	}
	else
		docked = false;

	// (un)dock it!
	this->show();
	displayPixmap();
}

void KBiff::setup()
{
TRACEINIT("KBiff::setup()");
	KBiffSetup* setup_dlg = new KBiffSetup;

	if (setup_dlg->exec())
		processSetup(setup_dlg);
}

void KBiff::checkMailNow()
{
TRACEINIT("KBiff::checkMailNow()");
	KBiffMonitor *monitor;
	for (monitor = monitorList.first();
	     monitor != 0;
		  monitor = monitorList.next())
	{
		monitor->checkMailNow();
	}
}

void KBiff::stop()
{
TRACEINIT("KBiff::stop()");
	KBiffMonitor *monitor;
	for (monitor = monitorList.first();
	     monitor != 0;
		  monitor = monitorList.next())
	{
		monitor->stop();
	}
}

void KBiff::start()
{
TRACEINIT("KBiff::start()");
	myMUTEX = true;
	KBiffMonitor *monitor;
	for (unsigned int i = 0; i < monitorList.count(); i++)
	{
		TRACE("Starting a monitor");
		monitor = monitorList.at(i);
		monitor->start();
	}
	myMUTEX = false;

	displayPixmap();
}

void KBiff::show()
{
	if (!isRunning())
		start();
	QLabel::show();
}

///////////////////////////////////////////////////////////////////////////
// Protected Functions
///////////////////////////////////////////////////////////////////////////
void KBiff::popupMenu()
{
TRACEINIT("KBiff::popupMenu()");
	QPopupMenu *popup = new QPopupMenu(0, "popup");

	if (docked)
		popup->insertItem(i18n("&UnDock"), this, SLOT(dock()));
	else
		popup->insertItem(i18n("&Dock"), this, SLOT(dock()));
	popup->insertItem(i18n("&Setup..."), this, SLOT(setup()));
	popup->insertSeparator();
	popup->insertItem(i18n("&Help..."), this, SLOT(invokeHelp()));
	popup->insertSeparator();

	int check_id;
	check_id = popup->insertItem(i18n("&Check mail now"), this, SLOT(checkMailNow()));

	if (isRunning())
	{
		popup->setItemEnabled(check_id, true);
		popup->insertItem(i18n("&Stop"), this, SLOT(stop()));
	}
	else
	{
		popup->setItemEnabled(check_id, false);
		popup->insertItem(i18n("&Start"), this, SLOT(start()));
	}

	popup->insertSeparator();
	popup->insertItem(i18n("E&xit"), kapp, SLOT(quit()));

	popup->popup(QCursor::pos());
}

void KBiff::reset()
{
TRACEINIT("KBiff::reset()");
	// reset all the member variables
	systemBeep     = true;
	runCommand     = false;
	runCommandPath = "";
	playSound      = false;
	playSoundPath  = "";
	notify         = false;

	noMailIcon  = "nomail.xpm";
	newMailIcon = "newmail.xpm";
	oldMailIcon = "oldmail.xpm";

	docked    = false;

	mailClient  = "xmutt";

	myMUTEX = false;

	displayPixmap();
}

bool KBiff::isRunning()
{
TRACEINIT("KBiff::isRunning()");
	bool is_running = false;
	KBiffMonitor *monitor;
	for (monitor = monitorList.first();
	     monitor != 0;
		  monitor = monitorList.next())
	{
		if (monitor->isRunning())
		{
			is_running = true;
			break;
		}
	}
	return is_running;
}
