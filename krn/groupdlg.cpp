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

#include <unistd.h>

#include "groupdlg.h"

#define Inherited KTopLevelWidget

#include <qapp.h>
#include <qfile.h>
#include <qstring.h>
#include <qtstream.h>
#include <qevent.h>

#include <kapp.h>
#include <kmsgbox.h>
#include <kconfig.h>
#include <kkeyconf.h>

#include "artdlg.h"
#include "identDlg.h"
#include "NNTPConfigDlg.h"
#include "rmbpop.h"
#include "aboutDlg.h"
#include "asker.h"
#include "groupdlg.moc"

#include "krnsender.h"
#include "kmcomposewin.h"
#include "kmreaderwin.h"
#include "kmidentity.h"

#define CONNECT 1
#define DISCONNECT 5
#define GET_ACTIVE 3
#define SUBSCRIBE 4
#define LOAD_FILES 2
#define CHECK_UNREAD 6
#define CHANGE_IDENTITY 8
#define CONFIG_NNTP 9
#define EXIT 7
#define OPENGROUP 10
#define TAGGROUP 11
#define HELP_CONTENTS 12
#define HELP_ABOUT 13
#define GET_SUBJECTS 14
#define GET_ARTICLES 15
#define CATCHUP 16
#define FIND_GROUP 17
#define POST 18
#define POST_QUEUED 19

extern QString krnpath,cachepath,artinfopath,pixpath;
extern KConfig *conf;

extern KMIdentity *identity;
extern KRNSender *msgSender;

GroupList groups;
GroupList subscr;
GroupList tagged;

bool checkPixmap(KTreeListItem *item,void *)
{
    QPixmap p;
    QString name(item->getText());
    if (name.right(1)==".") //it's a folder
    {
        p=kapp->getIconLoader()->loadIcon("krnfolder.xpm");
        item->setPixmap(&p);
        return false;
    }
    NewsGroup n(name);
    if (tagged.find(&n)!=-1) //it's tagged
    {
        p=kapp->getIconLoader()->loadIcon("tagged.xpm");
        item->setPixmap(&p);
        return false;
    }
    if (subscr.find(&n)!=-1) //it's subscribed
    {
        p=kapp->getIconLoader()->loadIcon("subscr.xpm");
        item->setPixmap(&p);
        return false;
    }
    p=kapp->getIconLoader()->loadIcon("followup.xpm");
    item->setPixmap(&p); //it's plain
    return false;
}

Groupdlg::Groupdlg
(
 const char *name
)
:
Inherited (name)
{
    groups.setAutoDelete(true);
    subscr.setAutoDelete(false);
    tagged.setAutoDelete(false);
    
    QPopupMenu *file = new QPopupMenu;
    file->insertItem(klocale->translate("Connect to Server"),CONNECT);
    file->insertItem(klocale->translate("Disconnect From Server"),DISCONNECT);
    file->insertItem(klocale->translate("Get Active File"),GET_ACTIVE);
    file->insertSeparator();
    file->insertItem(klocale->translate("Exit"),EXIT);
    connect (file,SIGNAL(activated(int)),SLOT(currentActions(int)));
    
    setCaption (klocale->translate("KRN - Group List"));
    
    QPopupMenu *subscribed = new QPopupMenu;
    subscribed->insertItem(klocale->translate("Get Subjects"),GET_SUBJECTS);
    subscribed->insertItem(klocale->translate("Get Articles"),GET_ARTICLES);
    subscribed->insertItem(klocale->translate("Catchup"),CATCHUP);
    connect (subscribed,SIGNAL(activated(int)),SLOT(subscrActions(int)));
    
    QPopupMenu *tagged = new QPopupMenu;
    tagged->insertItem(klocale->translate("Get Subjects"),GET_SUBJECTS);
    tagged->insertItem(klocale->translate("Get Articles"),GET_ARTICLES);
    tagged->insertItem(klocale->translate("(un)Subscribe"),SUBSCRIBE);
    tagged->insertItem(klocale->translate("Untag"),TAGGROUP);
    tagged->insertItem(klocale->translate("Catchup"),CATCHUP);
    connect (tagged,SIGNAL(activated(int)),SLOT(taggedActions(int)));
    
    QPopupMenu *newsgroup = new QPopupMenu;
    newsgroup->insertItem(klocale->translate("Open"),OPENGROUP);
    newsgroup->insertItem(klocale->translate("Find"),FIND_GROUP);
    newsgroup->insertItem(klocale->translate("(un)Subscribe"),SUBSCRIBE);
    newsgroup->insertItem(klocale->translate("(Un)Tag"),TAGGROUP);
    newsgroup->insertItem(klocale->translate("Catchup"),CATCHUP);
    newsgroup->insertSeparator();
    newsgroup->insertItem(klocale->translate("Post New Article"),POST);
    newsgroup->insertItem(klocale->translate("Post All Queued Articles"),POST_QUEUED);
    newsgroup->insertSeparator();
    newsgroup->insertItem (klocale->translate("&Subscribed"), subscribed);
    newsgroup->insertItem (klocale->translate("&Tagged"), tagged);
    
    connect (newsgroup,SIGNAL(activated(int)),SLOT(currentActions(int)));
    
    QPopupMenu *options = new QPopupMenu;
    options->insertItem(klocale->translate("Identity..."),CHANGE_IDENTITY);
    options->insertItem(klocale->translate("NNTP Options..."),CONFIG_NNTP);
    connect (options,SIGNAL(activated(int)),SLOT(currentActions(int)));
    
    QPopupMenu *help = new QPopupMenu;
    help->insertItem(klocale->translate("Contents"),HELP_CONTENTS);
    help->insertSeparator();
    help->insertItem(klocale->translate("About Krn..."),HELP_ABOUT);
    connect (help,SIGNAL(activated(int)),SLOT(currentActions(int)));
    
    KMenuBar *menu = new KMenuBar (this, "menu");
    
    menu->insertItem (klocale->translate("&File"), file);
    menu->insertItem (klocale->translate("&Newsgroup"), newsgroup);
    menu->insertItem (klocale->translate("&Options"), options);
    menu->insertSeparator();
    menu->insertItem (klocale->translate("&Help"), help);
    
    setMenu (menu);
    
    QPixmap pixmap;
    
    KToolBar *tool = new KToolBar (this, "tool");
    QObject::connect (tool, SIGNAL (clicked (int)), this, SLOT (currentActions (int)));
    
    pixmap=kapp->getIconLoader()->loadIcon("connected.xpm");
    tool->insertButton (pixmap, CONNECT, true, klocale->translate("Connect to server"));
    
    pixmap=kapp->getIconLoader()->loadIcon("disconnected.xpm");
    tool->insertButton (pixmap, DISCONNECT, false, klocale->translate("Disconnect from server"));
    tool->insertSeparator ();
    
    pixmap=kapp->getIconLoader()->loadIcon("previous.xpm");
    tool->insertButton (pixmap, GET_ACTIVE, true, klocale->translate("Get list of active groups"));
    pixmap=kapp->getIconLoader()->loadIcon("find.xpm");
    tool->insertButton (pixmap, FIND_GROUP, true, klocale->translate("Find group"));
    tool->insertSeparator ();
    
    tool->insertButton (kapp->getIconLoader()->loadIcon("subscr.xpm"), SUBSCRIBE, true, "(Un)Subscribe");
    addToolBar (tool);
    tool->insertSeparator ();
    
    pixmap=kapp->getIconLoader()->loadIcon("reload.xpm");
    tool->insertButton (pixmap, CHECK_UNREAD, true, klocale->translate("Check for Unread Articles"));
    
    pixmap=kapp->getIconLoader()->loadIcon("filenew.xpm");
    tool->insertButton (pixmap, POST, true, klocale->translate("Post New Article"));
    
    list = new MyTreeList (this, "");
    QObject::connect (list, SIGNAL (selected (int)), this, SLOT (openGroup (int)));
    setView (list);
    RmbPop *filter=new RmbPop(list);
    delete (filter->pop);
    filter->pop=newsgroup;
    
    
    KStatusBar *status = new KStatusBar (this, "status");
    status->insertItem ("                               ", 1);
    status->insertItem ("", 2);
    status->show ();
    setStatusBar (status);
    
    conf->setGroup("NNTP");
    QString sname=conf->readEntry("NNTPServer");
    server = new NNTP (sname.data());
    server->reportCounters (true,false);
    
    msgSender->setNNTP(server);
    
    connect (server,SIGNAL(newStatus(const char *)),this,SLOT(updateCounter(const char *)));
    conf->setGroup("Geometry");
    setGeometry(conf->readNumEntry("GroupX",100),
                conf->readNumEntry("GroupY",40),
                conf->readNumEntry("GroupW",400),
                conf->readNumEntry("GroupH",400));
    show();
    actions (LOAD_FILES);
    fillTree();
    
    conf->setGroup("NNTP");
    if (conf->readNumEntry("ConnectAtStart"))
        actions(CONNECT);
    
    // readProperties(); Kalle: no longer needed
    
    qApp->processEvents();
    //Open group windows
}

Groupdlg::~Groupdlg ()
{
    //    saveProperties(false); // Kalle: No longer needed
    QStrList openwin;
    //check for all open groups, and close them
    for (NewsGroup *g=groups.first();g!=0;g=groups.next())
    {
        if (g->isVisible)
        {
            openwin.append (g->name);
            delete g->isVisible;
        }
    }
    conf->setGroup("ArticleListOptions");
    conf->writeEntry("OpenWindows",openwin);
    conf->setGroup("Geometry");
    conf->writeEntry("GroupX",x());
    conf->writeEntry("GroupY",y());
    conf->writeEntry("GroupW",width());
    conf->writeEntry("GroupH",height());
}

void Groupdlg::openGroup (QString name)
{
    if (name.find('/')==0)
        name=name.right(name.length()-1);
    NewsGroup n(name);
    int i=groups.find(&n);
    if (groups.at(i)->isVisible)
    {
        return;
    }
    if (i!=-1)
    {
        groups.at(i)->load();
        Artdlg *a = new Artdlg (groups.at(i),server);
        QObject::connect(a->messwin,SIGNAL(spawnGroup(QString)),this,SLOT(openGroup(QString)));
        QObject::connect(a,SIGNAL(needConnection()),this,SLOT(needsConnect()));
    }
    else
    {
        KMsgBox:: message (0, klocale->translate("Sorry!"),
                           klocale->translate("That newsgroup is not in the current active file\n Krn can't handle this gracefully right now"),
                           KMsgBox::INFORMATION);
    }
}



void Groupdlg::openGroup (int index)
{
    QPixmap p;
    qApp->setOverrideCursor(waitCursor);
    QString base;
    KTreeListItem *it=list->itemAt(index);
    if (it->getText()[strlen(it->getText())-1]!='.')
    {
        QString temp=it->getText();
        int i=temp.find(' ');
        if (i>0)
        {
            temp=temp.left(i);;
        };
        openGroup(temp);
    }
    else
    {
        if (!it->hasChild())
        {
            if (!strcmp(klocale->translate("All Newsgroups."),it->getText()))
            {
                base="";
            }
            else
            {
                base=it->getText();
            }
            int l=base.length();
            int c=base.contains('.');
            char tc;
            QStrList bases;
            bases.setAutoDelete(true);
            QListIterator <NewsGroup> it(groups);
            NewsGroup *iter;
            for (;it.current();++it)
            {
                iter=it.current();
                //this group's name matches the base
                if (!strncmp(base.data(),iter->name,l))
                {
                    QString gname=iter->name;
                    //Check the dot count.
                    //If it's the same, then it's a group
                    //Add it as a child
                    if (gname.contains('.')==c)
                    {
                        p=kapp->getIconLoader()->loadIcon("followup.xpm");
                        list->addChildItem(iter->name,&p,index);
                    }
                    
                    else  //It may be a new hierarchy
                    {
                        //take what the new base would be
                        char *nextdot=strchr(iter->name+l+1,'.')+1;
                        tc=nextdot[0];
                        nextdot[0]=0;
                        if (bases.find(iter->name)==-1)
                        {
                            // It's new, so add it to the base list
                            // and insert it as a folder
                            bases.append(iter->name);
                            p=kapp->getIconLoader()->loadIcon("krnfolder.xpm");
                            list->addChildItem(iter->name,&p,index);
                        }
                        nextdot[0]=tc;
                    }
                }
            }
            list->expandItem(index);
            list->forEveryVisibleItem(checkPixmap,NULL);
            list->repaint();
        }
        else
        {
            if (it->isExpanded())
                list->collapseItem(index);
            else
                list->expandItem(index);
        }
    }
    qApp->restoreOverrideCursor();
}




void Groupdlg::subscribe (NewsGroup *group)
{
    QPixmap p;
    KPath path;
    int index=subscr.find (group);
    if (-1 != index)
    {
        subscr.remove ();
        path.push (new QString (klocale->translate("Subscribed Newsgroups.")));
        path.push (new QString (group->name));
        int l=list->currentItem();
        list->setCurrentItem(0);
        list->removeItem (&path);
        if (list->itemAt(0)->isExpanded() &&
            ((unsigned int)l>list->itemAt(0)->childCount()+1))
            list->setCurrentItem(l-1);
        else
            list->setCurrentItem(l);
    }
    else
    {
        if (-1 != groups.find (group))
        {
            p=kapp->getIconLoader()->loadIcon("subscr.xpm");
            list->addChildItem (group->name, &p, 0);
            subscr.append (group);
            if (list->itemAt(0)->isExpanded() &&
                ((unsigned int)list->currentItem()>list->itemAt(0)->childCount()+1))
                list->setCurrentItem(list->currentItem()+1);
        }
    };
    list->forEveryVisibleItem(checkPixmap,NULL);
    list->repaint();
    saveSubscribed();
}

void Groupdlg::tag (NewsGroup *group)
{
    if(tagged.find(group)>-1)
    {
        tagged.remove(group);
    }
    else
    {
        tagged.append(group);
    }
    list->forEveryVisibleItem(checkPixmap,NULL);
    list->repaint();
}


void Groupdlg::offline()
{
    msgSender=0;
    toolBar()->setItemEnabled (DISCONNECT,false);
    server->disconnect();
    toolBar()->setItemEnabled (CONNECT,true);
    statusBar ()->changeItem (klocale->translate("Disconnected"), 2);
    qApp->processEvents ();
}

void Groupdlg::online()
{
    toolBar()->setItemEnabled (CONNECT,false);
    statusBar ()->changeItem (klocale->translate("Connecting to server"), 2);
    qApp->processEvents ();
    if (server->connect ())
    {
        if (server->isReadOnly ())
            statusBar ()->changeItem (klocale->translate("Connected to server - Posting not allowed"), 2);
        else
            statusBar ()->changeItem (klocale->translate("Connected to server - Posting allowed"), 2);
    }
    else
    {
        qApp->setOverrideCursor (arrowCursor);
        KMsgBox:: message (0, klocale->translate("Error"),klocale->translate( "Can't connect to server"), KMsgBox::INFORMATION);
        qApp->restoreOverrideCursor ();
        statusBar ()->changeItem (klocale->translate("Connection to server failed"), 2);
        qApp->processEvents ();
        toolBar()->setItemEnabled (CONNECT,true);
    }
    if (conf->readNumEntry("Authenticate")!=0)
    {
        if (299<server->authinfo(conf->readEntry("Username"),conf->readEntry("Password")))
        {
            qApp->setOverrideCursor (arrowCursor);
            KMsgBox:: message (0, klocale->translate("Error"), klocale->translate("Authentication Failed"), KMsgBox::INFORMATION);
            qApp->restoreOverrideCursor ();
            actions(DISCONNECT);
            statusBar ()->changeItem (klocale->translate("Connection to server failed: Authentication problem"), 2);
            qApp->processEvents ();
            toolBar()->setItemEnabled (CONNECT,true);
        }
    }
    toolBar()->setItemEnabled (DISCONNECT,true);
}

void Groupdlg::fillTree ()
{
    QPixmap p;
    p=kapp->getIconLoader()->loadIcon("krnfolder.xpm");
    list->insertItem (klocale->translate("Subscribed Newsgroups."), &p);
    QListIterator <NewsGroup> it(subscr);
    it.toFirst();
    NewsGroup *g;
    for (;it.current();++it)
    {
        g=it.current();
        p=kapp->getIconLoader()->loadIcon("subscr.xpm");
        list->addChildItem (g->name, &p, 0);
    }

    p=kapp->getIconLoader()->loadIcon("krnfolder.xpm");
    list->insertItem (klocale->translate("All Newsgroups."), &p);
}

bool Groupdlg::needsConnect()
{
    bool success=false;
    qApp->setOverrideCursor (arrowCursor);
    if (server->isConnected())
    {
        success=true;
    }
    else
    {
        if (1==KMsgBox::yesNo(0,klocale->translate("Krn-Question"),
                              klocale->translate("The operation you requested needs a connection to the News server\nShould I attempt one?")))
        {
            actions(CONNECT);
            success=true;
        }
    }
    qApp->restoreOverrideCursor ();
    return success;
}


bool Groupdlg::actions (int action,NewsGroup *group)
{
    bool success=false;
    qApp->setOverrideCursor (waitCursor);
    switch (action)
    {
    case GET_SUBJECTS:
        {
            if (!group)
                break;
            getSubjects(group);
            break;
        }
    case GET_ARTICLES:
        {
            if (!group)
                break;
            getArticles(group);
            break;
        }
    case OPENGROUP:
        {
            openGroup (group->name);
            break;
        }
    case TAGGROUP:
        {
            if (!group)
                break;
            tag(group);
            break;
        }
    case CHANGE_IDENTITY:
        {
            qApp->setOverrideCursor (arrowCursor);
            IdentDlg id;
            if (id.exec())
            {
                identity->setEmailAddr(id.address->text());
                identity->setFullName(id.realname->text());
                identity->setOrganization(id.organization->text());
                identity->writeConfig();
            }
            qApp->restoreOverrideCursor ();
            success = true;
            break;
        }
        
    case CONFIG_NNTP:
        {
            qApp->setOverrideCursor (arrowCursor);
            NNTPConfigDlg dlg;
            if (dlg.exec())
            {
                conf->setGroup("NNTP");
                conf->writeEntry("NNTPServer",dlg.servername->text());
                conf->writeEntry("ConnectAtStart",dlg.connectatstart->isChecked());
                conf->writeEntry("Authenticate",dlg.authenticate->isChecked());
                conf->writeEntry("Username",dlg.username->text());
                conf->writeEntry("Password",dlg.password->text());
                conf->setGroup("sending mail");
                conf->writeEntry("Smtp Host",dlg.smtpserver->text());
                conf->sync();
                msgSender->readConfig();
            }
            qApp->restoreOverrideCursor ();
            success = true;
            break;
        }
        
    case EXIT:
        {
            qApp->exit();
            success = true;
            break;
        }
    case CONNECT:
        {
            online();
            break;
        }
    case DISCONNECT:
        {
            offline();
            break;
        }
    case LOAD_FILES:
        {
            loadSubscribed();
            loadActive();
            break;
        }
    case GET_ACTIVE:
        {
            if (needsConnect())
            {
                statusBar ()->changeItem ("Getting active list from server", 2);
                qApp->processEvents ();
                
                server->resetCounters (true,true);
                server->reportCounters (true,false);
                server->groupList (&groups,true);
                server->resetCounters (true,true);
                server->reportCounters (false,false);
                loadActive();
            }
            else
            {
                success = false;
            };
            break;
        }
    case SUBSCRIBE:
        {
            if (!group)
                break;
            subscribe (group);
            break;
        }
    case CHECK_UNREAD:
        {
            if (needsConnect())
            {
                printf ("check_unread\n");
                checkUnread();
            }
            break;
        }
    case HELP_ABOUT:
        {
            qApp->setOverrideCursor (arrowCursor);
            aboutDlg ab;
            ab.exec();
            qApp->restoreOverrideCursor ();
            success = true;
            break;
        }
    case HELP_CONTENTS:
        {
            kapp->invokeHTMLHelp("","");
            break;
        }
    case CATCHUP:
        {
            if (!group)
                break;
            group->catchup();
            
            success = true;
            break;
        }
    case FIND_GROUP:
        {
            findGroup();
            success = true;
            break;
        }
    case POST:
        {
            if (!group)
                break;
            
            conf->setGroup("Composer");
            int mShowHeaders = conf->readNumEntry("headers", 0x60);
            mShowHeaders = mShowHeaders & 0xfb;
            conf->writeEntry("headers",mShowHeaders);
            
            KMMessage *m=new KMMessage();
            m->initHeader();
            m->setGroups(group->name);
            KMComposeWin *comp=new KMComposeWin(m);
            comp->show();
            success=true;
            break;
        }
    case POST_QUEUED:
        {
            success=msgSender->sendQueued();
            break;
        }
    };
    
    
    qApp->restoreOverrideCursor ();
    return success;
}

bool Groupdlg::loadSubscribed()
{
    QString ac;
    ac=krnpath+"subscribed";
    QFile f(ac.data ());
    subscr.clear();
    if (f.open (IO_ReadOnly))
    {
        QTextStream st(&f);
        while (1)
        {
            ac=st.readLine ();
            if (st.eof())
                break;
            subscr.append (new NewsGroup(ac.data()));
        };
        f.close ();
        return true;
    }
    else
    {
        warning("Can't open subscribed file for reading!");
        return false;
    }
}

bool Groupdlg::saveSubscribed()
{
    QString ac;
    ac=krnpath+"/subscribed";
    unlink(ac.data());
    QFile f(ac.data ());
    if (f.open (IO_WriteOnly))
    {
        QListIterator <NewsGroup>it(subscr);
        for (;it.current();++it)
        {
            f.writeBlock(it.current()->name,strlen(it.current()->name));
            f.writeBlock("\n",1);
        }
        f.close ();
        return true;
    }
    else
    {
        printf ("Can't save subscribed file\n");
        return false;
    }
}

bool Groupdlg::loadActive()
{
    bool success=false;
    QString ac=krnpath+"/active";
    QFile f (ac.data ());
    if (!f.exists())	//doesn't have an active file
    {
        
        qApp->setOverrideCursor (arrowCursor);
        int i = KMsgBox::yesNo (0, klocale->translate("Error"),
                                klocale->translate("You don't have an active groups list.\n get it from server?"));
        if (!needsConnect())
            return false;
        qApp->restoreOverrideCursor ();
        if (1 == i)
        {
            server->groupList(&groups,true);
            success=true;
        }
        else
        {
            return false;
        }
    }
    else			// active file not exists
    {
        statusBar ()->changeItem (klocale->translate("Listing active newsgroups"), 2);
        qApp->processEvents ();
        server->groupList (&groups,false);
        success=true;
    };
    statusBar ()->changeItem ("", 2);
    return success;
};

void Groupdlg::findGroup()
{
    Asker ask;
    ask.setCaption (klocale->translate("KRN - Find a Newsgroup"));
    ask.label->setText(klocale->translate("Enter the name of the Newsgroup"));
    ask.entry->setText("");
    qApp->setOverrideCursor (arrowCursor);
    ask.exec();
    qApp->restoreOverrideCursor ();
    if (QString (ask.entry->text()).isEmpty())
        return;
    int index=-1;
    NewsGroup n(ask.entry->text());
    index=subscr.find (&n);
    if (index!=-1)
    {
        //It exists in subscribed
        list->expandItem(0);
        list->setCurrentItem(index+1);
        return;
    }
    index=groups.find (&n);
    if (index!=-1)
    {
        list->setAutoUpdate(false);
        //It exists and not in subscribed (ugh).
        
        KPath p;
        
        p.push (new QString(klocale->translate("All Newsgroups.")));
        
        if (!list->itemAt(&p)->isExpanded())
            openGroup(list->itemIndex(list->itemAt(&p)));
        
        char *s=qstrdup(ask.entry->text());
        char *s2;
        
        while (1)
        {
            if (!strchr(s,'.'))
            {
                QString *ss;
                QString *s1=p.pop();
                ss=new QString(qstrdup(s1->data()));
                p.push(s1);
                ss->append(s);
                p.push (ss);
                int index=list->itemIndex(list->itemAt(&p));
                list->setCurrentItem(index);
                list->setTopCell(index);
                break;
            }
            else
            {
                s2=strchr(s,'.')+1;
                *strchr(s,'.')=0;
                QString *ss;
                if (p.count()>1)
                {
                    QString *s1=p.pop();
                    ss=new QString(qstrdup(s1->data()));
                    p.push(s1);
                }
                else
                {
                    ss=new QString("");
                }
                ss->append(s);
                ss->append(".");
                p.push(ss);
                KTreeListItem *it=list->itemAt(&p);
                if (!it)
                {
                    debug ("no fscking item!!!!");
                    break;
                }
                if (!it->isExpanded())
                {
                    int in=list->itemIndex(it);
                    openGroup(in);
                }
                s=s2;
            }
        }
        list->repaint();
        list->setAutoUpdate(true);
        return;
    }
    qApp->setOverrideCursor (arrowCursor);
    KMsgBox::message(0,klocale->translate("Krn - Message"),
                     klocale->translate("I can't find the group you want!"));
    qApp->restoreOverrideCursor ();
}

bool Groupdlg::currentActions(int action)
{
    bool success=true;
    //Lets try to find the newsgroup
    if (action==OPENGROUP)
    {
        openGroup(list->currentItem());
        return success;
    }
    else
    {
        int index=-1;
        NewsGroup *g=0;
        if (list->getCurrentItem())
        {
            const char *text = list->getCurrentItem ()->getText ();
            NewsGroup n(text);
            index=groups.find (&n);
        }
        if (index!=-1)
            g=groups.at(index);
        actions(action,g);
    }
    return success;
}

bool Groupdlg::listActions(int action ,GroupList _list)
{
    bool success=true;
    QListIterator <NewsGroup> it(_list);
    for (;it.current();++it)
    {
        actions(action,it.current());
    }
    statusBar ()->changeItem ("Done.", 2);
    return success;
}


bool Groupdlg::taggedActions(int action)
{
    return listActions (action,tagged);
}

bool Groupdlg::subscrActions(int action)
{
    return listActions (action,subscr);
}

void Groupdlg::getArticles(NewsGroup *group)
{
    if (needsConnect())
    {
        QString s;
        s=klocale->translate("Getting messages in ");
        s+=group->name;
        statusBar ()->changeItem (s.data(), 2);
        qApp->processEvents();
        group->getMessages(server);
    }
}

void Groupdlg::getSubjects(NewsGroup *group)
{
    if (needsConnect())
    {
        QString s;
        s=klocale->translate("Getting list of messages in ");
        s+=group->name;
        statusBar ()->changeItem (s.data(), 2);
        qApp->processEvents();
        group->getSubjects(server);
    }
}

void Groupdlg::checkUnread()
{
    QString l,status,howmany,first,last,gname;
    KPath p;
    KTreeListItem *base=list->itemAt(0); //The "Subscribed Newsgroups" item.
    p.push(new QString(base->getText()));
    char countmessage[255];
    
    int c=base->childCount();
    for (int i=0;i<c;i++)
    {
        const char *it=base->childAt(i)->getText();
        gname=it;
        p.push(new QString(it));
        gname=gname.left(gname.find(' '));
        sprintf(countmessage, " [%d]", subscr.at(i)->countNew(server));
        
        l=gname+countmessage;
        // Updating the test this way doesn't repaint properly,
        // so I have to do do the path hack.
        //        base->childAt(i)->setText(l.data());
        list->changeItem(l.data(),0,&p);
        delete p.pop();
    }
    list->repaint();
}


void Groupdlg::updateCounter(const char *s)
{
    statusBar()->changeItem (s, 1);
    qApp->processEvents();
}

