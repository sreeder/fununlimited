@echo off

echo Dumping FunUnlimited database (from server) into %1.server.allDB.dump...
mysqldump -hwww.fununlimitedonline.com -ufununlimited -pfununlimited --opt fununlimited > %1.server.allDB.dump
echo Successfully dumped!
