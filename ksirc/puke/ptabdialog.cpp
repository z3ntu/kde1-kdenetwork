#include <stdio.h>

#include "ptabdialog.h"
#include "commands.h"
#include <iostream.h>

PObject *createWidget(CreateArgs &ca)
{
  PTabDialog *ptd = new PTabDialog(ca.parent);
  QTabDialog *qtd;
  // Retreive the border and direction information out of the
  // carg string

  cerr << "Bong!\n";
  
  if(ca.fetchedObj != 0 && ca.fetchedObj->inherits("QTabDialog") == TRUE)
    qtd = (QTabDialog *) ca.fetchedObj;
  else if(ca.parent != 0 && ca.parent->widget()->isWidgetType() == TRUE)
    qtd = new QTabDialog((QWidget *) ca.parent->widget());
  else
    qtd = new QTabDialog();
  ptd->setWidget(qtd);
  ptd->setWidgetId(ca.pwI);
  ptd->setPukeController(ca.pc);
  return ptd;
}

PTabDialog::PTabDialog(QObject *pobject)
  : PWidget()
{
    // Connect slots as needed
    cerr << "Bong2!\n";
  setWidget(0);
}

PTabDialog::~PTabDialog()
{
  //  debug("PObject: in destructor");
  delete widget();
  tab = 0;
  setWidget(0);
}

void PTabDialog::messageHandler(int fd, PukeMessage *pm)
{
  PukeMessage pmRet;

  if(pm->iCommand == PUKE_TABDIALOG_ADDTAB){
    if(!(pm->iTextSize > 0)){
      warning("QTabDialog/addtab: incorrent cArg size, bailing out.  Needed: >0 got: %d\n", pm->iTextSize);
      pmRet.iCommand = PUKE_TABDIALOG_ADDTAB_ACK; // ack the add widget
      pmRet.iWinId = pm->iWinId;
      pmRet.iArg = 1;
      pmRet.cArg = 0;
      emit outputMessage(fd, &pmRet);
      return;
    }
    widgetId wiWidget;
    wiWidget.fd = fd;
    wiWidget.iWinId = pm->iArg;
    PWidget *pw = controller()->id2pwidget(&wiWidget);
    //    debug("Adding widget with stretch: %d and align: %d", (int) pm->cArg[0],
    //	  (int) pm->cArg[1]);
    widget()->addTab(pw->widget(), pm->cArg);

    pmRet.iCommand = PUKE_TABDIALOG_ADDTAB_ACK; // ack the add widget
    pmRet.iWinId = pm->iWinId;
    pmRet.iArg = 0;
    pmRet.cArg = 0;
    emit outputMessage(fd, &pmRet);
  }
  else {
    PWidget::messageHandler(fd, pm);
  }

}

void PTabDialog::setWidget(QTabDialog *tb)
{
  //  debug("PObject setwidget called");
  tab = tb;
  if(tab != 0){
  }
  PObject::setWidget(tb);
  
}

QTabDialog *PTabDialog::widget()
{
  //  debug("PObject widget called");
  return tab;
}