// kmfilterdlg.cpp

#include <qfiledlg.h>

#include "kmfilterdlg.h"
#include "kmfilter.h"
#include "kmfiltermgr.h"
#include "kmglobal.h"
#include "kmfolder.h"
#include "kmfolderdir.h"
#include "kmfoldermgr.h"

#include <kapp.h>
#include <kmsgbox.h>
#include <ktablistbox.h>
#include <kbuttonbox.h>
#include <qbttngrp.h>
#include <qframe.h>
#include <qgrpbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbt.h>
#include <qradiobt.h>
#include <qcombo.h>
#include <qlined.h>
#include <assert.h>
#include <qstrlist.h>

static QStrList sFilterOpList, sFilterFuncList, sFilterFieldList, 
                sFilterActionList;

//=============================================================================
KMFaComboBox::KMFaComboBox(QWidget* p, const char* n):
  KMFaComboBoxInherited(false, p, n)
{
  initMetaObject();
  connect(this, SIGNAL(activated(int)), SLOT(slotSelected(int)));
}

void KMFaComboBox::slotSelected(int idx)
{
  emit selectType(this, idx);
}


//=============================================================================
KMFilterDlg::KMFilterDlg(QWidget* parent, const char* name):
  KMFilterDlgInherited(parent, name, FALSE)
{
  KButtonBox *buttonBox;
  QPushButton *btnHelp;
  QGridLayout *fgrid, *grid, *agrid;
  int h, w, i;
  QSize sz;
  updown_move_semaphore = 1;
  
  initMetaObject();

  grid  = new QGridLayout(this, 4, 2, 4, 4);
  mFilter = NULL;

  setCaption(i18n("Filter Rules"));
  //setOKButton(i18n("OK"));
  //setCancelButton(i18n("Cancel"));

  mFilterList = new QListBox(this);
  mFilterList->setMinimumSize(100, 200);
  grid->addMultiCellWidget(mFilterList, 0, 2, 0, 0);
  connect(mFilterList,SIGNAL(highlighted(int)),SLOT(slotFilterSelected(int)));

  initLists();

  //---------- static filter fields
  fgrid = new QGridLayout(3, 3, 4);
  grid->addLayout(fgrid, 0, 1);

  mRuleFuncA = new QComboBox(false, this);
  sz = mRuleFuncA->sizeHint();
  w = sz.width();
  h = sz.height();
  mCbxHeight = h;
  mRuleFuncA->setMinimumSize(40, h);
  mRuleFuncA->setMaximumSize(32767, h);
  mRuleFuncA->insertStrList(&sFilterFuncList);
  fgrid->addWidget(mRuleFuncA, 0, 1);

  mRuleFuncB = new QComboBox(false, this);
  mRuleFuncB->setMinimumSize(40, h);
  mRuleFuncB->setMaximumSize(32767, h);
  mRuleFuncB->insertStrList(&sFilterFuncList);
  fgrid->addWidget(mRuleFuncB, 2, 1);

  mRuleFieldA = new QComboBox(true, this);
  mRuleFieldA->insertStrList(&sFilterFieldList);
  mRuleFieldA->setMinimumSize(50, h);
  mRuleFieldA->setMaximumSize(32767, h);
  fgrid->addWidget(mRuleFieldA, 0, 0);

  mRuleFieldB = new QComboBox(true, this);
  mRuleFieldB->insertStrList(&sFilterFieldList);
  mRuleFieldB->setMinimumSize(50, h);
  mRuleFieldB->setMaximumSize(32767, h);
  fgrid->addWidget(mRuleFieldB, 2, 0);

  mRuleValueA = new QLineEdit(this);
  mRuleValueA->adjustSize();
  mRuleValueA->setMinimumSize(80, mRuleValueA->sizeHint().height());
  fgrid->addWidget(mRuleValueA, 0, 2);

  mRuleValueB = new QLineEdit(this);
  mRuleValueB->adjustSize();
  mRuleValueB->setMinimumSize(80, mRuleValueB->sizeHint().height());
  fgrid->addWidget(mRuleValueB, 2, 2);

  mRuleOp = new QComboBox(false, this);
  mRuleOp->insertStrList(&sFilterOpList);
  mRuleOp->setMinimumSize(50, h);
  mRuleOp->setMaximumSize(32767, h);
  fgrid->addMultiCellWidget(mRuleOp, 1, 1, 0, 1);

  //---------- filter action area
  agrid = new QGridLayout(FILTER_MAX_ACTIONS, 3, 4);
  grid->addLayout(agrid, 1, 1);
  for (i=0; i<FILTER_MAX_ACTIONS; i++)
  {
    mFaType[i] = new KMFaComboBox(this);
    mFaType[i]->insertStrList(&sFilterActionList);
    mFaType[i]->setMinimumSize(80, h);
    mFaType[i]->setMaximumSize(32767, h);
    agrid->addWidget(mFaType[i], i, 0);
    connect(mFaType[i], SIGNAL(selectType(KMFaComboBox*, int)),
	    SLOT(slotActionTypeSelected(KMFaComboBox*,int)));

    mFaBtnDetails[i] = NULL;
    mFaField[i] = NULL;
  }
  mActLineHeight = mFaType[0]->size().height();

  //---------- button area
  buttonBox = new KButtonBox(this, KButtonBox::HORIZONTAL, 0, 2);
  grid->addMultiCellWidget(buttonBox, 3, 3, 0, 1);

  mBtnUp = buttonBox->addButton(i18n("Up"));
  connect(mBtnUp,SIGNAL(clicked()),SLOT(slotBtnUp()));

  mBtnDown  = buttonBox->addButton(i18n("Down"));
  connect(mBtnDown,SIGNAL(clicked()),SLOT(slotBtnDown()));

  mBtnNew = buttonBox->addButton(i18n("New"));
  connect(mBtnNew,SIGNAL(clicked()),SLOT(slotBtnNew()));

  mBtnDelete = buttonBox->addButton(i18n("Delete"));
  connect(mBtnDelete,SIGNAL(clicked()),SLOT(slotBtnDelete()));

  buttonBox->addStretch();
  mBtnOk = buttonBox->addButton(i18n("OK"));
  connect(mBtnOk,SIGNAL(clicked()),SLOT(slotBtnOk()));

  mBtnCancel = buttonBox->addButton(i18n("Cancel"));
  connect(mBtnCancel,SIGNAL(clicked()),SLOT(slotBtnCancel()));

  buttonBox->addStretch();
  btnHelp = buttonBox->addButton(i18n("Help"));
  connect(btnHelp,SIGNAL(clicked()),SLOT(slotBtnHelp()));

  buttonBox->layout();
  buttonBox->setMaximumSize(32767, buttonBox->sizeHint().height());
  buttonBox->setMinimumSize(buttonBox->sizeHint());

  //----------
  grid->setRowStretch(0, 2);
  grid->setRowStretch(1, 10);
  grid->setRowStretch(2, 2);
  grid->setColStretch(0, 10);
  grid->setColStretch(1, 20);
  grid->setRowStretch(3, 100);
  grid->activate();

  resize(buttonBox->sizeHint().width()*1.2, sizeHint().height());
  show();

  reloadFilterList();
  if(mFilterList->count() > 0)
    mFilterList->setCurrentItem(0);
}


//-----------------------------------------------------------------------------
KMFilterDlg::~KMFilterDlg()
{
}


//-----------------------------------------------------------------------------
void KMFilterDlg::reloadFilterList(void)
{
  KMFilter* filter;
  int i, num;

  mFilterList->clear();

  num = filterMgr->count();
  for (i=0; i<num; i++)
  {
    filter = filterMgr->at(i);
    if (!filter) continue;
    mFilterList->insertItem(filter->name());
  }
}


//-----------------------------------------------------------------------------
void KMFilterDlg::clear(void)
{
  int i;
  for (i=0; i<FILTER_MAX_ACTIONS; i++)
  {
    if (mFaBtnDetails[i])
    {
      delete mFaBtnDetails[i];
      mFaBtnDetails[i] = NULL;
    }
    if (mFaField[i])
    {
      delete mFaField[i];
      mFaField[i] = NULL;
    }
  }
}


//-----------------------------------------------------------------------------
void KMFilterDlg::showFilter(KMFilter* aFilter)
{
  int i, w;
  KMFilterAction* action;
  QWidget* pwidg;
  QSize sz;

  assert(aFilter!=NULL); // Important assert
  clear();

  mRuleOp->setCurrentItem((int)aFilter->oper());

  i = indexOfRuleField(aFilter->ruleA().field());
  if (i < 0)
  {
    mRuleFieldA->changeItem(aFilter->ruleA().field(),0);
    i = 0;
  }
  else mRuleFieldA->changeItem(" ",0);
  mRuleFieldA->setCurrentItem(i);
  mRuleFuncA->setCurrentItem((int)aFilter->ruleA().function());
  mRuleValueA->setText(aFilter->ruleA().contents());

  i = indexOfRuleField(aFilter->ruleB().field());
  if (i < 0)
  {
    mRuleFieldB->changeItem(aFilter->ruleB().field(),0);
    i = 0;
  }
  else mRuleFieldB->changeItem(" ",0);
  mRuleFieldB->setCurrentItem(i);
  mRuleFuncB->setCurrentItem((int)aFilter->ruleB().function());
  mRuleValueB->setText(aFilter->ruleB().contents());

  for (i=0; i<FILTER_MAX_ACTIONS; i++)
  {
    action = aFilter->action(i);
    if (mFaField[i]) delete mFaField[i];
    mFaField[i] = NULL;

    if (!action) mFaType[i]->setCurrentItem(0);
    else
    {
      mFaType[i]->setCurrentItem(1+filterActionDict->indexOf(action->name()));
      pwidg = action->createParamWidget(this);
      mFaField[i] = pwidg;
      if (pwidg)
      {
	QPoint pos = mFaType[i]->pos();
	pos.setX(pos.x() + mFaType[i]->width() + 4);
	w = width() - pos.x();
	if (w > 300) w = 300;
	sz.setWidth(w);
	sz.setHeight(pwidg->height());
	pwidg->resize(sz);
	pwidg->move(pos);
	pwidg->show();
      }
    }
  }

  mFilter = aFilter;
  mCurFilterIdx = mFilterList->currentItem();
}


//-----------------------------------------------------------------------------
void KMFilterDlg::applyFilterChanges(void)
{
  KMFilterAction* action;
  int i;

  if (!mFilter || !updown_move_semaphore) return;

  mFilter->ruleA().init(mRuleFieldA->currentText(), 
			(KMFilterRule::Function)mRuleFuncA->currentItem(),
			mRuleValueA->text());
  mFilter->ruleB().init(mRuleFieldB->currentText(), 
			(KMFilterRule::Function)mRuleFuncB->currentItem(),
			mRuleValueB->text());
  mFilter->setOper((KMFilter::Operator)mRuleOp->currentItem());

  mFilter->setName(QString("<")+mRuleFieldA->currentText()+">:"+
		   mRuleValueA->text());
  mFilterList->changeItem(mFilter->name(), mCurFilterIdx);

  for (i=0; i<FILTER_MAX_ACTIONS; i++)
  {
    action = mFilter->action(i);
    if (!action) continue;
    action->applyParamWidgetValue(mFaField[i]);
  }
}


//-----------------------------------------------------------------------------
bool KMFilterDlg::testOpts(const QWidget* w) const
{
  if (!w) debug("KMFilterDlg: no widget given");
  return (w!=NULL);
}


//-----------------------------------------------------------------------------
QPushButton* KMFilterDlg::createDetailsButton(void)
{
  QPushButton* btn;

  btn = mFaBtnDetails[mGridRow];
  if (!btn)
  {
    btn = new QPushButton("...", this);
    mFaBtnDetails[mGridRow] = btn;
  }
  return btn;
}


//-----------------------------------------------------------------------------
QLineEdit* KMFilterDlg::createEdit(const QString aTxt)
{
  QLineEdit* edt = new QLineEdit(this);

  edt->setText(aTxt);
  return edt;
}


//-----------------------------------------------------------------------------
QComboBox* KMFilterDlg::createFolderCombo(const QString curFolder)
{
  QComboBox* cbx = new QComboBox(false, this);
  KMFolderDir* fdir = &(folderMgr->dir());
  KMFolder* cur;
  int i, idx=-1;

  cbx->setFixedHeight(mCbxHeight);

  for (i=0,cur=(KMFolder*)fdir->first(); cur; cur=(KMFolder*)fdir->next(), i++)
  {
    cbx->insertItem(cur->name());
    if (cur->name() == curFolder) idx=i;
  }
  if (idx>=0) cbx->setCurrentItem(idx);
  return cbx;
}


//-----------------------------------------------------------------------------
void KMFilterDlg::slotActionTypeSelected(KMFaComboBox* cbx, int idx)
{
  KMFilterAction* action;
  QWidget* widg;
  QPoint pos;
  QSize sz;
  int i, w;

  if (!mFilter) return;

  for (i=FILTER_MAX_ACTIONS-1; i>0; i--)
    if (cbx == mFaType[i]) break;
  if (i < 0) return;

  if (mFaField[i]) delete mFaField[i];
  mFaField[i] = NULL;
  mGridRow = i;

  if (mFilter->action(i)) delete mFilter->action(i);
  action = filterActionDict->create(
              filterActionDict->nameOf(sFilterActionList.at(idx)));
  mFilter->setAction(i, action);
  if (!action || idx < 0) 
  {
    debug("no action selected");
    return;
  }

  widg = action->createParamWidget(this);
  mFaField[i] = widg;
  if (!widg) return;
  
  pos = mFaType[i]->pos();
  pos.setX(pos.x() + mFaType[idx]->width() + 4);
  widg->move(pos);
  w = width() - pos.x();
  if (w > 300) w = 300;
  sz.setWidth(w);
  sz.setHeight(widg->height());
  widg->resize(sz);
  widg->show();
}


//-----------------------------------------------------------------------------
void KMFilterDlg::slotFilterSelected(int idx)
{

  if (mFilter) applyFilterChanges();
  if ((uint)idx < filterMgr->count())
  {
    mFilter = filterMgr->at(idx);
    if (mFilter) showFilter(mFilter);
  }
  else
  {
    clear();
    mFilter = 0;
  }
}


//-----------------------------------------------------------------------------
void KMFilterDlg::slotBtnUp()
{
  int idx = mFilterList->currentItem();
  KMFilter* filter;
  
  if (idx < 1) return;

  updown_move_semaphore = 0;

  filter = filterMgr->take(idx);
  assert(filter != NULL);
  filterMgr->insert(idx-1, filter);

  mFilterList->removeItem(idx);
  mFilterList->insertItem(filter->name(), idx-1);

  mFilterList->setCurrentItem(idx-1);

  updown_move_semaphore = 1;
}


//-----------------------------------------------------------------------------
void KMFilterDlg::slotBtnDown()
{
  int idx = mFilterList->currentItem();
  KMFilter* filter;

  if (idx < 0 || idx >= (int)mFilterList->count()-1) return;

  updown_move_semaphore = 0;

  filter = filterMgr->take(idx);
  assert(filter != NULL);
  filterMgr->insert(idx+1, filter);

  mFilterList->removeItem(idx);
  mFilterList->insertItem(filter->name(), idx+1);

  mFilterList->setCurrentItem(idx+1);

  updown_move_semaphore = 1;
}


//-----------------------------------------------------------------------------
void KMFilterDlg::slotBtnNew()
{
  int idx;
  KMFilter* filter = new KMFilter;
  filter->setName(i18n("Unnamed"));

  idx = mFilterList->currentItem();
  if (idx >= 0) filterMgr->insert(idx, filter);
  else filterMgr->append(filter);
  idx = filterMgr->find(filter);
  mFilterList->insertItem(filter->name(), idx);
  mFilterList->setCurrentItem(idx);
  slotFilterSelected(idx);

  mFilter=filter;
}


//-----------------------------------------------------------------------------
void KMFilterDlg::slotBtnDelete()
{
  int idx = mFilterList->currentItem();
  if (idx < 0) return;

  mFilter = 0;
  //debug ("I zeroed filter, cross my heart!");

  mFilterList->removeItem(idx);
  filterMgr->remove(idx); //this is one true autoDelete

  if (idx >= (int)filterMgr->count())
    idx = (int)filterMgr->count()-1;

  if (idx >= 0) mFilterList->setCurrentItem(idx);
  mFilter = filterMgr->at(idx);
  if (mFilter) showFilter(mFilter);
}


//-----------------------------------------------------------------------------
void KMFilterDlg::slotBtnOk()
{
  if (mFilter) 
  {
    applyFilterChanges();
    filterMgr->writeConfig();
  }
  accept();
}


//-----------------------------------------------------------------------------
void KMFilterDlg::slotBtnCancel()
{
  filterMgr->readConfig();
  reject();
}


//-----------------------------------------------------------------------------
void KMFilterDlg::slotBtnHelp()
{
  app->invokeHTMLHelp( app->appName() + "/" + "index-3.html", "ss3.5" );
}


//-----------------------------------------------------------------------------
int KMFilterDlg::indexOfRuleField(const QString aName) const
{
  int i;

  for (i=sFilterFieldList.count()-1; i>=0; i--)
  {
    if (sFilterFieldList.at(i)==aName) break;
  }
  return i;
}


//-----------------------------------------------------------------------------
void KMFilterDlg::initLists(void)
{
  QString name;

  //---------- initialize list of filter actions
  if (sFilterActionList.count() <= 0)
  {
    sFilterActionList.append(i18n("<nothing>"));
    for (name=filterActionDict->first(); !name.isEmpty(); 
	 name=filterActionDict->next())
    {
      sFilterActionList.append(filterActionDict->currentLabel());
    }
  }

  //---------- initialize list of filter functions
  if (sFilterOpList.count() <= 0)
  {
    sFilterOpList.append(i18n("ignore"));
    sFilterOpList.append(i18n("and"));
    sFilterOpList.append(i18n("unless"));
    sFilterOpList.append(i18n("or"));
  }

  //---------- initialize list of filter operators
  if (sFilterFuncList.count() <= 0)
  {
    sFilterFuncList.append(i18n("equals"));
    sFilterFuncList.append(i18n("not equal"));
    sFilterFuncList.append(i18n("contains"));
    sFilterFuncList.append(i18n("doesn't contain"));
    sFilterFuncList.append(i18n("regular expression"));
  }

  //---------- initialize list of filter operators
  if (sFilterFieldList.count() <= 0)
  {
    sFilterFieldList.append(" ");
    sFilterFieldList.append(i18n("<message>"));
    sFilterFieldList.append(i18n("<body>"));
    sFilterFieldList.append(i18n("<any header>"));
    sFilterFieldList.append("Subject");
    sFilterFieldList.append("From");
    sFilterFieldList.append("To");
    sFilterFieldList.append("Cc");
    sFilterFieldList.append("Reply-To");
    sFilterFieldList.append("Organization");
    sFilterFieldList.append("Resent-From");
    sFilterFieldList.append("X-Loop");
  }
}


//-----------------------------------------------------------------------------
#include "kmfilterdlg.moc"
