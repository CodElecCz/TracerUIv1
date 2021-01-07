#include <tsutilCPP.h>

#include "stdafx.h"
#include "ModBusWrapperTS.h"
#include "ModBusWrapper.h"

using namespace Utilities;

inline _bstr_t getValStringExpr(SequenceContext *seqContext, const _bstr_t &string)
{
	PropertyObjectPtr context = seqContext->AsPropertyObject();
	PropertyObjectPtr res = context->EvaluateEx(string, PropOption_NoOptions);
	
	return (context->EvaluateEx(res->GetValString("", 0), PropOption_NoOptions)->GetValString("", 0));
}

inline void setValStringExpr(SequenceContext *seqContext, const _bstr_t &string, const _bstr_t &value)
{
	PropertyObjectPtr context = seqContext->AsPropertyObject();
	PropertyObjectPtr res = context->EvaluateEx(string, PropOption_NoOptions);

	context->SetValString(res->GetValString("", 0), 0, value);
}

void __stdcall DoInit(SequenceContext *seqContext,
	_bstr_t &reportText,
	bool &errorOccurred,
	int &errorCode,
	_bstr_t &errorMsg)
{
	try
	{
		//_bstr_t valString = getValStringExpr(seqContext, "Step.String");

		ModBusWrapperLib::Instance()->Initialize();
	}
	catch (ModBusWrapperException& e)
	{
		errorMsg = e.what();

		errorOccurred = true;
		errorCode = (short)HRESULT_CODE(e.error());
	}
}

void __stdcall DoStart(SequenceContext *seqContext,
						_bstr_t &reportText,
						bool &errorOccurred,
						int &errorCode,
						_bstr_t &errorMsg)
{
	try
	{
		ModBusWrapperLib::Instance()->Start();
	}
	catch (ModBusWrapperException& e)
	{
		errorMsg = e.what();

		errorOccurred = true;
		errorCode = (short)HRESULT_CODE(e.error());
	}
}

void __stdcall DoStop(SequenceContext *seqContext,
	_bstr_t &reportText,
	bool &errorOccurred,
	int &errorCode,
	_bstr_t &errorMsg)
{
	try
	{
		ModBusWrapperLib::Instance()->Stop();
	}
	catch (ModBusWrapperException& e)
	{
		errorMsg = e.what();

		errorOccurred = true;
		errorCode = (short)HRESULT_CODE(e.error());
	}
}

void __stdcall DoRead(SequenceContext *seqContext,
					_bstr_t &reportText,
					bool &errorOccurred,
					int &errorCode,
					_bstr_t &errorMsg)
{
	try
	{
		PropertyObjectPtr step = seqContext->Step->AsPropertyObject();

		_bstr_t valVar = step->GetValString("Variable", 0);
		VARIANT_BOOL wait = step->GetValBoolean("Wait", 0);

		UINT16 val = 0;
		ModBusWrapperLib::Instance()->Read(valVar, &val, (wait>0)? true:false);

		INT16 val16 = static_cast<INT16>(val);
		INT64 val64 = static_cast<INT64>(val16);
		step->SetValInteger64("Number", 0, val64);
	}
	catch (ModBusWrapperException& e)
	{
		errorMsg = e.what();

		errorOccurred = true;
		errorCode = (short)HRESULT_CODE(e.error());
	}
}

void __stdcall DoWrite(SequenceContext *seqContext,
					_bstr_t &reportText,
					bool &errorOccurred,
					int &errorCode,
					_bstr_t &errorMsg)
{
	try
	{
		PropertyObjectPtr step = seqContext->Step->AsPropertyObject();

		_bstr_t valVar = step->GetValString("Variable", 0);
		VARIANT_BOOL wait = step->GetValBoolean("Wait", 0);
		INT64 val64 = step->GetValInteger64("Number", 0);

		INT16 val16 = static_cast<INT16>(val64);
		UINT16 val = static_cast<UINT16>(val16);
		ModBusWrapperLib::Instance()->Write(valVar, val, (wait > 0) ? true : false);
	}
	catch (ModBusWrapperException& e)
	{
		errorMsg = e.what();

		errorOccurred = true;
		errorCode = (short)HRESULT_CODE(e.error());
	}
}