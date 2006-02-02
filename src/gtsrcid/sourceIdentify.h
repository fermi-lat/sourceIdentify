/*------------------------------------------------------------------------------
Id ........: $Id: sourceIdentify.h,v 1.3 2006/02/01 13:33:37 jurgen Exp $
Author ....: $Author: jurgen $
Revision ..: $Revision: 1.3 $
Date ......: $Date: 2006/02/01 13:33:37 $
--------------------------------------------------------------------------------
$Log: sourceIdentify.h,v $
Revision 1.3  2006/02/01 13:33:37  jurgen
Tried to fix Win32 compilation bugs.
Change revision number to 1.3.2.
Replace header information with CVS typeset information.

------------------------------------------------------------------------------*/
#ifndef SOURCEIDENTIFY_H
#define SOURCEIDENTIFY_H

/* Definitions ______________________________________________________________ */
#define TOOL_NAME     "gtsrcid"
#define TOOL_VERSION  "gtsrcid-v0r5"
#define TOOL_LOGFILE  "gtsrcid.log"
#define USE_ST_APP 1

/* Includes _________________________________________________________________ */
#include <string>
#include "st_app/AppParGroup.h"
#include "st_app/StApp.h"
#include "st_app/StAppFactory.h"
#include "st_stream/StreamFormatter.h"
#include "st_stream/st_stream.h"

/* Architecture specific definitions ________________________________________ */
#ifndef EXPSYM
#ifdef WIN32
#define EXPSYM __declspec(dllexport)
#else
#define EXPSYM
#endif
#endif


/* Namespace definition _____________________________________________________ */
namespace sourceIdentify {


/* Type defintions __________________________________________________________ */
typedef enum  {                                  // Function status code
  STATUS_OK                =       0,             // Function status ok
  STATUS_MEM_ALLOC         = -100000,             // Memory allocation failure
  STATUS_LOG_OPEN_FAILED   = -100100,             // Log file open error
  STATUS_LOG_CLOSE_FAILED  = -100101,             // Log file close error
  STATUS_LOG_WRITE_FAILED  = -100102,             // Log file write error
  STATUS_PAR_BAD_PARAMETER = -100200,             // Bad task parameter
  STATUS_CAT_NOT_FOUND     = -100300,             // Catalogue not found
  STATUS_CAT_EMPTY         = -100301              // Catalogue empty
} Status;

/* Prototypes _______________________________________________________________ */

/* Globals __________________________________________________________________ */

/* Namespace ends ___________________________________________________________ */
}
#endif // SOURCEIDENTIFY_H
