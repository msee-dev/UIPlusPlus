#pragma once

#include <comdef.h>

namespace FTW {

class CScriptHost
{
public:
	CScriptHost(void);
	~CScriptHost(void);

	HRESULT __stdcall Eval(const _TCHAR *source, VARIANT *result);
};

}