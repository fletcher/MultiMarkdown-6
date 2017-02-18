@ECHO OFF
:Loop
IF "%1"=="" GOTO Continue
   "%~dp0\multimarkdown" -c -b "%1"
SHIFT
GOTO Loop
:Continue

