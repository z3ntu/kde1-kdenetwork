#include "kformatter.h"
#include "kstrtable.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <qfileinf.h>
#include <qregexp.h>
#include "kdecode.h"
#include <malloc.h>
#include <qregexp.h>
#include <kapp.h>
#include <mimelib/mimepp.h>

extern char *debugbuf;

KFormatter::KFormatter(QString sWN, QString vWN, QString s, bool c)
{
    saveWidgetName=sWN;
    viewWidgetName=vWN;
    message=new DwMessage(s.data());
    message->Parse();

    //Get the date format from the config
    QString *defFmt=new QString("%y/%m/%d");
    kapp->getConfig()->setGroup("Appearance");
    dateFmt=new QString(kapp->getConfig()->readEntry("Dateformat",defFmt->data()));
    CHECK_PTR(dateFmt);
    if(dateFmt->isEmpty())
        dateFmt=defFmt;
    else
        delete defFmt;
    mcomplete=c;
}

KFormatter::~KFormatter()
{
    delete dateFmt;
}

DwBodyPart* KFormatter::ffwdPart(int n, DwBodyPart* body)
{
    DwBodyPart* curr=body;
    for(int i=0; i<n; i++)
    {
        curr=curr->Next();
        //Fail if we have returned to the start of the part ring
        if(!curr) return NULL;
    }
    return curr;
}

DwBodyPart* KFormatter::getPartPrim(QList<int> partno, DwBodyPart* body)
{
    //The base case
    if(partno.count()==1) return ffwdPart(*(partno.first()),body);

    DwBodyPart* curr;
    curr=ffwdPart(*(partno.first()),body);
    int dummy=0;
    partno.remove(dummy);
    return getPartPrim(partno, curr->Body().FirstBodyPart());
}

DwBodyPart* KFormatter::getPart(QList<int> partno)
{
    DwBodyPart* iter;
    iter=message->Body().FirstBodyPart();
    //Clone the main body is there was no bodyparts at all, and the first was
    //requested. Fail if the article does not exist at all
    if(!iter)
    {
        if(partno.count()==1 && *(partno.at(0))==0)
        {
            return new DwBodyPart(message->AsString(), message);
        }
        else
        {
            return NULL;
        }
    }

    return getPartPrim(partno,iter);
}

bool KFormatter::isMultiPart(QList<int> part)
{
    DwBodyPart* p=getPart(part);
    return (p->Body().FirstBodyPart()!=NULL);
}

const char* KFormatter::rawPart(QList<int> partno)
{
    DwBodyPart* body=getPart(partno);
    const char* data=body->Body().AsString().c_str();
    return data;
}

QString KFormatter::htmlAll()
{
    if(mcomplete==FALSE)
    {
        return "<b>The requested message could not be found either in the "
                 "cache or on the server. Click on this link to try to "
                 "search for it with Altavista.</b>"+
                 searchLink("C Directory","comp.lang.c.moderated");
    }
    QList<int> l;
    int i=0;
    bool done=FALSE;
    QString text;
    int dummy=0;

    while(!done)
    {
        l.append(&i);
        if(getPart(l)) text+=htmlPart(l);
        else done=TRUE;
        l.remove(dummy);
        i++;
        if(!done) text+="<hr>";
        text+="\n";
    }
    return text;
}


QString KFormatter::htmlPart(QList<int> partno)
{
    DwBodyPart* body;
    body=getPart(partno);
    CHECK_PTR(body);
    body->Parse();
    int pos;

    QString baseType=body->Headers().ContentType().TypeStr().c_str();
    //Work-around for bug in mimelib
    pos=baseType.find('\n');
    if(pos!=-1) baseType=baseType.left(pos);

    pos=baseType.find('/');
    if(pos!=-1) baseType=baseType.left(pos);

    baseType=baseType.lower();

    QString subType=body->Headers().ContentType().SubtypeStr().c_str();
    //Work-around for bug in mimelib
    pos=subType.find('\n');
    if(pos!=-1) subType=subType.left(pos);
    pos=subType.find(';');
    if(pos!=-1) subType=subType.left(pos);
    subType=subType.lower();

    //Set a default type, just in case we lack a "content-type" header
    if (baseType.isEmpty())
    {
        KDEBUG (KDEBUG_INFO,3300,"no type, assuming text/plain");
        baseType="text";
        subType="plain";
    }

    QString wholeType;
    wholeType=baseType.copy();
    if(!subType.isEmpty()) wholeType+="/"+subType;

    QString encoding;
    if(body->Headers().HasCte())
    {
        encoding=body->Headers().ContentTransferEncoding().AsString().c_str();
        encoding=encoding.lower();
    }
    else{
        KDEBUG (KDEBUG_INFO,3300,"No encoding, assuming 7bit");
        encoding="7bit";
    }


    const char* udata=body->Body().AsString().c_str();
    CHECK_PTR(udata);

    const char* data=KDecode::decodeString(udata,encoding)->c_str();
    CHECK_PTR(data);

    if (baseType=="text")
    {
//        DwToLocalEol(data,data);
        if (subType=="html")
        {
            KDEBUG (KDEBUG_INFO,3300,"Found text/html part.");
            return text_htmlFormatter(data,partno);
        }
        else if (subType=="plain")
        {
            KDEBUG (KDEBUG_INFO,3300,"Found text/plain part.");
            return text_plainFormatter(data, partno);
        }
        else if (subType=="richtext")
        {
            KDEBUG (KDEBUG_INFO,3300,"Found text/richtext part");
            return text_richtextFormatter(data, partno);
        }
        else if (subType=="x-vcard")
        {
            KDEBUG (KDEBUG_INFO,3300,"Found mozilla vcard.");
            return text_x_vcardFormatter(data, partno);
        }
        else
        {
            sprintf (debugbuf,"Found unknown text part! (%s)", subType.data());
            KDEBUG (KDEBUG_INFO,3300,debugbuf);
            return text_plainFormatter(data, partno);
        }
    }
    else if (baseType=="image")
    {
        if(subType=="jpeg")
        {
            KDEBUG (KDEBUG_INFO,3300,"Found jpeg image");
            QByteArray a(strlen(data));
            a.setRawData(data,strlen(data));
            return image_jpegFormatter(a,partno);
        }
        else
        {
            KDEBUG (KDEBUG_INFO,3300,"Found unknown image part!");
            QString part;
            part.sprintf("This message part consists of an image of an "
                         "unsupported type (%s)<br>.\n %s",
                         subType.data(), saveLink(partno, "").data() );
            return part;
        }
    }
    else if (baseType=="multipart")
    {
        if(subType=="alternative")
        {
            debug("Found multipart/alternative message parts");
            int i=0;
            int max=0, maxpos=0, score;
            DwBodyPart* curr;
            QString baseType, subType;
            bool done=FALSE;
            while(!done)
            {
                int* j=new int;
                *j=i;
                partno.append(j);
                //debug("Checking part: %s",listToStr(partno).c_str());
                curr=getPart(partno);
                //debug("* curr=%p",curr);
                //debug("data: \"%s\"",curr->AsString().c_str());
                if(curr!=NULL)
                {
                    baseType=curr->Headers().ContentType().TypeStr().c_str();
                    subType=curr->Headers().ContentType().SubtypeStr().c_str();
                    score=rateType(baseType,subType);
                    //debug("This part scored %d",score);
                    if(score>=max)
                    {
                        //debug("A new world record!");
                        max=score;
                        maxpos=i;
                    }
                }
                else done=TRUE;
                partno.remove(partno.count()-1);
                i++;
            }
            partno.append(&maxpos);
            //debug("*** Best version: %s",listToStr(partno).c_str());
            if(getPart(partno)==NULL)
            {
                return "<strong>This part claims to be made up of several "
                    "parts, out of wich one should be displayd. However, no "
                    "part was found at all. This might indicate a bug in "
                    "krn's KFormatter class.</strong>\n";
            }
            else return htmlPart(partno);
        }
        else if(subType=="related")
        {
            debug("Found related multipart parts");
            QString part="<b>This part consists of several related parts. "
                "There is not yet any support for embedding such "
                "parts into each other, but who knows, that may come "
                "some day. Anyway, here are all the parts one at a "
                "time:</b><hr>\n";
            int i=0;
            DwBodyPart* curr;
            bool done=FALSE;
            while(!done)
            {
                int* j=new int;
                *j=i;
                partno.append(j);
                curr=getPart(partno);
                if(curr!=NULL) part+=htmlPart(partno)+"<hr>\n";
                else done=TRUE;
                partno.remove(partno.count()-1);
                i++;
            }
            return part;
        }
        else if(subType=="digest")
        {
            debug("Found multipart/digest part");
            QString part="<b>This part consists of several parts, "
                "some being digests of thers. There is not yet any "
                "support for displaying such parts correctly, but "
                "who knows, that may come some day. Anyway, here "
                "are all the parts one at a time:</b><hr>\n";
            int i=0;
            DwBodyPart* curr;
            bool done=FALSE;
            while(!done)
            {
                int* j=new int;
                *j=i;
                partno.append(j);
                curr=getPart(partno);
                if(curr!=NULL) part+=htmlPart(partno)+"<hr>\n";
                else done=TRUE;
                partno.remove(partno.count()-1);
                i++;
            }
            return part;
        }
        else
        {
            debug("unknown multipart subtype encountered: %s",subType.data());
            QString part;
            part.sprintf("<b>This type of multipart messages is not supported yet.</b>",subType.data());
            return part;
        }

    }
    else
    {
        debug("Type %s not handled natively. Searching for plug-in",
              wholeType.data());
        KConfig* conf=kapp->getConfig();
        conf->setGroup("Formatters");
        if(conf->hasKey(wholeType.data()))
        {
            QString plugin=conf->readEntry(wholeType);
            debug("Plug-in found: %s",plugin.data());
            int i=tempfile.create("format_in","");
            int o=tempfile.create("format_out","");
            QFile* f=tempfile.file(i);
            f->open(IO_WriteOnly);
            f->writeBlock(data,strlen(data));
            f->close();

            system((plugin+" <"+tempfile.file(i)->name()+" >"+
                   tempfile.file(o)->name()).data());

            f=tempfile.file(o);
            f->open(IO_ReadOnly);
            char* ndata=(char*)malloc(f->size());
            f->readBlock(ndata,f->size());
            f->close();

            tempfile.remove(i);
            tempfile.remove(o);

            return ndata;
        }

        debug ("Found some part! (%s)",wholeType.data());
        QString part;
        part.sprintf("This message part consists of an attachment of an "
                     "unsupported type (%s)<br>.\n%s%s",
                     wholeType.data(),
                     saveLink(partno, "").data(),viewLink(partno,"").data() );
        return part;
    }
}

QString KFormatter::getType(QList<int> part)
{
    int pos;
    QString baseType=getPart(part)->Headers().ContentType().TypeStr().c_str();
    pos=baseType.find('\n');
    if(pos!=-1) baseType=baseType.left(pos);
    pos=baseType.find(';');
    if(pos!=-1) baseType=baseType.left(pos);
    QString subType=getPart(part)->Headers().ContentType().SubtypeStr().c_str();
    pos=subType.find('\n');
    if(pos!=-1) subType=subType.left(pos);
    QString wholeType=baseType;
    if(!subType.isEmpty()) baseType+=subType;
    return wholeType;
}

QString KFormatter::saveLink(QList<int> part, char* text)
{
    QString type=getType(part);

    QString link;
    QString partno=listToStr(part);
    debug("Creating save link for part %s of type %s. Text: %s Widgetname: %s",
          partno.data(), type.data(), text, saveWidgetName.data());
    link.sprintf("<a href=\"save://%s/%s\">%s<img src=%s alt=\"save\"></a>",
                 partno.data(), type.data(), text,
                 saveWidgetName.data() );
    return link;
}

QString KFormatter::viewLink(QList<int> part, char* text)
{
    QString type=getType(part);

    QString link;
    link.sprintf("<a href=\"view://%s/%s\">%s<img src=%s alt=\"view\"></a>",
                 listToStr(part).data(), type.data(), text,
                 viewWidgetName.data() );
    return link;
}

QString KFormatter::mailLink(QString reciptent, char* text)
{
    return "<a href=\"mailto:" + reciptent + "\">" + text + "</a>";
}

QString KFormatter::searchLink(QString subj, QString group)
{
    subj.replace(QRegExp(" "),"+");

    return "<a href=\"http://www.altavista.digital.com/cgi-bin/query?pg=aq"
           "&kl=XX&what=news&fmt=d&q=%22" + subj + "%22+AND+" + group +
           "&r=" + group + "&d0=" + "" + "&d1=" + "" + "\">"
           "Search!</a>\n";
}

QString KFormatter::htmlHeader()
{
    QString header("<pre>");
    //Build the header
    QStrIList visheaders;
    visheaders.setAutoDelete(true);
    // This should be handled by a dialog
    // And stored in the config file
    visheaders.append("From");
    visheaders.append("To");
    visheaders.append("Subject");
    visheaders.append("Newsgroups");
    visheaders.append("Reply-To");
    visheaders.append("Followup-To");
    visheaders.append("Date");
    visheaders.append("References");

    for (char *iter=visheaders.first();!iter==0;iter=visheaders.next())
    {
        if (message->Headers().HasField(iter))
        {
            QString headerName=iter;
            QString headerContents=message->Headers().FieldBody(iter).AsString().c_str();
            debug("Header contents: %s",headerContents.data());
            header+="<b>"+headerName.leftJustify(10)+":</b> ";

            if(headerName=="Newsgroups" || headerName=="Followup-to")
            {
                headerContents.simplifyWhiteSpace();
                headerContents+=',';
                unsigned int index=0, len;
                QString group;
                while(index<headerContents.length())
                {
                    len=headerContents.find(',',index)-index;
                    group=headerContents.mid(index,len);
                    header+="<a href=\"news://newsserver/"+group+"\">"+group+"&nbsp;</a>";
                    index+=len+1;
                }
                header+="\n";
            }
            else if(headerName=="References")
            {
                QString articles=headerContents;
                articles.simplifyWhiteSpace();
                articles+=' ';
                unsigned int index=0, len;
                QString article,t;
                int count=1;
                while(index<articles.length())
                {
                    len=articles.find(' ',index)-index;
                    article=articles.mid(index,len);
                    index+=len+1;
                    article=article.mid(1,article.length()-2);
                    if (article.isEmpty()) continue;
                    t.setNum(count++);
                    t="<a href=\"news:///"+article+"\">"+t+"</a> ";
                    header+=t;
                }
                header+="\n";
            }
            else
                // header without special formatting
                // needs to escape < and >
            {
                QString s=headerContents;
                s.replace(QRegExp("<"),"&lt;");
                s.replace(QRegExp(">"),"&gt;");
                header=header+s+"\n";
            }
        }
    }
    header+="</pre>";
    return header;
}

QString KFormatter::image_jpegFormatter(QByteArray data, QList<int> partno)
{
    QString part;
    QString name=tmpnam(NULL);
    QFile file(name);
    file.open(IO_WriteOnly);
    file.writeBlock(data.data(),data.size());
    file.close();
    QString link;
    link.sprintf("<img src=%s alt=\"%d kb image\">",
                 name.data(), QFileInfo(name).size()/1024 );
    part.sprintf("Attached jpeg image<br>\n %s",
                 saveLink(partno, link.data()).data() );
    /*
     else
     {
     part.sprintf("This part consists of an jpeg image, wich unfortunately "
     "could not be automatically saved, and therefore can not be"
     "shown. To save this image, click on save %s",
     saveLink(partno, "").c_str() );
     }
     */
    return part;
}

QString KFormatter::text_plainFormatter(QString data, QList<int>)
{
    bool insig=false;
    QString st,sig,body;
    while (!data.isEmpty())
    {

        int i=data.find("\n");
        if (i==-1)
        {
            st=data;
            data="";
        }
        else
        {
            st=data.left(i);
            data=data.right(data.length()-i-1);
        };
        if (st.left(2)=="--")
        {
            //If it's more than 8 lines, it prabably isn't a sig
            if(st.contains('\n')<=8) insig=true;
        }
        st.replace(QRegExp("<"),"&lt;");
        st.replace(QRegExp(">"),"&gt;");

        if (insig)
        {
            sig=sig+st+"\n";
        }
        else
        {
            if (st.left(4)=="&gt;")      //It's a quote
            {
                // This doesn't work. Why?
                st="<i>"+st+"</i>";

            }
            else if(st.contains('-') + st.contains('+') + st.contains('|') +
                    st.contains('/') + st.contains('\\') > (int) st.length()/2)
            {
                //it's part of an ASCII drawing
                st="<pre>"+st+"</pre>\n";
            }
            else
            {
                int pos=0;
                int* len=new int;
                QRegExp re("_[a-z]*_",FALSE);

                while(pos<(int)st.length())
                {
                    pos=re.match(st.left(st.length()-pos), pos, len);
                    if(pos==-1) {
                        break;
                    }
                    QString word;
                    word=st.mid(pos+1,*len-2);
                    word.prepend("<i>");
                    word.append("</i>");
                    st.replace(pos, *len, word.data());
                }
                delete len;
            }

            if (st.isEmpty())    //Empty line (end of para)
                st="<p>";
            else                      //Plain line
                st=st+"<br>";
            body+=st+"\n";
        }
    }
    return body+"<pre>"+sig+"</pre>";
}

QString KFormatter::text_x_vcardFormatter(QString data, QList<int>)
{
    QString result;

    KStrTable card;
    card.read(data);

    result="<h3>This is the mozilla vcard of ";

    QString name;
    if(card.hasItem("fn")) name=card.getItem("fn");
    else if(card.hasItem("n")) name=card.getItem("n");
    else name="an unknown person.";
    if(card.hasItem("email;internet"))
        result+=mailLink(card.getItem("email;internet").data(), name.data());
    else result+=name;
    result+="</h3><br>\n";

    if(card.hasItem("n")) result+="<b>Name: </b>"+card.getItem("n")+"<br>\n";
    if(card.hasItem("fn")) result+="<b>Full name: </b>"+card.getItem("fn")+"<br>\n";
    if(card.hasItem("org")) result+="<b>Organization: </b>"+card.getItem("org")+"<br>\n";
    if(card.hasItem("title")) result+="<b>Title: </b>"+card.getItem("title")+"<br>\n";

    if(card.hasItem("x-mozilla-html"))
    {
        result+="<string>The sender prefers ";
        if(card.getItem("x-mozilla-html")=="yes") result+="html";
        else result+="plaintext";
        result+=" emails.</strong><br>\n";
    }

    if(card.hasItem("note")) result+="<b>Note: </b><blockquote>"+card.getItem("note") + "</blockquote><br>\n";

    return result + "<hr>The rest of this vcard can not be shown yet. Work is being done at"
        "supporting more fileds. The raw card follows:<br>\n<pre>" + data + "</pre>\n";
}

QString KFormatter::text_richtextFormatter(QString data, QList<int>)
{
    data.replace(QRegExp("<lt>",FALSE),"&lt;");
    data.replace(QRegExp("<nl>",FALSE),"<br>");
    data.replace(QRegExp("<np>",FALSE),"<hr>");
    data.replace(">","&gt;");
    data.replace("\\n","<br>");

    data.replace(QRegExp("<bold>",FALSE),"<b>");
    data.replace(QRegExp("</bold>",FALSE),"</b>");

    data.replace(QRegExp("<italic>",FALSE),"<i>");
    data.replace(QRegExp("</italic>",FALSE),"</i>");

    data.replace(QRegExp("<fixed>",FALSE),"<tt>");
    data.replace(QRegExp("</fixed>",FALSE),"</tt>");

    data.replace(QRegExp("<flushleft>",FALSE),"<p align=left>");
    data.replace(QRegExp("</flushleft>",FALSE),"</p>");

    data.replace(QRegExp("<flushright>",FALSE),"<p align=right>");
    data.replace(QRegExp("</flushright>",FALSE),"</p>");

    data.replace(QRegExp("<bigger>",FALSE),"<big>");
    data.replace(QRegExp("</bigger>",FALSE),"</big>");

    data.replace(QRegExp("<smaller>",FALSE),"<small>");
    data.replace(QRegExp("</smaller>",FALSE),"</small>");

    data.replace(QRegExp("<subscript>",FALSE),"<sub>");
    data.replace(QRegExp("</subscript>",FALSE),"</sub>");

    data.replace(QRegExp("<superscript>",FALSE),"<sup>");
    data.replace(QRegExp("</superscript>",FALSE),"</sup>");

    data.replace(QRegExp("<excerpt>",FALSE),"<blockquote>");
    data.replace(QRegExp("</excerpt>",FALSE),"</blockquote>");

    data.replace(QRegExp("<comment>.*</comment>",FALSE),"");

    data.replace(QRegExp("<no-op>",FALSE),"");
    data.replace(QRegExp("</no-op>",FALSE),"");

    data.replace(QRegExp("<Underline>",FALSE),"<em>");
    data.replace(QRegExp("</Underline>",FALSE),"</em>");

    data.replace(QRegExp("<Indent>",FALSE),"");
    data.replace(QRegExp("</Indent>",FALSE),"");

    data.replace(QRegExp("<IndentRight>",FALSE),"");
    data.replace(QRegExp("</IndentRight>",FALSE),"");

    data.replace(QRegExp("<Outdent>",FALSE),"");
    data.replace(QRegExp("</Outdent>",FALSE),"");

    data.replace(QRegExp("<OutdentRight>",FALSE),"");
    data.replace(QRegExp("</OutdentRight>",FALSE),"");

    data.replace(QRegExp("<SamePage>",FALSE),"");
    data.replace(QRegExp("</SamePage>",FALSE),"");

    data.replace(QRegExp("<Heading>",FALSE),"");
    data.replace(QRegExp("</Heading>",FALSE),"");

    data.replace(QRegExp("<Footing>",FALSE),"");
    data.replace(QRegExp("</Footing>",FALSE),"");

    data.replace(QRegExp("<ISO-8859-.*>",FALSE),"<b>Krn note: This text is incoded in another scharacter set, and may look wierd</b>");
    data.replace(QRegExp("</ISO-8859-.*>",FALSE),"<b>Krn note: Character set reset.</b>");

    data.replace(QRegExp("<US-ASCII>",FALSE),"");
    data.replace(QRegExp("</US-ASCII>",FALSE),"");

    data.replace(QRegExp("<Paragraph>",FALSE),"<p>");
    data.replace(QRegExp("</Paragraph>",FALSE),"</p>");

    data.replace(QRegExp("<Signature>",FALSE),"<br><small><pre>");
    data.replace(QRegExp("</Signature>",FALSE),"</pre></small>>");

    /*
     data.replace(QRegExp("<>,FALSE)","<>");
     data.replace(QRegExp("</>,FALSE)","</>");
     */

    return data;

}
QString KFormatter::text_htmlFormatter(QString data, QList<int> )
{
    data.replace(QRegExp("<html>",FALSE),"");
    data.replace(QRegExp("</html>",FALSE),"");
    data.replace(QRegExp("<head>",FALSE),"");
    data.replace(QRegExp("</head>",FALSE),"");
    data.replace(QRegExp("<body>",FALSE),"");
    data.replace(QRegExp("</body>",FALSE),"");
    data.replace(QRegExp("<title>",FALSE),"<b>Title: </b>");
    data.replace(QRegExp("</title>",FALSE),"");

    return data;
}

QString KFormatter::listToStr(QList<int> l)
{
    QString r;
    QString t;
    for(int i=0; i<(int)l.count(); i++)
    {
        t.setNum(*l.at(i));
        r.append(t);
        if(i<(int)l.count()-1) r.append(".");
    }
    return r;
}

QList<int> KFormatter::strToList(QString s)
{
    QList<int> l;
    s+='.';
    unsigned int start=0,end;
    int* t;
    while(start<s.length()-1)
    {
        end=s.find('.',start);
        t=new int;
        *t=atoi(s.mid(start,end-start));
        l.append(t);
        start=end+1;
    }
    return l;
}

bool KFormatter::dump(QList<int> part, QString fileName)
{
    DwString src=getPart(part)->Body().AsString();

    QFile file(fileName);
    if(!file.open(IO_WriteOnly)) return FALSE;
    if(file.writeBlock(src.c_str(), src.length()) != (int)src.length())
    {
        file.close();
        return FALSE;
    }
    file.close();
    return TRUE;
}

//This function returns an integer that can be used to decide what part
//to display, when having to choose between several.
//The result may be between 0 and 999, inclusive.
unsigned int KFormatter::rateType(QString baseType, QString subType)
{
    debug("***Rating %s/%s",baseType.data(),subType.data());
    if(baseType=="message")
    {
        return 700;
    }
    if(baseType=="multipart")
    {
        if(subType=="digest")		return 603;
        if(subType=="parallel")		return 602;
        if(subType=="mixed")		return 601;
        if(subType=="alternative")	return 600;
    }
    //FIXME: rate other base types!
    //if(baseType!="text")
    //{
    //    return 200;
    //}
    if(baseType=="text")
    {
        if(subType=="html")	return 104;
        if(subType=="richtext")	return 103;
        if(subType=="plain")	return 102;
        else			return 100;
    }

    else return 0;
}
