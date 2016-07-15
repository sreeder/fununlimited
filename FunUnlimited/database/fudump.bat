@echo off

echo Dumping FunUnlimited database (from store) into %1.store.allDB.dump...
mysqldump -hfununlimitedpos -ufununlimited -pfununlimited --opt fununlimited > %1.store.allDB.dump
echo Successfully dumped!
