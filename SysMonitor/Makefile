GCC := arm-linux-gnueabihf-gcc
CFLAGS = -g
CFLAGS += -Wall


all : Sysmonitor
SRC=$(wildcard *.c)
Objs := $(patsubst %.c,%.o,$(SRC))

Sysmonitor:$(Objs)
	$(GCC) $(Objs) -o $@ -lpthread
	
%.d: %.c
	@set -e; rm -f $@; \
	$(GCC) -M $(CFLAGS) $< > $@.

	; \
	sed 's,$ *\.o[ :]*,\1.o $@ : ,g' < $@.

	> $@; \
	rm -f $@.

%.o:%.c
	@echo 'Buildingfile: $<'
	@echo 'Invoking:GCC C Compiler'
	$(GCC) -O0 -g3 -Wall -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ''
	
.PHONY : clean
clean :
	rm Sysmonitor $(Objs)
