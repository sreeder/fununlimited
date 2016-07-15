@Echo Off
    If %1'==:' If Not %2'==' Goto %2
    If Not %1'==/?' If Not %2'==' Goto Begin
    Echo Remove/Replace Character(s) From/In a String.
    Echo.
    Echo [Call] %0 Str Chr [Newchr]
    Echo.
    Echo   Str        The string
    Echo   Chr        The character to be removed or replaced
    Echo              with 'Newchr' (non NT OS: case sensitive!)
    Echo   Newchr     The new character or string (optional)
    Echo.
    Echo The resulting string will be assigned to the STR variable.
    Goto End

   :Begin
    Set STR=
    Set CCHR=%2
    Set NCHR=%3
    If %OS%'==Windows_NT' Goto %OS%
    Echo ; | Choice /S /C:;%1; %0 : Loop,>%TEMP%.\Tmp.bat
    %TEMP%.\Tmp.bat
   :Loop
    If %4'==]?' Goto End
    If %4'==%CCHR%' For %%C In (Set Shift Goto:Loop) Do %%C STR=%STR%%NCHR%
    For %%C In (Set Shift Goto:Loop) Do %%C STR=%STR%%4

   :Windows_NT
    Set STR=%1
    Echo %STR% | Find /I "%CCHR%" > NUL && Call :%OS% %%STR:%CCHR%=%NCHR%%%

   :End
    If Exist %TEMP%.\Tmp.bat Del %TEMP%.\Tmp.bat
    For %%V In (CCHR NCHR) Do Set %%V=
