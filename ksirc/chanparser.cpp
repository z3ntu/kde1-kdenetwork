#include "chanparser.h"
#include "estring.h"
#include "alistbox.h"
#include <iostream.h>
#include <stdio.h>
#include <ssfeprompt.h>

// Static parser table is "initialized"
QDict<parseFunc> ChannelParser::parserTable;


ChannelParser::ChannelParser(KSircTopLevel *_top) /*fold00*/
{
  top = _top;

  /*
   * Initial helper variables
   */
  prompt_active = FALSE;
  continued_line = FALSE;

  if(parserTable.isEmpty() == TRUE){
    parserTable.setAutoDelete(TRUE);
    parserTable.insert("`l`", new parseFunc(&parseSSFEClear));
    parserTable.insert("`s`", new parseFunc(&parseSSFEStatus));
    parserTable.insert("`i`", new parseFunc(&parseSSFEInit));
    parserTable.insert("`t`", new parseFunc(&parseSSFEMsg));
    parserTable.insert("`o`", new parseFunc(&parseSSFEOut));
    parserTable.insert("`p`", new parseFunc(&parseSSFEPrompt));
    parserTable.insert("`P`", new parseFunc(&parseSSFEPrompt));
    parserTable.insert("`R`", new parseFunc(&parseSSFEReconnect));
    // The rest are *** info message
    parserTable.insert("***", new parseFunc(&parseINFOInfo));
    parserTable.insert("*E*", new parseFunc(&parseINFOError));
    parserTable.insert("*#*", new parseFunc(&parseINFONicks));
  }

}

void ChannelParser::parse(QString string) /*FOLD00*/
{
  parseFunc *pf;
  if(string.length() < 3){
    warning("Dumb string, too short: %s", string.data());
    throw(parseError(1, string));
  }

  /**
   * Start pre-parsing the strings to make them fit the 3 character
   * parser codes, etc
   */
  
  /*
   * SSFE control messages are too long, so we convert the
   * messges into a 3 character code, `#ssfe#\S becomes `\S`
   */
  if(string[0] == '`'){
    if(strncmp("`#ssfe#", string, 7) == 0){
      char s[] = { string[7], 0 };
      uint space;
      for(space = 6; (string[space] != ' '); space ++){
        if(space >= string.length()){
          space++;
          break;
        }
      }
      string.remove(0, space);
      string.prepend("` ");
      string.prepend(s);
      string.prepend("`");
    }
  }
  /*
   * If we are watching for a continued line, check to see if the current
   * line is not a nick list.  If it isn't the coninued line is finished
   * so set it false
   */
  if(continued_line == TRUE){
    // Don't switch back for ssfe control messages, but for all the rest
    if(string[0] != '`' && string[1] != '#')
      continued_line = FALSE;
  }

  // Pre-parsing is now complete
  
  pf = parserTable[string.mid(0, 3)];
  if(pf != 0x0){
    debug("New hanlder handling: %s", string.data());
    (this->*(pf->parser))(string);
  }
  // If it's unkown we just fall out of the function
}

void ChannelParser::parseSSFEClear(QString string) /*fold00*/
{
  top->mainw->clear();
  top->mainw->repaint(TRUE);
  string.truncate(0);
  throw(parseSucc(QString(""))); // Null string, don't display anything
}

void ChannelParser::parseSSFEStatus(QString string) /*fold00*/
{
  const int offset = 13;// 10 for " [sirc] " 3 for "`s`"
  QString new_caption = string.mid(offset, string.length() - offset);
  if(new_caption != top->caption){
    if(new_caption[0] == '@')                 // If we're an op,,
      // update the nicks popup menu
      top->opami = TRUE;                  // opami = true sets us to an op
    else
      top->opami = FALSE;                 // FALSE, were not an ops
    top->UserUpdateMenu();                // update the menu
    top->setCaption(new_caption);
    if(top->ticker)
      top->ticker->setCaption(new_caption);
    top->caption = new_caption;           // Make copy so we're not
    // constantly changing the title bar
  }
  throw(parseSucc(QString(""))); // Null string, don't display anything
}

void ChannelParser::parseSSFEInit(QString string) /*fold00*/
{
  throw(parseSucc(QString(""))); // Null string, don't display anything
}

void ChannelParser::parseSSFEOut(QString string) /*fold00*/
{
  throw(parseSucc(QString(""))); // Null string, don't display anything
}

void ChannelParser::parseSSFEMsg(QString string) /*fold00*/
{
  if(string.length() > 100)
    throw(parseError(0, string, "String length for nick is greater than 100 characters, insane, too big"));

  char nick[string.length()];
  int found = sscanf(string.data(), "`t` %s", nick);

  if(found < 1)
    throw(parseError(1, string, "Could not find nick in string"));

  if(!top->nick_ring.contains(nick)){
    top->nick_ring.append(nick);
    cerr << "Appending: " << nick << endl;
    if(top->nick_ring.count() > 10)
      top->nick_ring.removeFirst();
  }
  throw(parseSucc(QString(""))); // Null string, don't display anything
}


void ChannelParser::parseSSFEPrompt(QString string) /*fold00*/
{
  if(prompt_active == FALSE){
    QString prompt, caption;
    ssfePrompt *sp;
    int p1, p2;

    // Flush the screen.
    // First remove the prompt message from the Buffer.
    // (it's garunteed to be the first one)
    top->LineBuffer->removeFirst();
    top->Buffer = FALSE;
    top->sirc_receive(QString(""));

    caption = top->mainw->text(top->mainw->count() - 1);
    if(caption.length() < 3){
      caption = top->mainw->text(top->mainw->count() - 2);
      if(caption.length() > 2)
	top->mainw->removeItem(top->mainw->count() - 2 );
    }
    else
      top->mainw->removeItem(top->mainw->count() - 1 );
    p1 = 4; // "'[pP]' " gives 4 spaces
    p2 = string.length();
    if(p2 <= p1)
      prompt = "No Prompt Given?";
    else
      prompt = string.mid(p1, p2 - p1);
    prompt_active = TRUE;
    // If we use this, then it blows up
    // if we haven't popped up on the remote display yet.
    sp = new ssfePrompt(prompt, 0);
    sp->setCaption(caption);
    if(string[1] == 'P')
      sp->setPassword(TRUE);
    sp->exec();
    //	  cerr << "Entered: " << sp->text() << endl;
    prompt = sp->text();
    prompt += "\n";
    emit top->outputLine(prompt);
    delete sp;
    prompt_active = FALSE;
  }

  throw(parseSucc(QString(""))); // Null string, don't display anything
}

void ChannelParser::parseSSFEReconnect(QString string) /*fold00*/
{
  if(top->channel_name[0] == '#'){
    QString str = "/join " + QString(top->channel_name) + "\n";
    emit top->outputLine(str);
  }

  throw(parseSucc(QString(""))); // Null string, don't display anything
}

void ChannelParser::parseINFOInfo(QString string) /*fold00*/
{
  string.remove(0, 3);                // takes off the junk

  throw(parseSucc(string, kSircConfig->colour_info, top->pix_info)); // Null string, don't display anything
}

void ChannelParser::parseINFOError(QString string) /*fold00*/
{
  string.remove(0, 3);               // strip the junk

  throw(parseSucc(string,kSircConfig->colour_error, top->pix_madsmile)); // Null string, don't display anything
}

void ChannelParser::parseINFONicks(QString in_string) /*FOLD00*/
{

  EString string = in_string;
  EString nick;

  int start, end, count;
  char channel_name[101];

  // Get the channel name portion of the string
  count = sscanf(string, "*#* Users on %100[^:] ", channel_name);
  if(count < 1)
    throw(parseError(1, string, "Could not find channel name"));

  if (strcasecmp(channel_name,top->channel_name) != 0){
    string.remove(0,3);
    throw(parseSucc(string,kSircConfig->colour_info,top->pix_info));
  }

  top->nicks->setAutoUpdate(FALSE);        // clear and update nicks
  if(continued_line == FALSE)
    top->nicks->clear();
  continued_line = TRUE;
  start = string.find(": ", 0, FALSE) + 1; // Find start of nicks
  while(start > 0){                     // While there's nick to go...
    try {
      end = string.find(" ", start + 1, FALSE); // Find end of nick
    }
    catch (estringOutOfBounds &err){
      end = string.length();         // If the end's not found,
    }
    // set to end of the string
    nick = string.mid(start+1, end - start - 1); // Get nick
    if(nick[0] == '@'){    // Remove the op part if set
      nick.remove(0, 1);
      nickListItem *irc = new nickListItem();
      irc->setText(nick);
      irc->setOp(TRUE);
      top->nicks->inSort(irc);
    }                                  // Remove voice if set
    else if(nick[0] == '+'){
      nick.remove(0, 1);
      nickListItem *irc = new nickListItem();
      irc->setVoice(TRUE);
      irc->setText(nick);
      top->nicks->inSort(irc);
    }
    else{
      top->nicks->inSort(nick);
    }
    try{
      start = string.find(" ", end, FALSE); // find next nick
    }
    catch (estringOutOfBounds &err){
      start = -1;
    }

  }
  top->nicks->setAutoUpdate(TRUE);         // clear and repaint the listbox
  top->nicks->repaint(TRUE);
  throw(parseSucc(QString("")));           // Parsing ok, don't print anything though
}