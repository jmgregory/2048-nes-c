CC65 = `which cc65`
CA65 = `which ca65`
LD65 = `which ld65`
NAME = lesson2


$(NAME).nes: $(NAME).o asm4c.o reset.o nes.cfg
	$(LD65) -C nes.cfg -o $(NAME).nes asm4c.o reset.o $(NAME).o nes.lib
	rm *.o
	@echo $(NAME).nes created

%.o: %.s
	$(CA65) $<

reset.o: tiles.chr

$(NAME).s: DEFINE.c
%.s: %.c
	$(CC65) -Oi $< --add-source

clean:
	-rm *.nes

