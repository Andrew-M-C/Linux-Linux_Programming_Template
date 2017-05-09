# This is a example to show how to use a dynamic Makefile

# gcc compiler
#CC = arm-mv5sft-linux-gnueabi-gcc
#LD = arm-mv5sft-linux-gnueabi-ld
CC = mipsel-linux-gcc
LD = mipsel-linux-ld

# program name
PROG_NAME = gpioTest

# install target
INSTDIR = /tftpboot/NC200_UserLocal/bin/

# target access mode
TARMODE = 554

# where include files are kept
# ---------I am not sure whether this is right----------------
INCLUDE = .
INCLUDE = /opt/RT288x_SDK/source/linux-2.6.36.x/drivers/char/
INCLUDE += -I/opt/RT288x_SDK/source/lib/include/

# link dir
LINKDIR = .
LINKDIR = /opt/RT288x_SDK/source/lib/lib/

# headers to include
CFLAGS = -Wall -I$(INCLUDE)
# shared libs to include
LDFLAGS = -lpthread -lm -lrt -I$(INCLUDE) -L$(LINKDIR) 

# This is a good shell command
# put your .c files here
OBJS =$(shell ls *.c > clist.txt; sed 's/\.c/\.o/g' < clist.txt) 

all: $(PROG_NAME)
	@echo ""
	@echo "	<< $(PROG_NAME) build >>"
	@echo ""

install: all
	@if [ -d $(INSTDIR) ]; \
		then \
		cp $(PROG_NAME) $(INSTDIR) -f; \
		chmod $(TARMODE) $(INSTDIR)/$(PROG_NAME); \
		echo "Install in $(INSTDIR)"; \
		echo ""; \
		echo "	<< $(PROG_NAME) installed >>"; \
		echo ""; \
	else \
		echo "Error, $(INSTDIR) does not exist"; \
	fi

-include $(OBJS:.o=.d)

%.o: %.c  
	$(CC) -c $(CFLAGS) $*.c -o $*.o  
	$(CC) -MM $(CFLAGS) $*.c > $*.d  
	@mv -f $*.d $*.d.tmp  
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d  
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp 



$(PROG_NAME): $(OBJS)
	@rm clist.txt
	$(CC) $(OBJS) -o $(PROG_NAME) $(LDFLAGS)
#	$(LD) $(LDFLAGS) -r -o $@ $(OBJS)

.PHONY: clean
clean:
	@rm -f $(OBJS) $(PROG_NAME).o clist.txt *.d *.d.* $(PROG_NAME)
	@echo "Project cleaned."

