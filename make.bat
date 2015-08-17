path=C:\devkitadv\bin

gcc  -o gd.elf gd.cpp -lm

objcopy -O binary gd.elf gd.gba

pause