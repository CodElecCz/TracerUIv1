#include <tsutilCPP.h>
#include <fstream>
#include <iostream>

#include "stdafx.h"
#include "ModBusWrapperTS.h"
#include "ModBusWrapper.h"

//------------------------------------------------------------------------------
// Internal library function prototypes
//------------------------------------------------------------------------------
static _bstr_t ErrorCodeToString(DWORD errorCode);
inline _bstr_t getValStringExpr(SequenceContext *seqContext, const _bstr_t &string);

//------------------------------------------------------------------------------
// Exported library functions 
//------------------------------------------------------------------------------
void __stdcall DoPropertyObjectGetXMLToFile(SequenceContext *seqContext,
									_bstr_t &reportText, 
									bool &errorOccurred, 
									int &errorCode, 
									_bstr_t &errorMsg)
{
	HANDLE hxmlFile = NULL;
	
	try
	{
		PropertyObjectPtr step = seqContext->Step->AsPropertyObject();
		PropertyObjectPtr context = seqContext->AsPropertyObject();

		_bstr_t propertyObjectStr = step->GetValString("PropertyObjectExpr", PropOption_NoOptions);
		_bstr_t xmlFilePath = getValStringExpr(seqContext, "Step.FilePathExpr");

		PropertyObjectPtr obj = context->EvaluateEx(propertyObjectStr, PropOption_NoOptions);

		_bstr_t xmlData = obj->GetXML(XMLOption_UseCRLFInsteadOfLF, 0, "", "");

		hxmlFile = ::CreateFile(xmlFilePath,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL );

		if( hxmlFile != INVALID_HANDLE_VALUE )
		{
			DWORD dwBytes = 0;
			DWORD dwLen = xmlData.length();

			if(!(::WriteFile( hxmlFile,
						(const char*)xmlData,
						dwLen,
                        &dwBytes,
                        NULL )))
			{
				// no data written
				errorMsg = ErrorCodeToString(GetLastError());
				errorOccurred = true;
				errorCode = -1;
			}
		}
		else
		{
			errorMsg = ErrorCodeToString(GetLastError());
			errorOccurred = true;
			errorCode = -1;
		}
	}
	catch (_com_error &com_error)   // The TestStand API only throws this kind of exception 
	{
		errorMsg = com_error.Description();
		errorOccurred = true;
		errorCode = (short)HRESULT_CODE(com_error.Error());
	}

	if (hxmlFile) ::CloseHandle( hxmlFile );
}

void __stdcall DoPropertyObjecSetXMLFromFile(SequenceContext *seqContext,
									_bstr_t &reportText, 
									bool &errorOccurred, 
									int &errorCode, 
									_bstr_t &errorMsg)
{
	HANDLE hxmlFile = NULL;
	BYTE* pbytes	= NULL;
	try
	{
		_bstr_t xmlData;
		PropertyObjectPtr step = seqContext->Step->AsPropertyObject();
		PropertyObjectPtr context = seqContext->AsPropertyObject();

		_bstr_t propertyObjectStr = step->GetValString("PropertyObjectExpr", PropOption_NoOptions);
		_bstr_t xmlFilePath = getValStringExpr(seqContext, "Step.FilePathExpr");

		PropertyObjectPtr obj = context->EvaluateEx(propertyObjectStr, PropOption_NoOptions);

		hxmlFile = ::CreateFile(xmlFilePath,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL );

		if( hxmlFile != INVALID_HANDLE_VALUE )
		{
			DWORD dwBytes = 0;
			DWORD dwSize = ::GetFileSize(hxmlFile, NULL);

			if (dwSize != INVALID_FILE_SIZE)
			{
				BYTE* pbytes = (BYTE*) GlobalAlloc(GPTR, dwSize+1);
				
				if (pbytes != NULL)
				{
					if( ::ReadFile( hxmlFile,
									pbytes,
									dwSize,
									&dwBytes,
									NULL ))
					{
						xmlData = _bstr_t((const char*)pbytes);
						obj->SetXML(xmlData, 0, ConflictHandler_Error);
					}
					else
					{
						// no data written
						errorMsg = ErrorCodeToString(GetLastError());
						errorOccurred = true;
						errorCode = -1;
					}
				}
				else
				{
					errorMsg = ErrorCodeToString(GetLastError());
					errorOccurred = true;
					errorCode = -1;
				}
			}
		}
		else
		{
			errorMsg = ErrorCodeToString(GetLastError());
			errorOccurred = true;
			errorCode = -1;
		}
	}
	catch (_com_error &com_error)   // The TestStand API only throws this kind of exception 
	{
		errorMsg = com_error.Description();
		errorOccurred = true;
		errorCode = (short)HRESULT_CODE(com_error.Error());
	}

	if (pbytes)   ::GlobalFree( (HGLOBAL) pbytes);
	if (hxmlFile) ::CloseHandle( hxmlFile );
}

//------------------------------------------------------------------------------
// Internal library function
//------------------------------------------------------------------------------
static _bstr_t ErrorCodeToString(DWORD errorCode)
{
	_bstr_t result;
	HLOCAL pBuffer;

	::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					0, 
					errorCode, 
					0,
					(LPTSTR)&pBuffer, 
					0, 
					0);

	result = _bstr_t((LPTSTR)pBuffer);

	if (pBuffer != 0)
	{
		::LocalFree(pBuffer);
	}

	return result;
}

inline _bstr_t getValStringExpr(SequenceContext *seqContext, const _bstr_t &string)
{
	PropertyObjectPtr context = seqContext->AsPropertyObject();
	PropertyObjectPtr res = context->EvaluateEx(string, PropOption_NoOptions);
	return (context->EvaluateEx(res->GetValString("", 0), PropOption_NoOptions)->GetValString("", 0));
}