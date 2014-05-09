BASEDIR = moon/
include $(BASEDIR)Make.env

SUBDIR = client moon/lib/mnl server plugin demo conf

all: $(subdir)
	@$(MULTIMAKE) $(SUBDIR)

clean:
	@$(MULTIMAKE) -m clean $(SUBDIR)

install:
	@$(MULTIMAKE) -m install $(SUBDIR)

backup:
	@$(BACKUPDIR) $(SUBDIR)
