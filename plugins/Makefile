#!/usr/bin/make -f

all: build

build:
	$(MAKE) -C euphoria
	$(MAKE) -C lms_comb
	$(MAKE) -C lms_delay
	$(MAKE) -C lms_distortion
	$(MAKE) -C lms_eq5
	$(MAKE) -C lms_filter
	$(MAKE) -C ray_v

install:
	$(MAKE) install -C euphoria
	$(MAKE) install -C lms_comb
	$(MAKE) install -C lms_delay
	$(MAKE) install -C lms_distortion
	$(MAKE) install -C lms_eq5
	$(MAKE) install -C lms_filter
	$(MAKE) install -C ray_v

clean:
	$(MAKE) clean -C euphoria
	$(MAKE) clean -C lms_comb
	$(MAKE) clean -C lms_delay
	$(MAKE) clean -C lms_distortion
	$(MAKE) clean -C lms_eq5
	$(MAKE) clean -C lms_filter
	$(MAKE) clean -C ray_v
