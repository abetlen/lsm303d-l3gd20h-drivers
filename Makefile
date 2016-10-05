DRIVERS=l3gd20h lsm303d
BOARD=board
SUBDIRS= $(DRIVERS) $(BOARD)

.PHONY: subdirs $(SUBDIRS)

all: $(SUBDIRS)

drivers: $(DRIVERS)

$(SUBDIRS):
	$(MAKE) -C $@
