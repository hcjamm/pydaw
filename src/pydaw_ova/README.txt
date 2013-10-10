Login credentials for unlocking the screen or installing software:

	Username:  pydaw
	Password:  pydaw


How to run PyDAW:

	Simply click the "PyDAW" icon on the left

How do I browse the internet?

	Click on "Midori" (the green leaf looking icon on the left).  You can install Firefox or Chrome/Chromium by 
	clicking on Ubuntu Software Centre (the icon that looks like a shopping bag).

How do I access the samples and MIDI files on my hard-drive?

	Click on the "Devices" menu on the Virtualbox window (it's not inside the VM), click "Shared Folders", click the 
	folder icon with a '+' symbol in the dialog, and set the folder to "Automount" and "Permanent" (you probably don't 
	want "Read-only", but maybe you do).  Reboot and the folder should mount automatically.

How do I select my soundcard?

	AFAIK Virtualbox will only use the default system device, so you must set that to the desired device.  DO NOT try 
	to set the device in PyDAW, PyDAW can only see the virtualized audio hardware that Virtualbox provides (usually 
	says Intel audio).

How do I use my MIDI device?

	Currently, you can only use USB MIDI devices using Virtualbox "USB MIDI pass-through".  You can Google that 
	for more info, but some devices do not work well this way.

How can I make PyDAW full-screen?

	Go to the "View" menu and select the full-screen option.  Alternately press Right-CTRL+F in Windows 
	(I'm not sure what the equivalent Mac command is, sorry).

How do I upgrade to the latest version of PyDAW?

	You do not need to download the Virtualbox .ova image again.  Instead, open Midori (the green leaf icon on the 
	left), and browse to this URL:

	http://sourceforge.net/projects/libmodsynth/files/linux/

	...and download the 32-bit .deb package of PyDAW and install(ie: not the one that says AMD64, and 
	not the one that says source-code).  It will only be a few megabytes.

How do I use the other software in the VM?
	
	Click the top icon on the bar on the left (has the Ubuntu "circle of friends" logo on it) and either type
	in a search query for what you want, or else click on the applications button (bottom of the launcher window).

Can I upgrade and install software in the VM?
	
	Yes, you can.  I don't advocate running updates, generally there is little need to do so, and there's always
	the possibility that it will break something.  If you would like to install additional software, I recommend
	doing it through "Ubuntu Software Center" (shopping bag icon on the launcher).


