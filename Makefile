# Project Name
TARGET = ccam_vco

# Sources
CPP_SOURCES = main.cpp

# Library Locations
LIBDAISY_DIR = libDaisy
DAISYSP_DIR = DaisySP

CPP_STANDARD = -std=gnu++20
APP_TYPE = BOOT_QSPI

USE_DAISYSP_LGPL = 1

JSON_FILES := $(wildcard ./json/*.json)
JSON_HEADERS := $(JSON_FILES:.json=.h)

earth: $(JSON_HEADERS)
	cd $(DAISYSP_DIR) && make
	cd $(LIBDAISY_DIR) && make
	$(MAKE) all

$(JSON_HEADERS): %.h: %.json
	PYTHONPATH=$(PYTHONPATH):json2daisy/src \
	python -m json2daisy $^ -o $@

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

.PHONY: clean
clean:
	cd $(DAISYSP_DIR) && $(MAKE) clean
	cd $(LIBDAISY_DIR) && $(MAKE) clean
	rm -rf build
	rm -f json/*.h