## Requirements
Visual Studio 2017 (C++)
ObjectBRX or ObjectARX SDK to build plugin for AutoCAD or BricsCAD respectively.
(Note: current source only setup for BricsCAD at the moment)
ECL (Embedded Common Lisp) binaries built from source.
Emacs with Slime for testing and basic IDE (or other editor that can hook up with a swank server)

## Building ECL
Refer to the ECL doc's for basic instructions and requirements for building ECL 
https://common-lisp.net/project/ecl/static/manual/Building-ECL.html#MSVC-based-configuration

Use the makefile provided with the config setup to work for this project.

To build run the following nmake commands in 'x64 Native Tools Command Prompt for VS2017'
>nmake

To install/create package for distribution (will put built binaries and lib's in a 'package' folder in the ecl source folder)
>nmake install

## Running the plugin
Build the plugin as you would any other ARX/BRX being sure to fix the inlude and lib directories so that VS can find ECL.
On succesful load of the plugin into CAD enter command 'ECL_START', a console should open displaying the status of the initialisation of ECL.
If you see 'I've run the contents of initrc.lisp' all should have loaded succesfully and started the Swank server.

## Testing and writing CL (Common Lisp) code
Startup Emacs and start up a slime repl:

M-x slime-connect RET localhost RET 4005 RET

You should have a REPL ready to go, try entering (testecl) to call a function we define in intrc.lisp
Enter (cad-prompt) to run our C function to send a message to the CAD prompt.
Start developing :)