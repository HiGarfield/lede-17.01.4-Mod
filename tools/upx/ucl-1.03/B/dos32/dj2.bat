@echo // Copyright (C) 1996-2004 Markus F.X.J. Oberhumer
@echo //
@echo //   DOS 32-bit
@echo //   djgpp2 + gcc
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


set BLIB=lib%BNAME%.a
set CC=gcc
set CF=-O2 -fomit-frame-pointer -Wall %CFI% %CFASM%
set LF=%BLIB% -s

%CC% %CF% -c @b\src.rsp
@if errorlevel 1 goto error
%CC% -x assembler-with-cpp -c asm/i386/src_gas/*.S
@if errorlevel 1 goto error
ar rcs %BLIB% @b/win32/cygwin.rsp
@if errorlevel 1 goto error

%CC% %CF% -o simple.exe examples/simple.c %LF%
@if errorlevel 1 goto error
%CC% %CF% -o uclpack.exe examples/uclpack.c %LF%
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
