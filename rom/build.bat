FOR %%i IN (*.asm) DO ECHO %%i & fasm %%i %%~ni.bin & bin2c -o %%~ni.h -m %%~ni.bin
