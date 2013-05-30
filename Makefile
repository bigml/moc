BASEDIR = ../
include $(BASEDIR)Make.env

SUBDIR = server plugin client demo conf

all: $(subdir)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

install:
	@$(MULTIMAKE) -m install $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
