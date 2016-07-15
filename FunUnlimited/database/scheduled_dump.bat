@echo off
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
echo !!!                             !!!
echo !!!   BACKING UP THE DATABASE!  !!!
echo !!!   DO NOT CLOSE THIS WINDOW  !!!
echo !!!    OR DO ANYTHING ON THE    !!!
echo !!! SOFTWARE UNTIL THIS IS DONE !!!
echo !!!                             !!!
echo !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

cd c:\webroot\fununlimited\database

call strrep %date%
set usedate=%STR%
call strrep %time% : _
call strrep %STR% . _
set usetime=%STR%
set usefile=%usedate%_%usetime%

call dump %usefile%

echo Deleting old dump files...
c:\php4\php.exe -q del_old.php

echo Updating rankings...
c:\php4\php.exe -q update_rankings.php
:: exit
