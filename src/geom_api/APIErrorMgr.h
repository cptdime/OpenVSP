//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

// API.h: interface for the Vehicle Class and Vehicle Mgr Singleton.
// J.R Gloudemans
//
//////////////////////////////////////////////////////////////////////

#if !defined(APIERRORMGR__INCLUDED_)
#define APIERRORMGR__INCLUDED_

#include "APIDefines.h"

#include <string>
#include <stack>
#include <vector>

using std::string;
using std::stack;
using std::vector;

class Vehicle;

namespace vsp
{


//======================== Error Object ================================//
class ErrorObj
{
public:
    ErrorObj();
    ErrorObj( ErrorCode err_code, const string & err_str );

    ErrorCode m_ErrorCode;
    string m_ErrorString;

    void NoError()
    {
        m_ErrorCode = VSP_OK;
        m_ErrorString = "No Error";
    }
};


//======================== Error Mgr ================================//
class ErrorMgrSingleton
{
public:

    bool GetErrorLastCallFlag();                // Did the last call have an error?
    int  GetNumTotalErrors();                   // Total number of errors on stack
    ErrorObj PopLastError();                    // Pop last error off stack
    //ErrorObj GetLastError();                  // Get last error but leave on stack
    bool PopErrorAndPrint( FILE* stream );      // Check for error, pop and print to stream

    void AddError( ErrorCode code, const string & desc );
    void NoError();

    static ErrorMgrSingleton& getInstance()
    {
        static ErrorMgrSingleton instance;
        return instance;
    }

private:

    bool m_ErrorLastCallFlag;
    stack< ErrorObj > m_ErrorStack;

    ErrorMgrSingleton();
    ~ErrorMgrSingleton();
    ErrorMgrSingleton( ErrorMgrSingleton const& copy );          // Not Implemented
    ErrorMgrSingleton& operator=( ErrorMgrSingleton const& copy ); // Not Implemented
};

#define ErrorMgr ErrorMgrSingleton::getInstance()

}

#endif // !defined(APIERRORMGR__INCLUDED_)
