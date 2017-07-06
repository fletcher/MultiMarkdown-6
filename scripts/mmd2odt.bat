@ECHO OFF
:Loop
IF "%1"=="" GOTO Continue
   "%~dp0\multimarkdown" -b -t odt "%1"
SHIFT
GOTO Loop
:Continue

