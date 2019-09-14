/*
 ======================================================================
 Copyright 2002-2003 by Blackdog Software Pty Ltd.

                         All Rights Reserved

 Permission to use, copy, modify, and distribute this software and
 its documentation for any purpose and without fee is hereby
 granted, provided that the above copyright notice appear in all
 copies and that both that copyright notice and this permission
 notice appear in supporting documentation, and that the name of
 Blackdog Software not be used in advertising or publicity pertaining to
 distribution of the software without specific, written prior
 permission.

 BLACKDOG SOFTWARE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 NO EVENT SHALL BLACKDOG SOFTWARE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ======================================================================
 */

#ifndef __UTILS_H
#define __UTILS_H

// ---------------------------------------------------------------------------
// Class: CSLock
// Locking class which handles the serialisation of objects using CSingleLock.
//
class CSLock {
   public:
    inline CSLock(CRITICAL_SECTION &sem) : m_Lock(sem) { ::EnterCriticalSection(&m_Lock); }
    ~CSLock() { ::LeaveCriticalSection(&m_Lock); }

   private:
    CRITICAL_SECTION &m_Lock;
};  // CSLock

// Formats a system error code

char *FormatSysError(const DWORD nErrNo);

// Dump out an HTML error response page
char *HTMLErrorResp(const char *msg);

// returns the pathname of this module
TCHAR *GetModulePath(void);

// Write entry to the event log
extern "C" BOOL WriteEventLogMessage(WORD eventType, DWORD eventID, WORD num_inserts, const char **inserts);

#endif  // __UTILS_H
