@echo off

echo Dumping FunUnlimited database into %1.allDB.dump...
mysqldump -ufununlimited -pfununlimited --opt fununlimited > %1.allDB.dump
echo Successfully dumped!
