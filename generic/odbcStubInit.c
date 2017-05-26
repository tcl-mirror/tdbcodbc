/*
 * odbcStubInit.c --
 *
 *	Stubs tables for the foreign ODBC libraries so that
 *	Tcl extensions can use them without the linker's knowing about them.
 *
 * @CREATED@ 2017-05-17 13:54:37Z by genExtStubs.tcl from ../generic/odbcStubDefs.txt
 *
 * Copyright (c) 2010 by Kevin B. Kenny.
 *
 * Please refer to the file, 'license.terms' for the conditions on
 * redistribution of this file and for a DISCLAIMER OF ALL WARRANTIES.
 *
 *-----------------------------------------------------------------------------
 */

#include <tcl.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "fakesql.h"

/*
 * Static data used in this file
 */

/*
 * Names of the libraries that might contain the ODBC API
 */


/* Uncomment or -DTDBC_NEW_LOADER=1 to use the new loader */
/* #define TDBC_NEW_LOADER 1*/


#ifdef TDBC_NEW_LOADER

/* Sorted by name asc. */
static const char *const odbcStubLibNames[] = {
    "iodbc", "odbc", "odbc32", NULL
};
/* Sorted by num desc. No leading dots. Empty first. */
static const char *const odbcStubLibNumbers[] = {
	"", "1.2", "0.1", NULL
};
/* Sorted by name asc. */
static const char *const odbcOptLibNames[] = {
    "iodbcinst", "odbccp", "odbccp32", "odbcinst", NULL
};
/* Sorted by num desc. No leading dots. Empty first. */
static const char *const odbcOptLibNumbers[] = {
	"", "2.6", "0.0", NULL
};

#else

static const char *const odbcStubLibNames[] = {
    /* @LIBNAMES@: DO NOT EDIT THESE NAMES */
    "odbc32", "odbc", "libodbc32", "libodbc", "libiodbc", NULL
    /* @END@ */
};
static const char *const odbcOptLibNames[] = {
    "odbccp", "odbccp32", "odbcinst",
    "libodbccp", "libodbccp32", "libodbcinst", "libiodbcinst", NULL
};

#endif


/*
 * Names of the functions that we need from ODBC
 */

static const char *const odbcSymbolNames[] = {
    /* @SYMNAMES@: DO NOT EDIT THESE NAMES */
    "SQLAllocHandle",
    "SQLBindParameter",
    "SQLCloseCursor",
    "SQLColumnsW",
    "SQLDataSourcesW",
    "SQLDescribeColW",
    "SQLDescribeParam",
    "SQLDisconnect",
    "SQLDriverConnectW",
    "SQLDriversW",
    "SQLEndTran",
    "SQLExecute",
    "SQLFetch",
    "SQLForeignKeysW",
    "SQLFreeHandle",
    "SQLGetConnectAttr",
    "SQLGetData",
    "SQLGetDiagFieldA",
    "SQLGetDiagRecW",
    "SQLGetTypeInfo",
    "SQLMoreResults",
    "SQLNumParams",
    "SQLNumResultCols",
    "SQLPrepareW",
    "SQLPrimaryKeysW",
    "SQLRowCount",
    "SQLSetConnectAttr",
    "SQLSetConnectOption",
    "SQLSetEnvAttr",
    "SQLTablesW",
    NULL
    /* @END@ */
};

/*
 * Table containing pointers to the functions named above.
 */

static odbcStubDefs odbcStubsTable;
const odbcStubDefs* odbcStubs = &odbcStubsTable;

/*
 * Pointers to optional functions in ODBCINST
 */

BOOL (INSTAPI* SQLConfigDataSourceW)(HWND, WORD, LPCWSTR, LPCWSTR)
= NULL;
BOOL (INSTAPI* SQLConfigDataSource)(HWND, WORD, LPCSTR, LPCSTR)
= NULL;
BOOL (INSTAPI* SQLInstallerError)(WORD, DWORD*, LPSTR, WORD, WORD*)
= NULL;


#ifdef TDBC_NEW_LOADER

#ifndef TCL_SHLIB_EXT
#  define TCL_SHLIB_EXT ".so"
#endif

#ifndef LIBPREFIX
#  ifdef __CYGWIN__
#    define LIBPREFIX "cyg"
#  else
#    define LIBPREFIX "lib"
#  endif
#endif

#ifdef __CYGWIN__
#  define TDBC_SHLIB_SEP "-"
#else
#  define TDBC_SHLIB_SEP "."
#endif

const char *const tdbcLibFormats[] = {
	LIBPREFIX "%s" TCL_SHLIB_EXT "%s" "%s",
	"%s" TCL_SHLIB_EXT "%s" "%s",
	NULL
};

/*
 *-----------------------------------------------------------------------------
 *
 * tdbcLoadLib --
 *
 *	Tries to load a shared library using all combinations of
 *	LIBPREFIX, no LIBPREFIX, lib names and lib numbers.
 *	Takes CYGWIN into account.
 *
 * Results:
 *	Returns the handle to the loaded ODBC client library and leaves the
 *	name of the the loaded ODBC client library in the interpreter, or NULL
 *	if the load is unsuccessful and leaves a list of error message(s) in the
 *	interpreter.
 *
 *-----------------------------------------------------------------------------
 */

static Tcl_LoadHandle
tdbcLoadLib (
    Tcl_Interp *interp,
    const char *const soNames[],
    const char *const soNumbers[],
    const char *const soSymbolNames[],
    const void *soStubDefs,
    const char *const soFormats[]
) {
    const char *const *nam;
    const char *const *num;
    const char *const *fmt;
    Tcl_Obj *errors;
    Tcl_Obj *lib;
    Tcl_LoadHandle handle;
    int status;

    if (soFormats == NULL) {
	soFormats = tdbcLibFormats;
    }

    errors = Tcl_NewListObj(0, NULL);
    Tcl_IncrRefCount(errors);

    for (nam = &soNames[0]; *nam != NULL; nam++) {
	for (num = &soNumbers[0]; *num != NULL; num++) {
	    for (fmt = &soFormats[0]; *fmt != NULL; fmt++) {
		lib = Tcl_ObjPrintf(*fmt, *nam, (*num[0] == '\0' ? "" : TDBC_SHLIB_SEP), *num);
		Tcl_IncrRefCount(lib);
		handle = NULL;
		status = Tcl_LoadFile(interp, lib, soSymbolNames, 0, (void *) soStubDefs, &handle);
		if (status == TCL_OK) {
		    Tcl_SetObjResult(interp, lib);
		    Tcl_DecrRefCount(lib);
		    Tcl_DecrRefCount(errors);
		    return handle;
		}
		Tcl_DecrRefCount(lib);
		Tcl_ListObjAppendElement(NULL, errors, Tcl_GetObjResult(interp));
	    }
	}
    }

    Tcl_SetObjResult(interp, errors);
    Tcl_DecrRefCount(errors);

    return NULL;
}

/*
 *-----------------------------------------------------------------------------
 *
 * OdbcInitStubs --
 *
 *	Initialize the Stubs table for the ODBC API
 *
 * Results:
 *	Returns the handle to the loaded ODBC client library, or NULL
 *	if the load is unsuccessful. Leaves an error message in the
 *	interpreter.
 *
 *-----------------------------------------------------------------------------
 */

MODULE_SCOPE Tcl_LoadHandle
OdbcInitStubs(Tcl_Interp* interp,
				/* Tcl interpreter */
	      Tcl_LoadHandle* handle2Ptr)
				/* Pointer to a second load handle
				 * that represents the ODBCINST library */
{
    int odbcStatus;		/* Status of Tcl library calls */
    int odbcOptStatus;		/* Status of Tcl library calls */
    Tcl_LoadHandle handle;	/* Handle to a load module */

    SQLConfigDataSourceW = NULL;
    SQLConfigDataSource = NULL;
    SQLInstallerError = NULL;

    /*
     * Try to load a client library and resolve the ODBC API within it.
     */

    handle = tdbcLoadLib(interp, odbcStubLibNames, odbcStubLibNumbers, odbcSymbolNames, odbcStubs, NULL);
    odbcStatus = (handle == NULL ? TCL_ERROR : TCL_OK);

    /*
     * We've run out of library names (in which case odbcStatus==TCL_ERROR
     * and the error message reflects the last unsuccessful load attempt).
     */

    if (odbcStatus != TCL_OK) {
	return NULL;
    }

    /*
     * If a client library is found, then try to load ODBCINST as well.
     */

    *handle2Ptr = tdbcLoadLib(interp, odbcOptLibNames, odbcOptLibNumbers, NULL, NULL, NULL);
    odbcOptStatus = (*handle2Ptr == NULL ? TCL_ERROR : TCL_OK);

    if (odbcOptStatus == TCL_OK) {
	SQLConfigDataSourceW =
	    (BOOL (INSTAPI*)(HWND, WORD, LPCWSTR, LPCWSTR))
	    Tcl_FindSymbol(NULL, *handle2Ptr, "SQLConfigDataSourceW");
	if (SQLConfigDataSourceW == NULL) {
	    SQLConfigDataSource =
		(BOOL (INSTAPI*)(HWND, WORD, LPCSTR, LPCSTR))
		Tcl_FindSymbol(NULL, *handle2Ptr,
			       "SQLConfigDataSource");
	}
	SQLInstallerError =
	    (BOOL (INSTAPI*)(WORD, DWORD*, LPSTR, WORD, WORD*))
	    Tcl_FindSymbol(NULL, *handle2Ptr, "SQLInstallerError");
    } else {
	Tcl_ResetResult(interp);
    }

    /*
     * We've successfully loaded a library.
     */

    return handle;
}

#else

/*
 *-----------------------------------------------------------------------------
 *
 * OdbcInitStubs --
 *
 *	Initialize the Stubs table for the ODBC API
 *
 * Results:
 *	Returns the handle to the loaded ODBC client library, or NULL
 *	if the load is unsuccessful. Leaves an error message in the
 *	interpreter.
 *
 *-----------------------------------------------------------------------------
 */

MODULE_SCOPE Tcl_LoadHandle
OdbcInitStubs(Tcl_Interp* interp,
				/* Tcl interpreter */
	      Tcl_LoadHandle* handle2Ptr)
				/* Pointer to a second load handle
				 * that represents the ODBCINST library */
{
    int i;
    int status;			/* Status of Tcl library calls */
    Tcl_Obj* path;		/* Path name of a module to be loaded */
    Tcl_Obj* shlibext;		/* Extension to use for load modules */
    Tcl_LoadHandle handle = NULL;
				/* Handle to a load module */

    SQLConfigDataSourceW = NULL;
    SQLConfigDataSource = NULL;
    SQLInstallerError = NULL;

    /*
     * Determine the shared library extension
     */
    status = Tcl_EvalEx(interp, "::info sharedlibextension", -1,
			TCL_EVAL_GLOBAL);
    if (status != TCL_OK) return NULL;
    shlibext = Tcl_GetObjResult(interp);
    Tcl_IncrRefCount(shlibext);

    /*
     * Walk the list of possible library names to find an ODBC client
     */
    status = TCL_ERROR;
    for (i = 0; status == TCL_ERROR && odbcStubLibNames[i] != NULL; ++i) {
	path = Tcl_NewStringObj(odbcStubLibNames[i], -1);
	Tcl_AppendObjToObj(path, shlibext);
	Tcl_IncrRefCount(path);
	Tcl_ResetResult(interp);

	/*
	 * Try to load a client library and resolve the ODBC API within it.
	 */
	status = Tcl_LoadFile(interp, path, odbcSymbolNames, 0,
			      (void*)odbcStubs, &handle);
	Tcl_DecrRefCount(path);
    }

    /*
     * If a client library is found, then try to load ODBCINST as well.
     */
    if (status == TCL_OK) {
	int status2 = TCL_ERROR;
	for (i = 0; status2 == TCL_ERROR && odbcOptLibNames[i] != NULL; ++i) {
	    path = Tcl_NewStringObj(odbcOptLibNames[i], -1);
	    Tcl_AppendObjToObj(path, shlibext);
	    Tcl_IncrRefCount(path);
	    status2 = Tcl_LoadFile(interp, path, NULL, 0, NULL, handle2Ptr);
	    if (status2 == TCL_OK) {
		SQLConfigDataSourceW =
		    (BOOL (INSTAPI*)(HWND, WORD, LPCWSTR, LPCWSTR))
		    Tcl_FindSymbol(NULL, *handle2Ptr, "SQLConfigDataSourceW");
		if (SQLConfigDataSourceW == NULL) {
		    SQLConfigDataSource =
			(BOOL (INSTAPI*)(HWND, WORD, LPCSTR, LPCSTR))
			Tcl_FindSymbol(NULL, *handle2Ptr,
				       "SQLConfigDataSource");
		}
		SQLInstallerError =
		    (BOOL (INSTAPI*)(WORD, DWORD*, LPSTR, WORD, WORD*))
		    Tcl_FindSymbol(NULL, *handle2Ptr, "SQLInstallerError");
	    } else {
		Tcl_ResetResult(interp);
	    }
	    Tcl_DecrRefCount(path);
	}
    }

    /*
     * Either we've successfully loaded a library (status == TCL_OK),
     * or we've run out of library names (in which case status==TCL_ERROR
     * and the error message reflects the last unsuccessful load attempt).
     */
    Tcl_DecrRefCount(shlibext);
    if (status != TCL_OK) {
	return NULL;
    }
    return handle;
}

#endif
