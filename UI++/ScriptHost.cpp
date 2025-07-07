#include "StdAfx.h"
#include "ScriptHost.h"
#include "FTW\FTWError.h"
#include <comdef.h>

namespace FTW {

CScriptHost::CScriptHost(void) : m_activeScriptParse (0), m_activeScriptSite(0), m_activeScript(0)
{
	// PowerShell-based script host initialization
	// No COM scripting engine needed - we'll execute PowerShell directly
}


CScriptHost::~CScriptHost(void)
{
	// PowerShell-based script host - no COM objects to clean up
}

// Translate VBScript expressions to PowerShell equivalents
CString TranslateVBScriptToPowerShell(const _TCHAR* vbExpression)
{
    CString psExpression(vbExpression);
    
    // Replace VBScript string comparison operators
    psExpression.Replace(_T(" = "), _T(" -eq "));
    psExpression.Replace(_T(" <> "), _T(" -ne "));
    psExpression.Replace(_T(" > "), _T(" -gt "));
    psExpression.Replace(_T(" < "), _T(" -lt "));
    psExpression.Replace(_T(" >= "), _T(" -ge "));
    psExpression.Replace(_T(" <= "), _T(" -le "));
    
    // Replace VBScript logical operators
    psExpression.Replace(_T(" AND "), _T(" -and "));
    psExpression.Replace(_T(" OR "), _T(" -or "));
    psExpression.Replace(_T(" NOT "), _T(" -not "));
    psExpression.Replace(_T("And "), _T("-and "));
    psExpression.Replace(_T("Or "), _T("-or "));
    psExpression.Replace(_T("Not "), _T("-not "));
    
    // Handle concatenation
    psExpression.Replace(_T(" & "), _T(" + "));
    
    // Replace VBScript string functions with PowerShell equivalents
    // Len() -> .Length
    int pos = 0;
    while ((pos = psExpression.Find(_T("Len("), pos)) != -1)
    {
        int endPos = pos + 4; // Start after "Len("
        int parenCount = 1;
        while (endPos < psExpression.GetLength() && parenCount > 0)
        {
            if (psExpression[endPos] == _T('('))
                parenCount++;
            else if (psExpression[endPos] == _T(')'))
                parenCount--;
            endPos++;
        }
        
        if (parenCount == 0)
        {
            CString content = psExpression.Mid(pos + 4, endPos - pos - 5);
            CString replacement;
            replacement.Format(_T("(%s).Length"), (LPCTSTR)content);
            psExpression = psExpression.Left(pos) + replacement + psExpression.Mid(endPos);
            pos += replacement.GetLength();
        }
        else
        {
            pos++;
        }
    }
    
    // Left() -> .Substring(0, n)
    pos = 0;
    while ((pos = psExpression.Find(_T("Left("), pos)) != -1)
    {
        int endPos = pos + 5; // Start after "Left("
        int parenCount = 1;
        int commaPos = -1;
        
        while (endPos < psExpression.GetLength() && parenCount > 0)
        {
            if (psExpression[endPos] == _T('('))
                parenCount++;
            else if (psExpression[endPos] == _T(')'))
                parenCount--;
            else if (psExpression[endPos] == _T(',') && parenCount == 1 && commaPos == -1)
                commaPos = endPos;
            endPos++;
        }
        
        if (parenCount == 0 && commaPos != -1)
        {
            CString string_part = psExpression.Mid(pos + 5, commaPos - pos - 5);
            CString length_part = psExpression.Mid(commaPos + 1, endPos - commaPos - 2);
            string_part.Trim();
            length_part.Trim();
            
            CString replacement;
            replacement.Format(_T("(%s).Substring(0, %s)"), (LPCTSTR)string_part, (LPCTSTR)length_part);
            psExpression = psExpression.Left(pos) + replacement + psExpression.Mid(endPos);
            pos += replacement.GetLength();
        }
        else
        {
            pos++;
        }
    }
    
    // CDbl() -> [double]
    psExpression.Replace(_T("CDbl("), _T("([double]("));
    
    // InStr() -> .IndexOf() + 1 (VBScript is 1-based, .NET is 0-based)
    pos = 0;
    while ((pos = psExpression.Find(_T("InStr("), pos)) != -1)
    {
        int endPos = pos + 6; // Start after "InStr("
        int parenCount = 1;
        int commaPos = -1;
        
        while (endPos < psExpression.GetLength() && parenCount > 0)
        {
            if (psExpression[endPos] == _T('('))
                parenCount++;
            else if (psExpression[endPos] == _T(')'))
                parenCount--;
            else if (psExpression[endPos] == _T(',') && parenCount == 1 && commaPos == -1)
                commaPos = endPos;
            endPos++;
        }
        
        if (parenCount == 0 && commaPos != -1)
        {
            CString string_part = psExpression.Mid(pos + 6, commaPos - pos - 6);
            CString search_part = psExpression.Mid(commaPos + 1, endPos - commaPos - 2);
            string_part.Trim();
            search_part.Trim();
            
            CString replacement;
            replacement.Format(_T("((%s).IndexOf(%s) + 1)"), (LPCTSTR)string_part, (LPCTSTR)search_part);
            psExpression = psExpression.Left(pos) + replacement + psExpression.Mid(endPos);
            pos += replacement.GetLength();
        }
        else
        {
            pos++;
        }
    }
    
    return psExpression;
}

HRESULT __stdcall CScriptHost::Eval(const _TCHAR *source, VARIANT *result)
{
    ASSERT(source != NULL);

    if (source == NULL)
        return E_POINTER;

    if (result == NULL)
        return E_POINTER;

    // Initialize result
    VariantInit(result);

    try
    {
        // Translate VBScript expression to PowerShell
        CString psExpression = TranslateVBScriptToPowerShell(source);
        
        // Wrap the expression to ensure it evaluates properly
        CString command;
        command.Format(_T("powershell.exe -NoProfile -NonInteractive -Command \"try { $result = %s; if ($result -is [bool]) { if ($result) { 'True' } else { 'False' } } elseif ($result -ne $null) { $result.ToString() } else { '' } } catch { 'False' }\""), 
                      (LPCTSTR)psExpression);

        // Execute PowerShell command
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;

        HANDLE hStdoutRead, hStdoutWrite;
        if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0))
        {
            return E_FAIL;
        }

        SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);

        PROCESS_INFORMATION pi;
        STARTUPINFO si;
        ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
        ZeroMemory(&si, sizeof(STARTUPINFO));
        si.cb = sizeof(STARTUPINFO);
        si.hStdError = hStdoutWrite;
        si.hStdOutput = hStdoutWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;

        if (!CreateProcess(NULL, command.GetBuffer(), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        {
            CloseHandle(hStdoutRead);
            CloseHandle(hStdoutWrite);
            return E_FAIL;
        }

        CloseHandle(hStdoutWrite);

        // Wait for process to complete
        WaitForSingleObject(pi.hProcess, 10000); // 10 second timeout

        // Read output
        DWORD bytesRead;
        char buffer[1024];
        CStringA output;
        
        while (ReadFile(hStdoutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        CloseHandle(hStdoutRead);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        // Parse result
        output.Trim();
        
        if (output.CompareNoCase("True") == 0)
        {
            result->vt = VT_BOOL;
            result->boolVal = VARIANT_TRUE;
        }
        else if (output.CompareNoCase("False") == 0)
        {
            result->vt = VT_BOOL;
            result->boolVal = VARIANT_FALSE;
        }
        else
        {
            // Try to parse as number
            char* endPtr;
            double numValue = strtod(output, &endPtr);
            if (*endPtr == '\0' && endPtr != (LPCSTR)output)
            {
                result->vt = VT_R8;
                result->dblVal = numValue;
            }
            else
            {
                // Return as string
                CString wideOutput(output);
                result->vt = VT_BSTR;
                result->bstrVal = SysAllocString(wideOutput);
            }
        }

        return S_OK;
    }
    catch (...)
    {
        result->vt = VT_BOOL;
        result->boolVal = VARIANT_FALSE;
        return E_FAIL;
    }
}

}