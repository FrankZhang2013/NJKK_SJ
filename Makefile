CC=gcc
DEBUG_FLAGS=-g #-DDEBUG
INCLUDE_FLAGS=-I../include
LIBRARY_FLAGS=-L../lib -lcurl -lxml2 -lpthread
OPTIMIZE_FLAGS=#-O6
WARNING_FLAGS=#-Wall
CFLAGS:=$(DEBUG_FLAGS) $(INCLUDE_FLAGS) $(OPTIMIZE_FLAGS) $(WARNING_FLAGS)

objects=Irdm_main.c SBD_FTP_Classify.c njkk_interface.c  Irdm_Interface.c FTP_Download.c \
		SBD_Check.c FTP_Upload.c SBD_Upload.c SBD_Cmd.c OpenPort_Modem_state_check.c SJ_heart.c

Irdm_main:$(objects)
	$(CC) $(CFLAGS) $(LIBRARY_FLAGS)  $(objects) -o  Irdm_main 

clean:
	rm Irdm_main
#	@echo files cleared
run:
	./Irdm_main
