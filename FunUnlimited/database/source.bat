@echo off

echo Loading %1.allDB.dump into FunUnlimited database...
mysql -ufununlimited -pfununlimited fununlimited < %1.allDB.dump
echo Successfully loaded!