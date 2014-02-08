######## HOW TO INSTALL #############################3

Step 1:  Install Virtualbox if you have not already installed it

Step 2:  Open Virtualbox and go to:

	File->Import Appliance

Then accept all default values.

IMPORTANT:

Please ensure that you have enabled all forms of virtualization acceleration in you BIOS.

If the VM gives an error message on starting, try going into:

	[vm name]->settings->system->processor

and setting the CPU core count to 1, and/or disabling the features in:

	[vm name]->settings->system->acceleration


IMPORTANT:

Some audio drivers do not perform well;  if you are not getting smooth playback, you
may need to:

* Open the Virtualbox window
* Click on the pydaw4 VM on the left
* Click the "Settings" button
* Click the "Audio" tab on the left
* Select a better "Host Audio Driver" for your system
* Click the "OK" button on the bottom right of the window
