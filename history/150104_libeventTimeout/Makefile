# This is a example to show how to use a dynamic Makefile

# gcc compiler
#CC = arm-mv5sft-linux-gnueabi-gcc
#LD = arm-mv5sft-linux-gnueabi-ld
CC = gcc
LD = ld

# program name
PROG_NAME = tmp

# install target
INSTDIR = ./

# target access mode
TARMODE = 554

# where include files are kept
# ---------I am not sure whether this is right----------------
INCLUDE = .

# link dir
LINKDIR = .

# headers to include
CFLAGS = -Wall -I$(INCLUDE)
# shared libs to include
LDFLAGS = -lpthread -lm -lrt -I$(INCLUDE) -L$(LINKDIR) -levent_core

# This is a good shell command
# put your .c files here
OBJS =$(shell ls *.c > clist.txt; sed 's/\.c/\.o/g' < clist.txt) 

all: $(PROG_NAME)
	@-rm *.d -rf
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
#	$(LD) $(LDFLAGS) -r -o $@ $(OBJS)# do not necessary

.PHONY: clean
clean:
	@rm -f $(OBJS) $(PROG_NAME).o clist.txt *.d *.d.* $(PROG_NAME)
	@echo "Project cleaned."

