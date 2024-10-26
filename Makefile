# Library Locations
LIBDAISY_DIR = libDaisy
DAISYSP_DIR = DaisySP
JSON_DIR = json

JSON_FILES := $(wildcard ./$(JSON_DIR)/*.json)
JSON_HEADERS := $(JSON_FILES:.json=.h)

ifndef CCAM_PROJECT
$(error invalid CCAM_PROJECT)
else
include $(CCAM_PROJECT)/Makefile
endif

# Since this is the very first target
# it will be the default
ccam_main: $(JSON_HEADERS)
	cd $(DAISYSP_DIR) && make
	cd $(LIBDAISY_DIR) && make
	$(MAKE) all

# build all board header files
$(JSON_HEADERS): %.h: %.json
	PYTHONPATH=$(PYTHONPATH):json2daisy/src \
	python -m json2daisy $^ -o $@

# Core location, and generic Makefile.
C_INCLUDES = -I$(JSON_DIR)
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

.PHONY: deepclean
deepclean:
	cd $(DAISYSP_DIR) && $(MAKE) clean
	cd $(LIBDAISY_DIR) && $(MAKE) clean
	rm -rf build
	rm -f json/*.h