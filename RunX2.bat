@ECHO OFF

CALL :MainScript
GOTO :EOF

:MainScript
	CALL :RunProgramAsync "C:\Users\BTH\source\repos\SocioBlade\Circle-Buffer\x64\Debug\Shared.exe producer  0 64 10000 random"
	CALL :RunPRogramAsync "C:\Users\BTH\source\repos\SocioBlade\Circle-Buffer\x64\Debug\Shared.exe consumer  0 64 10000 random"
GOTO :EOF

:RunProgramAsync
	REM ~sI expands the variable to contain short DOS names only
	start %~s1
GOTO :EOF