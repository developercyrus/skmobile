#pragma once

#include <string>
#include <sstream>
#include "stdint.h"

namespace mysk
{

#ifdef _UNICODE

	typedef wchar_t tchar;

	typedef std::wstring tstring;
	
	typedef std::wstringstream tstringstream;

#else

	typedef char tchar;

	typedef std::string tstring;

	typedef std::stringstream tstringstream;

#endif // _UNICODE

	class wruntime_error
		: public std::runtime_error
	{

	public:                 // --- PUBLIC INTERFACE ---

		// constructors:
		wruntime_error( const std::wstring& errorMsg ) ;
		// copy/assignment:
		wruntime_error( const wruntime_error& rhs ) ;
		wruntime_error&     operator=( const wruntime_error& rhs ) ;
		// destructor:
		virtual             ~wruntime_error() ;

		// exception methods:
		const std::wstring& errorMsg() const ;

	private:                // --- DATA MEMBERS ---

		// data members:
		std::wstring        mErrorMsg ; ///< Exception error message.

	} ;

#ifdef _UNICODE
	typedef wruntime_error truntime_error;

	inline std::wstringstream&
		operator<<( std::wstringstream& os , const wruntime_error& xcptn )
	{
		// insert the exception
		os << xcptn.errorMsg().c_str() ; 

		return os;
	}

#else 
	typedef std::runtime_error truntime_error;

#endif // _UNICODE


#define THROW_RUNTIME_EXCEPTION(X)			\
	{										\
		tstringstream buf;					\
		buf << _T("[") << __FILE__ << _T(":") << __LINE__ << _T("] - ") << X ; \
		throw truntime_error(buf.str());	\
	}

/* use ATL codec class
	template<typename InputIterator, typename OutputContainer> 
		void utf2uni(InputIterator begin, InputIterator end, OutputContainer & dest)
	{
		for(InputIterator it = begin; it != end; it++) {
			uint8_t byte1 = *it;
			if(0x00 <= byte1 && 0x7F >= byte1 ) {
				wchar_t uni = byte1;
				dest.push_back(uni);
			} else if(0xC0 <= byte1 && 0xDF >= byte1 ) {
				it++;
				uint8_t byte2 = *it;
				wchar_t uni = ((byte1 & 0x1F) << 6) + byte2 & 0x3F;
				dest.push_back(uni);
			} else if(0xE0 <= byte1 && 0xEF >= byte1 ) {
				it++;
				uint8_t byte2 = *it;
				it++;
				uint8_t byte3 = *it;
				wchar_t uni = ((byte1 & 0x0F) << 12) + ((byte2 & 0x3F) << 6) + (byte3 & 0x3F);
				uint8_t out1 = uni & 0xFF;
				uint8_t out2 = (uni & 0xFF00) >> 8;
				uint16_t out = (out1 << 8) + out2;
				wchar_t unile = out;
				dest.push_back(uni);
			} else if(0xF0 <= byte1 && 0xF7 >= byte1 ) {
				it++;
				uint8_t byte2 = *it;
				it++;
				uint8_t byte3 = *it;
				it++;
				uint8_t byte4 = *it;
				wchar_t uni = ((byte1 & 0x07) << 18) + ((byte2 & 0x3F) << 12) + ((byte3 & 0x3F) << 6) + (byte4 & 0x3F);
				dest.push_back(uni);
			}
		}
	}

	inline std::wstring utf2uni(std::string utf8)
	{
		std::wstring unicode;
		utf2uni(utf8.begin(), utf8.end(), unicode);
		return unicode;
	}
*/

}	// namespace mysk

