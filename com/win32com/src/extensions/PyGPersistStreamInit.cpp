#include "stdafx.h"
#include "PyIPersistStreamInit.h"
#include "PyIStream.h"

STDMETHODIMP PyGPersistStreamInit::InitNew(void)
{
    PY_GATEWAY_METHOD;
    return InvokeViaPolicy("InitNew");
}
