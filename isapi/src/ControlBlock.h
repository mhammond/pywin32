#ifndef __CONTROL_BLOCK_H__
#define __CONTROL_BLOCK_H__

// wrapper for the session context (ECB block for IIS)
// If we move away from IIS these will have to change

class CControlBlock
{
public:
	CControlBlock(EXTENSION_CONTROL_BLOCK *pECB) :
		m_pECB(pECB)
	{
		assert(pECB);
	}
	~CControlBlock()
	{
	}
	void Done() {
		assert(m_pECB); // destructed more than once?
		m_pECB = NULL;
	}
	// member retrieval functions
	EXTENSION_CONTROL_BLOCK * GetECB(void)
	{
		assert(m_pECB);
		return m_pECB;
	}

	// wrappers for IIS ECB structures
	void SetStatus(const DWORD status)
	{
		m_pECB->dwHttpStatusCode = status;
	}


	void SetLogMessage(const char *msg)
	{
		strncpy(m_pECB->lpszLogData, msg, HSE_LOG_BUFFER_LEN);
	}

	DWORD WriteStream(char *buffer, const int buffLen, const int reserved=0)
	{
		DWORD dwBufLen = buffLen;	
		m_pECB->WriteClient(m_pECB->ConnID, (void *) buffer, &dwBufLen, reserved);
		return dwBufLen;
	}
	BOOL WriteClient(char *buffer, DWORD *buffLen, const int reserved = 0)
	{
		return m_pECB->WriteClient(m_pECB->ConnID, (void *) buffer, buffLen, reserved);
	}

	bool ReadClient(LPVOID lpvBuffer, LPDWORD lpdwSize)
	{
		return m_pECB->ReadClient(m_pECB->ConnID, lpvBuffer, lpdwSize) ? true : false;
	}

	void DoneWithSession(DWORD dwState)
	{

		// Let IIS know that this worker thread is done with this request.  This will allow
		// IIS to recycle the EXTENSION_CONTROL_BLOCK.  
		m_pECB->ServerSupportFunction(m_pECB->ConnID, HSE_REQ_DONE_WITH_SESSION, &dwState, NULL, 0);
	}

	bool GetServerVariable(char *varName, LPSTR lpBuff, DWORD *pBuffSize)
	{
		BOOL bOK = m_pECB->GetServerVariable(m_pECB->ConnID, varName, lpBuff, pBuffSize);
		if (!bOK)
			*pBuffSize = 0;

		if ((int)(*pBuffSize)>0 && lpBuff[(*pBuffSize)-1]=='\0')
			(*pBuffSize)--;
		return (bOK) ? true : false;
	}

	bool GetImpersonationToken(HANDLE *ret)
	{
		return (m_pECB->ServerSupportFunction)( m_pECB->ConnID,  HSE_REQ_GET_IMPERSONATION_TOKEN, ret, 0,0) ?
			true : false;
	}

	bool TransmitFile(HSE_TF_INFO *info)
	{
		return (m_pECB->ServerSupportFunction)( m_pECB->ConnID,  DWORD HSE_REQ_TRANSMIT_FILE, info, 0,0) ?
			true : false;
	}

	BOOL Redirect(char *url)
	{
		DWORD buffSize = (DWORD)strlen(url);
		BOOL bOK = (m_pECB->ServerSupportFunction)( m_pECB->ConnID,   HSE_REQ_SEND_URL_REDIRECT_RESP, url, &buffSize,0);
		return bOK;
	}

	BOOL MapURLToPath(char *buffer, LPDWORD pSizeofBuffer)
	{
		BOOL bOK = (m_pECB->ServerSupportFunction)(m_pECB->ConnID,
							   HSE_REQ_MAP_URL_TO_PATH,
							   (void *)buffer, pSizeofBuffer,0);
		return bOK;
	}

	BOOL IsKeepConn(LPBOOL bIs)
	{
		BOOL bOK = (m_pECB->ServerSupportFunction)(m_pECB->ConnID,
							   HSE_REQ_IS_KEEP_CONN,
							   (void *)bIs, 0,0);
		return bOK;
	}

	bool IsKeepAlive(void)
	{
		bool bKeepAlive = false;
		char buf[256];
		DWORD bufsize = sizeof(buf)/sizeof(buf[0]);
		if (GetServerVariable("HTTP_CONNECTION",buf, &bufsize)){
			bKeepAlive = _strcmpi(buf, "keep-alive")==0;
		}
		return bKeepAlive;
	}

private:
	EXTENSION_CONTROL_BLOCK * m_pECB;		// IIS control block
};

#endif // __CONTROL_BLOCK_H__
