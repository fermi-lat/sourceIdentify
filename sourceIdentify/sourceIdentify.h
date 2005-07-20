/*----------------------------------------------------------------------------*/
/*                              sourceIdentify.h                              */
/* -------------------------------------------------------------------------- */
/* Task            : Source identification header file.                       */
/* Author          : Jurgen Knodlseder CESR (C) (all rights reserved)         */
/* Revision        : 1.0                                                      */
/* Date of version : 25-Feb-2005                                              */
/* -------------------------------------------------------------------------- */
/* History :                                                                  */
/* 1.0   25-Feb-2005  first version                                           */
/*----------------------------------------------------------------------------*/
#ifndef SOURCEIDENTIFY_H
#define SOURCEIDENTIFY_H

/* Definitions ______________________________________________________________ */
#define USE_ST_APP 1

/* Includes _________________________________________________________________ */
#if USE_ST_APP
#include <string>
#include "st_app/AppParGroup.h"
#include "st_app/StApp.h"
#include "st_app/StAppFactory.h"
#include "st_stream/StreamFormatter.h"
#include "st_stream/st_stream.h"
#endif

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
#if USE_ST_APP
//extern st_app::StAppFactory g_factory;
#endif

/* Namespace ends ___________________________________________________________ */
}
#endif // SOURCEIDENTIFY_H
