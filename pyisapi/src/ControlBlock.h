#ifndef __CONTROL_BLOCK_H__
#define __CONTROL_BLOCK_H__

// wrapper for the session context (ECB block for IIS)
// If we move away from IIS these will have to change

class CControlBlock
{
public:
	CControlBlock(EXTENSION_CONTROL_BLOCK *pECB=NULL) :
		m_pECB(pECB)
	{
	}
	
	~CControlBlock()
	{
	}

	CControlBlock(const CControlBlock & rhs)
	{
		Copy(rhs);
	}

	CControlBlock & operator=(const CControlBlock & rhs)
	{
		if (this != &rhs)
			Copy(rhs);
		return *this;
	}

	// member retrieval functions


	EXTENSION_CONTROL_BLOCK * GetECB(void)
	{
		return m_pECB;
	}

	// wrappers for IIS ECB structures
	void SetStatus(const DWORD status)
	{
		m_pECB->dwHttpStatusCode = status;
	}


	void SetLogMessage(LPCTSTR msg)
	{
		strncpy(m_pECB->lpszLogData, msg, HSE_LOG_BUFFER_LEN);
	}

	DWORD WriteHeaders(LPCTSTR szStatus, LPCTSTR szHeader, const bool bKeepAlive=true)
	{
		//  NOTE we must send Content-Length header with correct byte count
		//  in order for keep-alive to work, the bKeepAlive flag is not enough
		//  by itself..

		HSE_SEND_HEADER_EX_INFO  SendHeaderExInfo;
	    DWORD cchStatus = lstrlen(szStatus);
		DWORD cchHeader = lstrlen(szHeader);

		//  Populate SendHeaderExInfo struct
	    SendHeaderExInfo.pszStatus = szStatus;
		SendHeaderExInfo.pszHeader = szHeader;
		SendHeaderExInfo.cchStatus = cchStatus;
		SendHeaderExInfo.cchHeader = cchHeader;
        SendHeaderExInfo.fKeepConn = (bKeepAlive) ? TRUE:FALSE;

		//  Send header
		return m_pECB->ServerSupportFunction(m_pECB->ConnID, HSE_REQ_SEND_RESPONSE_HEADER_EX, &SendHeaderExInfo, NULL,NULL);
	}

	DWORD WriteStream(LPCTSTR buffer, const int buffLen, const int reserved=0)
	{
		DWORD dwBufLen = buffLen;	
		m_pECB->WriteClient(m_pECB->ConnID, (void *) buffer, &dwBufLen, reserved);
		return dwBufLen;
	}

	bool ReadClient(LPVOID lpvBuffer, LPDWORD lpdwSize)
	{
		return m_pECB->ReadClient(m_pECB->ConnID, lpvBuffer, lpdwSize) ? true : false;
	}

	void SignalAsyncRequestDone(const bool bKeepAlive=true)
	{

		// Let IIS know that this worker thread is done with this request.  This will allow
		// IIS to recycle the EXTENSION_CONTROL_BLOCK.  
		DWORD dwState = (bKeepAlive) ? HSE_STATUS_SUCCESS_AND_KEEP_CONN : HSE_STATUS_SUCCESS;
		m_pECB->ServerSupportFunction(m_pECB->ConnID, HSE_REQ_DONE_WITH_SESSION, &dwState, NULL, 0);
	}

	bool GetServerVariable(LPCTSTR varName, LPSTR lpBuff, DWORD *pBuffSize)
	{
		BOOL bOK = m_pECB->GetServerVariable(m_pECB->ConnID,(LPSTR) varName,lpBuff,pBuffSize);
		if (!bOK)
			*pBuffSize = 0;

		if (lpBuff[(*pBuffSize)-1]=='\0')
			(*pBuffSize)--;
		return (bOK) ? true : false;
	}

	bool GetImpersonationToken(HANDLE *ret)
	{
		return (m_pECB->ServerSupportFunction)( m_pECB->ConnID,  HSE_REQ_GET_IMPERSONATION_TOKEN, ret, 0,0) ?
			true : false;
	}

	BOOL Redirect(LPCTSTR url)
	{
		DWORD buffSize = strlen(url);
		BOOL bOK = (m_pECB->ServerSupportFunction)( m_pECB->ConnID,   HSE_REQ_SEND_URL_REDIRECT_RESP, (LPSTR) url, &buffSize,0);
		return bOK;
	}

	bool IsKeepAlive(void)
	{
		bool bKeepAlive = false;
		char buf[256];
		DWORD bufsize = sizeof(buf)/sizeof(buf[0]);
		if (GetServerVariable("HTTP_CONNECTION",buf, &bufsize)){
			bKeepAlive = strcmpi(buf, "keep-alive")==0;
		}
		return bKeepAlive;
	}

private:
	EXTENSION_CONTROL_BLOCK * m_pECB;		// IIS control block
private:

	void Copy(const CControlBlock & rhs)
	{
		m_pECB = rhs.m_pECB;
	}
};

#endif // __CONTROL_BLOCK_H__