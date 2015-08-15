TARGET = gd
EXT = cpp
BINPATH = /home/giulio/Programmazione/GBA/SDK/devkitARM/bin

$(TARGET).gba: $(TARGET).elf
	$(BINPATH)/arm-none-eabi-objcopy -O binary $(TARGET).elf $(TARGET).gba
	$(BINPATH)/gbafix $(TARGET).gba
	rm -f $(TARGET).o $(TARGET).elf
	
$(TARGET).elf: $(TARGET).o
	$(BINPATH)/arm-none-eabi-gcc -specs=gba.specs -mthumb -mthumb-interwork $(TARGET).o -o $(TARGET).elf
	
$(TARGET).o: $(TARGET).$(EXT)
	$(BINPATH)/arm-none-eabi-gcc -mthumb -mthumb-interwork -g -c $(TARGET).$(EXT)
