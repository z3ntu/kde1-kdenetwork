//=============================================================================
// File:       nntp.cpp
// Contents:   Definitions for DwNntpClient
// Maintainer: Doug Sauder <dwsauder@fwb.gulf.net>
// WWW:        http://www.fwb.gulf.net/~dwsauder/mimepp.html
// $Revision$
// $Date$
//
// Copyright (c) 1996, 1997 Douglas W. Sauder
// All rights reserved.
// 
// IN NO EVENT SHALL DOUGLAS W. SAUDER BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF
// THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF DOUGLAS W. SAUDER
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// DOUGLAS W. SAUDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT
// NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
// BASIS, AND DOUGLAS W. SAUDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
// SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
//=============================================================================

#define DW_IMPLEMENTATION

#include <mimelib/config.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <mimelib/nntp.h>

#define NNTP_PORT 119
#define RECV_BUFFER_SIZE  8192
#define SEND_BUFFER_SIZE  1024

#if defined(DW_DEBUG_NNTP)
#  define DBG_NNTP_STMT(x) x
#else
#  define DBG_NNTP_STMT(x)
#endif


// To do: create a private Initialize() member function that is called from
// the constructor and from Open() and that initializes the state.

DwNntpClient::DwNntpClient()
{
    mSendBuffer = new char[SEND_BUFFER_SIZE];
    mRecvBuffer = new char[RECV_BUFFER_SIZE];
    mLastChar = -1;
    mLastLastChar = -1;
    mNumRecvBufferChars = 0;
    mRecvBufferPos = 0;
    mReplyCode = 0;
    mObserver = 0;
}


DwNntpClient::~DwNntpClient()
{
    if (mRecvBuffer) {
        delete [] mRecvBuffer;
        mRecvBuffer = 0;
    }
    if (mSendBuffer) {
        delete [] mSendBuffer;
        mSendBuffer = 0;
    }
}


int DwNntpClient::Open(const char* aServer, DwUint16 aPort)
{
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    mReplyCode = 0;
    int err = DwProtocolClient::Open(aServer, aPort);
    if (! err) {
        PGetSingleLineResponse();
    }
    return mReplyCode;
}


DwObserver* DwNntpClient::SetObserver(DwObserver* aObserver)
{
    DwObserver* obs = mObserver;
    mObserver = aObserver;
    return obs;
}


int DwNntpClient::ReplyCode() const
{
    return mReplyCode;
}


const DwString& DwNntpClient::SingleLineResponse() const
{
    return mSingleLineResponse;
}


const DwString& DwNntpClient::MultiLineResponse() const
{
    return mMultiLineResponse;
}


int DwNntpClient::Article(int aArticleNum)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    if (aArticleNum >= 0) {
        sprintf(mSendBuffer, "ARTICLE %d\r\n", aArticleNum);
    }
    else {
        strcpy(mSendBuffer, "ARTICLE\r\n");
    }
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mReplyCode/100%10 == 2) {
            PGetMultiLineResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Article(const char* aMsgId)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    if (!aMsgId || !*aMsgId) {
        // error!
        return mReplyCode;
    }
    strcpy(mSendBuffer, "ARTICLE ");
    strncat(mSendBuffer, aMsgId, 80);
    strcat(mSendBuffer, "\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mReplyCode/100%10 == 2) {
            PGetMultiLineResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Head(int aArticleNum)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    if (aArticleNum >= 0) {
        sprintf(mSendBuffer, "HEAD %d\r\n", aArticleNum);
    }
    else {
        strcpy(mSendBuffer, "HEAD\r\n");
    }
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mReplyCode/100%10 == 2) {
            PGetMultiLineResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Head(const char* aMsgId)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    if (!aMsgId || !*aMsgId) {
        return mReplyCode;
    }
    strcpy(mSendBuffer, "HEAD ");
    strncat(mSendBuffer, aMsgId, 80);
    strcat(mSendBuffer, "\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mReplyCode/100%10 == 2) {
            PGetMultiLineResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Body(int articleNum)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    if (articleNum >= 0) {
        sprintf(mSendBuffer, "BODY %d\r\n", articleNum);
    }
    else {
        strcpy(mSendBuffer, "BODY\r\n");
    }
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mReplyCode/100%10 == 2) {
            PGetMultiLineResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Body(const char* aMsgId)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    if (!aMsgId || !*aMsgId) {
        return mReplyCode;
    }
    strcpy(mSendBuffer, "BODY ");
    strncat(mSendBuffer, aMsgId, 80);
    strcat(mSendBuffer, "\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mReplyCode/100%10 == 2) {
            PGetMultiLineResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Stat(int articleNum)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    if (articleNum >= 0) {
        sprintf(mSendBuffer, "STAT %d\r\n", articleNum);
    }
    else {
        strcpy(mSendBuffer, "STAT\r\n");
    }
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Stat(const char* aMsgId)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    if (!aMsgId || !*aMsgId) {
        return mReplyCode;
    }
    strcpy(mSendBuffer, "STAT ");
    strncat(mSendBuffer, aMsgId, 80);
    strcat(mSendBuffer, "\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Group(const char* aNewsgroupName)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    sprintf(mSendBuffer, "GROUP %s\r\n", aNewsgroupName);
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Help()
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    strcpy(mSendBuffer, "HELP\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mReplyCode/100%10 == 1) {
            PGetMultiLineResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Last()
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    strcpy(mSendBuffer, "LAST\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mReplyCode;
}


int DwNntpClient::List()
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    strcpy(mSendBuffer, "LIST\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mReplyCode/100%10 == 2) {
            PGetMultiLineResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Newgroups(const char* aDate, const char* aTime,
    DwBool aIsGmt, const char* aDistribution)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    strcpy(mSendBuffer, "NEWGROUPS ");
    strcat(mSendBuffer, aDate);
    strcat(mSendBuffer, " ");
    strcat(mSendBuffer, aTime);
    if (aIsGmt) {
        strcat(mSendBuffer, " GMT");
    }
    if (aDistribution) {
        strcat(mSendBuffer, " ");
        strcat(mSendBuffer, aDistribution);
    }
    strcat(mSendBuffer, "\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mReplyCode/100%10 == 2) {
            PGetMultiLineResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Newnews(const char* aNewsgroups, const char* aDate,
    const char* aTime, DwBool aIsGmt, const char* aDistribution)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    strcpy(mSendBuffer, "NEWNEWS ");
    strcat(mSendBuffer, aNewsgroups);
    strcat(mSendBuffer, " ");
    strcat(mSendBuffer, aDate);
    strcat(mSendBuffer, " ");
    strcat(mSendBuffer, aTime);
    if (aIsGmt) {
        strcat(mSendBuffer, " GMT");
    }
    if (aDistribution) {
        strcat(mSendBuffer, " ");
        strcat(mSendBuffer, aDistribution);
    }
    strcat(mSendBuffer, "\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
        if (mReplyCode/100%10 == 2) {
            PGetMultiLineResponse();
        }
    }
    return mReplyCode;
}


int DwNntpClient::Next()
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    strcpy(mSendBuffer, "NEXT\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Post()
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    strcpy(mSendBuffer, "POST\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Quit()
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    strcpy(mSendBuffer, "QUIT\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mReplyCode;
}


int DwNntpClient::Slave()
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();
    strcpy(mSendBuffer, "SLAVE\r\n");
    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
    int bufferLen = strlen(mSendBuffer);
    int numSent = PSend(mSendBuffer, bufferLen);
    if (numSent == bufferLen) {
        PGetSingleLineResponse();
    }
    return mReplyCode;
}


//int DwNntpClient::Date()
//{
//    mReplyCode = 0;
//    mSingleLineResponse.clear();
//    mMultiLineResponse.clear();
//    strcpy(mSendBuffer, "DATE\r\n");
//    DBG_NNTP_STMT(cout << "C: " << mSendBuffer << endl;)
//    int bufferLen = strlen(mSendBuffer);
//    int numSent = PSend(mSendBuffer, bufferLen);
//    if (numSent == bufferLen) {
//        PGetSingleLineResponse();
//    }
//    return mReplyCode;
//}


int DwNntpClient::SendData(const DwString& aStr)
{
    return SendData(aStr.data(), aStr.length());
}


int DwNntpClient::SendData(const char* aBuf, int aBufLen)
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    mMultiLineResponse.clear();

    int pos = 0;
    int len = 0;
    const char* buf = 0;

    int lastLastChar = '\r';
    int lastChar = '\n';

    while (1) {

        len = SEND_BUFFER_SIZE;
        len = (len < aBufLen - pos) ? len : aBufLen - pos;
        if (len == 0) break;

        // Look for CR LF '.'.  If it is found, then we have to copy the buffer
        // and stuff an extra '.'.

        int hasCrLfDot = 0;
        int tLastChar = lastChar;
        int tLastLastChar = lastLastChar;
        for (int i=0; i < len; ++i) {
            int ch = aBuf[pos+i];
            if (tLastLastChar == '\r' && tLastChar == '\n' && ch == '.') {
                hasCrLfDot = 1;
                break;
            }
            tLastLastChar = tLastChar;
            tLastChar = ch;
        }
        if (! hasCrLfDot) {
            lastChar = tLastChar;
            lastLastChar = tLastLastChar;
            buf = &aBuf[pos];
            pos += len;
        }

        // If CR LF '.' was found, copy the chars to a different buffer and stuff
        // the extra '.'.

        else /* (hasCrLfDot) */ {
            tLastChar = lastChar;
            tLastLastChar = lastLastChar;
            int iDst = 0;
            int iSrc = 0;
            // Implementation note: be careful to avoid overrunning the
            // destination buffer when CR LF '.' are the last three characters
            // of the source buffer.
            while (iDst < SEND_BUFFER_SIZE && iSrc < len) {
                int ch = aBuf[pos+iSrc];
                if (tLastLastChar == '\r' && tLastChar == '\n' && ch == '.') {
                    if (iDst == SEND_BUFFER_SIZE-1) {
                        break;
                    }
                    mSendBuffer[iDst++] = '.';
                }
                mSendBuffer[iDst++] = ch;
                ++iSrc;
                tLastLastChar = tLastChar;
                tLastChar = ch;
            }
            lastChar = tLastChar;
            lastLastChar = tLastLastChar;
            len = iDst;
            buf = mSendBuffer;
            pos += iSrc;
        }

        // Send the buffer

        int numSent = PSend(buf, len);
        if (numSent != len) {
            mReplyCode = 0;
            return mReplyCode;
        }
    }

    // Send final '.' CR LF.  If CR LF are not at the end of the buffer, then
    // send a CR LF '.' CR LF.

    if (lastLastChar == '\r' && lastChar == '\n') {
        PSend(".\r\n", 3);
    }
    else {
        PSend("\r\n.\r\n", 5);
    }

    // Get the server's response

    PGetSingleLineResponse();
    return mReplyCode;
}


void DwNntpClient::PGetSingleLineResponse()
{
    mReplyCode = 0;
    mSingleLineResponse.clear();
    char* ptr;
    int len;
    int err = PGetLine(&ptr, &len);
    if (! err) {
        mReplyCode = strtol(ptr, NULL, 10);
        mSingleLineResponse.assign(ptr, len);
        DBG_NNTP_STMT(char buffer[256];)
        DBG_NNTP_STMT(strncpy(buffer, ptr, len);)
        DBG_NNTP_STMT(buffer[len] = 0;)
        DBG_NNTP_STMT(cout << "S: " << buffer;)
    }
}


void DwNntpClient::PGetMultiLineResponse()
{
    mMultiLineResponse.clear();

    // Get a line at a time until we get CR LF . CR LF

    while (1) {
        char* ptr;
        int len;
        int err = PGetLine(&ptr, &len);

        // Check for an error

        if (err) {
            mReplyCode = 0;
            return;
        }

        // Check for '.' on a line by itself, which indicates end of multiline
        // response

        if (len >= 3 && ptr[0] == '.' && ptr[1] == '\r' && ptr[2] == '\n') {
            break;
        }

        // Remove '.' at beginning of line

        if (*ptr == '.') ++ptr;

        // If an observer is assigned, notify it.
        // Implementation note: An observer is assumed to fetch the multiline
        // response one line at a time, therefore we assign to the string,
        // rather than append to it.

        if (mObserver) {
            mMultiLineResponse.assign(ptr, len);
            mObserver->Notify();
        }
        else {
            mMultiLineResponse.append(ptr, len);
        }
    }
}


int DwNntpClient::PGetLine(char** aPtr, int* aLen)
{
    // Restore the saved state

    int startPos = mRecvBufferPos;
    int pos = mRecvBufferPos;
    int lastChar = -1;

    // Keep trying until we get a complete line, detect an error, or determine that
    // the connection has been closed

    int isEndOfLineFound = 0;
    while (1) {

        // Search buffer for end of line chars. Stop when we find them or when
        // we exhaust the buffer.

        while (pos < mNumRecvBufferChars) {
            if (lastChar == '\r' && mRecvBuffer[pos] == '\n') {
                isEndOfLineFound = 1;
                ++pos;
                break;
            }
            lastChar = mRecvBuffer[pos];
            ++pos;
        }
        if (isEndOfLineFound) {
            *aPtr = &mRecvBuffer[startPos];
            *aLen = pos - startPos;
            mRecvBufferPos = pos;
            return 0;
        }

        // Replenish the buffer

        memmove(mRecvBuffer, &mRecvBuffer[startPos], mNumRecvBufferChars-startPos);
        mNumRecvBufferChars -= startPos;
        mRecvBufferPos = mNumRecvBufferChars;
        int bufLen = RECV_BUFFER_SIZE - mRecvBufferPos;
        int n = PReceive(&mRecvBuffer[mRecvBufferPos], bufLen);
        if (n == 0) {
            // The connection has been closed or an error occurred
            return -1;
        }
        mNumRecvBufferChars += n;
        startPos = 0;
        pos = mRecvBufferPos;
    }
}

