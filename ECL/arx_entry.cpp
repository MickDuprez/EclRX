

#include "stdafx.h"

#include <tchar.h>
#include "rxregsvc.h"
#include "acutads.h"
#include "accmd.h"

#include <Windows.h>

#include <string>
#include <iostream>
#include <fstream>
#include <conio.h>
#include <stdio.h>
#include <crtdbg.h>
#include <fcntl.h>
#include <io.h>

#include <stdlib.h>
#include "ecl\ecl.h"


#pragma comment(linker,"/EXPORT:acrxEntryPoint")
#pragma comment(linker,"/EXPORT:acrxGetApiVersion")

//static const WORD MAX_CONSOLE_LINES = 500;

void CreateConsole()
{
	if (!AllocConsole()) {
		// Add some error handling here.
		// You can call GetLastError() to get more info about the error.
		return;
	}
	
	// std::cout, std::clog, std::cerr, std::cin
	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	std::cout.clear();
	std::clog.clear();
	std::cerr.clear();
	std::cin.clear();
}

// A macro to create a DEFUN abstraction in C++
// Credit: https://gist.github.com/vwood/662109
// Updated by Mick Duprez Oct 2020 - updated from depricated function.
#define DEFUN(name,fun,args) \
 ecl_def_c_macro(c_string_to_object(name), \
 (cl_objectfn_fixed)fun, \
 args)

// Define a helper function to run arbitrary Lisp expressions
cl_object lisp(const std::string & call, cl_object msg) {
	cl_object form = ecl_read_from_cstring(call.c_str());
	return si_safe_eval(3, form, ECL_NIL, msg);
}

// test function to call CAD from lisp:
cl_object cad_prompt() {
	acutPrintf(L"Call to ARX from CL works as well.");
	return ecl_make_integer(1);
}


char* argv;
char** pargv;
void static startEcl()
{
	// open a console window to see logging/errors etc, may be able
	// to remove this or have it optional once system is stabilised.
	// NOTE!!! - do not close console without calling stopEcl, it 
	// will crash CAD!
	CreateConsole(); // sets up FILE* for stdin/out etc

	// setup args to pass to ECL boot:
	argv = (char*)"eclrx-app";
	pargv = &argv;

	// holds some info returned from ECL calls we use for debugging.
	cl_object error = NULL;

	// Initialise ECL:
	cl_boot(1, pargv);

	// set up the in/out/err filestreams:
	ecl_setq(ecl_process_env(),
		ecl_make_symbol("*STANDARD-OUTPUT*", "COMMON-LISP"),
		ecl_make_stream_from_FILE(ecl_make_constant_base_string("stdout", -1), /* file name */
			stdout, /* pointer to FILE struct */
			ecl_smm_output, /* stream mode */
			8, /* byte-size */
			0, /* default flags */
			ecl_make_keyword("UTF-8"))); /* external-format: utf-8 */

	ecl_setq(ecl_process_env(),
		ecl_make_symbol("*ERROR-OUTPUT*", "COMMON-LISP"),
		ecl_make_stream_from_FILE(ecl_make_constant_base_string("stderr", -1),
			stderr,
			ecl_smm_output,
			8,
			0,
			ecl_make_keyword("UTF-8")));

	ecl_setq(ecl_process_env(),
		ecl_make_symbol("*STANDARD-INPUT*", "COMMON-LISP"),
		ecl_make_stream_from_FILE(ecl_make_constant_base_string("stdin", -1),
			stdin,
			ecl_smm_input,
			8,
			0,
			ecl_make_keyword("UTF-8")));

	lisp("(load \"~/initrc.lisp\")", error);
	// see if anything was put in 'error' cl_object:
	if (error != NULL) {
		printf("Error in 'load initrc' call. %s", error->base_string.self);
		error = NULL; // reset for next checks.
	}

	// Define a simple function to call a C function from ECL
	DEFUN("cad-prompt", cad_prompt, 0);
}

void static stopEcl() {
	// this function is supposed to do the clean up for ECL
	// but doesn't seem to be working as expected. That is,
	// a restart of ECL doesn't seem to reload dep's etc.
	cl_shutdown();
	FreeConsole(); // just frees the console, doesn't close it!
}

extern "C"
AcRx::AppRetCode acrxEntryPoint(AcRx::AppMsgCode msg, void* appId)
{

	switch (msg)
	{
	case AcRx::kInitAppMsg:
		acrxDynamicLinker->unlockApplication(appId);
		acrxRegisterAppMDIAware(appId);

		//register the commands
		acedRegCmds->addCommand(L"ECLRXGROUP",
			_T("ECL_START"),
			_T("ECL_START"),
			ACRX_CMD_MODAL | ACRX_CMD_USEPICKSET,
			startEcl);
		acedRegCmds->addCommand(L"ECLRXGROUP",
			_T("ECL_STOP"),
			_T("ECL_STOP"),
			ACRX_CMD_MODAL | ACRX_CMD_USEPICKSET,
			stopEcl);

		acutPrintf(L"\nEclRX Application Loaded");
		break;
	case AcRx::kUnloadAppMsg:

		acedRegCmds->removeGroup(L"ECLRXGROUP");
		acutPrintf(L"\nEclRX Application Unloaded");
		break;
	case AcRx::kLoadDwgMsg:
		// initialize lisp functions here
		break;
	case AcRx::kUnloadDwgMsg:
		// remove lisp functions here
		break;
	}
	return AcRx::kRetOK;
}