# This is a example to show how to use a dynamic Makefile

# gcc compiler
CROSS_COMPILE ?= mipsel-linux-
CC = $(CROSS_COMPILE)gcc
CPP= $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld

# program name
PROG_NAME = cpu
STATIC_LIBS =

# install target
INSTDIR = ./

# target access mode
TARMODE = 774

# where include files are kept
# ---------I am not sure whether this is right----------------
INCLUDE = .

# link dir
LINKDIR = .

# headers to include
CFLAGS = -Wall -I$(INCLUDE) -I/opt/RT288x_SDK/source/lib/include
CPPFLAGS = $(CFLAGS)
# shared libs to include
LDFLAGS = -lpthread -lm -lrt #-lev -lbmbedtls -lmbedx509 -lmbedcrypto

EXCLUDE_C_SRCS =AMCArp.c AMCMbedTLSTools.c AMCLibeventTool.c AMCLibevTools.c AMCBinSearchTree_Static.c AMCRandom.c
EXCLUDE_CPP_SRCS =#

# This is a good shell command
# put your .c files here
C_SRCS = $(filter-out $(EXCLUDE_C_SRCS), $(wildcard *.c))
CPP_SRCS = $(filter-out $(EXCLUDE_CPP_SRCS), $(wildcard *.cpp))

C_OBJS = $(C_SRCS:.c=.o)
CPP_OBJS = $(CPP_SRCS:.cpp=.o)
#C_OBJS =$(shell ls *.c > clist.txt 2>/dev/null; sed 's/\.c/\.o/g' < clist.txt) 
#CPP_OBJS =$(shell ls *.cpp > cpplist.txt 2>/dev/null; sed 's/\.cpp/\.o/g' < cpplist.txt) 

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
	@$(CPP) -MM $(CPPFLAGS) $*.cpp > $*.d  
	@mv -f $*.d $*.d.tmp  
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d  
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp 

$(C_OBJS): $(C_OBJS:.o=.c)
	$(CC) -c $(CFLAGS) $*.c -o $*.o
	@$(CC) -MM $(CFLAGS) $*.c > $*.d  
	@mv -f $*.d $*.d.tmp  
	@sed -e 's|.*:|$*.o:|' < $*.d.tmp > $*.d  
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $*.d.tmp 


$(PROG_NAME): $(C_OBJS) $(CPP_OBJS)
	@echo "$(LD) -r -o $@.o *.o"
	@$(LD) -r -o $@.o $(C_OBJS) $(CPP_OBJS)
	$(CC) $@.o $(STATIC_LIBS) -o $@ $(LDFLAGS)
	chmod $(TARMODE) $@

.PHONY: clean
clean:
	@rm -f $(C_OBJS) $(CPP_OBJS) $(PROG_NAME) clist.txt cpplist.txt *.d *.d.* *.o
	@echo "Project cleaned."

.PHONY: test
test:
	@echo -e "C_OBJS:\n$(C_OBJS)"
	@echo -e "CPP_OBJS:\n$(CPP_OBJS)"
	@echo -e "C_SRCS:\n$(C_SRCS)"
	@echo -e "EXCLUDE_C_SRCS:\n$(EXCLUDE_C_SRCS)"

