# This is a example to show how to use a dynamic Makefile

# gcc compiler
#CC = arm-mv5sft-linux-gnueabi-gcc
#LD = arm-mv5sft-linux-gnueabi-ld
#CC = mipsel-linux-gcc
#CPP= mipsel-linux-g++
CC = gcc
CPP= g++
LD = ld

# program name
PROG_NAME = tmp
STATIC_LIBS = /usr/local/lib/libmbedtls.a /usr/local/lib/libmbedx509.a /usr/local/lib/libmbedcrypto.a

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
CPPFLAGS = $(CFLAGS)
# shared libs to include
LDFLAGS = -lpthread -lm -lrt -lev #-lbmbedtls -lmbedx509 -lmbedcrypto

# This is a good shell command
# put your .c files here
C_OBJS =$(shell ls *.c > clist.txt 2>/dev/null; sed 's/\.c/\.o/g' < clist.txt) 
CPP_OBJS =$(shell ls *.cpp > cpplist.txt 2>/dev/null; sed 's/\.cpp/\.o/g' < cpplist.txt) 

NULL =#
ifneq ($(strip $(CPP_OBJS)), $(NULL))
CC = $(CPP)
endif

.PHONY:all
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

-include $(C_OBJS:.o=.d)
-include $(CPP_OBJS:.o=.d)

$(CPP_OBJS): $(CPP_OBJS:.o=.cpp)
	$(CPP) -c $(CPPFLAGS) $*.cpp -o $*.o  
	$(CPP) -MM $(CPPFLAGS) $*.cpp > $*.d  
	@mv -f $*.d $*.d.tmp  
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d  
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp 

$(C_OBJS): $(C_OBJS:.o=.c)
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	$(CC) -MM $(CFLAGS) $*.c > $*.d  
	@mv -f $*.d $*.d.tmp  
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d  
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp 


$(PROG_NAME): $(C_OBJS) $(CPP_OBJS)
	@rm clist.txt cpplist.txt
	$(LD) -r -o $@.o $(C_OBJS) $(CPP_OBJS)
	$(CC) $@.o $(STATIC_LIBS) -o $@ $(LDFLAGS)
	chmod $(TARMODE) $@

.PHONY: clean
clean:
	@rm -f $(C_OBJS) $(CPP_OBJS) $(PROG_NAME) clist.txt cpplist.txt *.d *.d.*
	@echo "Project cleaned."

.PHONY: test
test:
	@echo -e "C_OBJS:\n$(C_OBJS)"
	@echo -e "CPP_OBJS:\n$(CPP_OBJS)"

