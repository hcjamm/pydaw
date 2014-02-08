This file requires a recent version of Virtualbox or another suitable desktop hypervisor.

Virtualbox can be had from here:
	https://www.virtualbox.org/wiki/Downloads

To install the image, open Virtualbox and go to:

	File->Import Appliance

Then accept all default values.

IMPORTANT:

Please ensure that you have enabled all forms of virtualization acceleration in you BIOS.

If the VM gives an error message on starting, try going into:

	[vm name]->settings->system->processor

and setting the CPU core count to 1, and/or disabling the features in:

	[vm name]->settings->system->acceleration
