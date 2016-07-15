@echo off
setlocal

:: variables
set fud_dir=c:\webroot\fununlimited\synch
set f=%fud_dir%\ftpc.txt

:: create ftp commands file
echo open 206.130.100.155>>%f%
echo USER fununlimitedonline>>%f%
echo 6bq9q55m>>%f%
echo lcd %fud_dir%>>%f%
echo cd /var/www/html/synch>>%f%
echo binary>>%f%
echo put %1>>%f%
echo bye>>%f%

:: execute ftp command
ftp -n -d -s:%f%

:: delete ftp commands file
del /f /q %f%

endlocal