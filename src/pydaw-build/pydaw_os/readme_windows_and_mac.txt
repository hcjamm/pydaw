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

However, if Virtualbox is unstable on your CPU, you may need to disable 
virtualization acceleration.

You can adjust the number of cores that PyDAW can use by:

1.  Click on VM
2.  Click "Settings"
3.  Click "System" tab on the left
4.  Click the "Processor" tab on the top
5.  Change the number of processors

IMPORTANT:

Some audio drivers do not perform well;  if you are not getting smooth playback, you
may need to:

* Open the Virtualbox window
* Click on the pydaw4 VM on the left
* Click the "Settings" button
* Click the "Audio" tab on the left
* Select a better "Host Audio Driver" for your system
* Click the "OK" button on the bottom right of the window
