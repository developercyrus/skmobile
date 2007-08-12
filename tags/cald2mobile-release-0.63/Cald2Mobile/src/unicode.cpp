#include "unicode.h"

using namespace std;

using namespace mysk;

wruntime_error::wruntime_error( const wstring& errorMsg )
: runtime_error("Wide Error Message. Please call errorMsg()")
, mErrorMsg(errorMsg)
{
	// NOTE: We give the runtime_error base the narrow version of the 
	//  error message. This is what will get shown if what() is called.
	//  The wruntime_error inserter or errorMsg() should be used to get 
	//  the wide version.
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

wruntime_error::wruntime_error( const wruntime_error& rhs )
: runtime_error("Wide Error Message. Please call errorMsg()")
, mErrorMsg(rhs.errorMsg())
{
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

wruntime_error&
wruntime_error::operator=( const wruntime_error& rhs )
{
	// copy the wruntime_error
	runtime_error::operator=( rhs ) ; 
	mErrorMsg = rhs.mErrorMsg ; 

	return *this ; 
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

wruntime_error::~wruntime_error()
{
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  */

const wstring& wruntime_error::errorMsg() const { return mErrorMsg ; }


