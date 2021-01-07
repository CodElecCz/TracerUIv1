#pragma once

#include <tsapivc.h>

using namespace TS;
using namespace TSUI;

//------------------------------------------------------------------------------
// Exported library function prototypes
//------------------------------------------------------------------------------

LIB_API void __stdcall DoInit(SequenceContext *seqContext,
							_bstr_t &reportText,
							bool &errorOccurred,
							int &errorCode,
							_bstr_t &errorMsg);

LIB_API void __stdcall DoStart(SequenceContext *seqContext,
							_bstr_t &reportText,
							bool &errorOccurred,
							int &errorCode,
							_bstr_t &errorMsg);

/*
	DoRead()
	Step.Variable	... String
	Step.Number		... Number
	Step.Wait		... Bool
*/
LIB_API void __stdcall DoRead(SequenceContext *seqContext,
							_bstr_t &reportText,
							bool &errorOccurred,
							int &errorCode,
							_bstr_t &errorMsg);

/*
	DoWrite()
	Step.Variable	... String
	Step.Number		... Number
	Step.Wait		... Bool
*/
LIB_API void __stdcall DoWrite(SequenceContext *seqContext,
							_bstr_t &reportText,
							bool &errorOccurred,
							int &errorCode,
							_bstr_t &errorMsg);

LIB_API void __stdcall DoStop(SequenceContext *seqContext,
							_bstr_t &reportText,
							bool &errorOccurred,
							int &errorCode,
							_bstr_t &errorMsg);


LIB_API void __stdcall DoPropertyObjectGetXMLToFile(SequenceContext *seqContext,
							_bstr_t &reportText,
							bool &errorOccurred,
							int &errorCode,
							_bstr_t &errorMsg);


LIB_API void __stdcall DoPropertyObjecSetXMLFromFile(SequenceContext *seqContext,
							_bstr_t &reportText,
							bool &errorOccurred,
							int &errorCode,
							_bstr_t &errorMsg);
