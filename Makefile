#!/usr/bin/make -f

all: build

build:
	$(MAKE) -C plugins/euphoria
	$(MAKE) -C plugins/lms_comb
	$(MAKE) -C plugins/lms_delay
	$(MAKE) -C plugins/lms_distortion
	$(MAKE) -C plugins/lms_eq5
	$(MAKE) -C plugins/lms_filter
	$(MAKE) -C plugins/ray_v

install:
	$(MAKE) install -C plugins/euphoria
	$(MAKE) install -C plugins/lms_comb
	$(MAKE) install -C plugins/lms_delay
	$(MAKE) install -C plugins/lms_distortion
	$(MAKE) install -C plugins/lms_eq5
	$(MAKE) install -C plugins/lms_filter
	$(MAKE) install -C plugins/ray_v

clean:
	$(MAKE) clean -C plugins/euphoria
	$(MAKE) clean -C plugins/lms_comb
	$(MAKE) clean -C plugins/lms_delay
	$(MAKE) clean -C plugins/lms_distortion
	$(MAKE) clean -C plugins/lms_eq5
	$(MAKE) clean -C plugins/lms_filter
	$(MAKE) clean -C plugins/ray_v
