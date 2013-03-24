Thank you for trying the PyDAW live DVD/USB image, below are some tips to help get you going if you're not familiar with how to use Linux audio or Ubuntu.  You may also want to Google search 'Ubuntu tutorial' if you're having trouble using it.

---------------------------------------------------------------------------

Some VERY important concepts of using the live USB/DVD:

1. Anything you save to your desktop or home folder while running the live USB/DVD WILL NOT BE THERE WHEN YOU REBOOT!!!
Therefore, you must save projects, audio, etc... to a separate data partition of the drive in {filesystem}/data/{name of partition}/folder1/folder2/...  You browse there in the SaveAs/Open dialogs by clicking 'filesystem' on the left, then the the folder with the name you gave the partition, then browse through or create folders as necessary.

2. You won't be able to reliably record audio directly back to the flash drive, unless you have some epic-ly fast USB3 flash drive, 99.9999% of flash drives just aren't fast enough.  However, a good USB3 mechanical hard drive or SSD could probably handle it...
IN THE FUTURE, I will create a "use super-sized memory buffers" for recording option to mitigate this problem, however, that is going to require an extraordinary amount of RAM to function correctly, and it would still be difficult to recommend that over a proper hard drive.

3.  When creating a new project using the live DVD/USB, you should immediately do a SaveAs...  PyDAW currently copies and then fully reloads the project on Save-As to ensure filesystem and memory integrity, copying the project from it's default in-memory location to the flash drive will freeze the window for a very long time due to the slow speed of most flash drives.

4.  For the best experience possible, you should install to your main hard drive rather than using live mode...  Of course, live mode offers certain advantages like being able to take your projects with you on a flash drive and boot on any other computer, but installing to a hard drive has better performance and less chance of losing anything by forgetting to save to the flash drive.

5.  In order to have internet in live mode, you are going to have to either plug in a network cable, or manually connect to your wireless network from the icon on the upper left of the screen, since the live USB won't remember your wireless network between boots.  If you don't need internet, you don't have to connect anything.

---------------------------------------------------------------------------
IMPORTANT:  Soundcard setup and configuration.

The soundcard you intend to use may not load by default when running the live DVD/USB image.  Below are instructions for configuring your soundcard.

All users:

Double-click the QJackCtl icon on your desktop.
Click the "Setup" button on the left of the small window that opened

1.  PCI/USB/onboard(and basically everything BUT firewire) users:

Make sure that the Server->Driver dropdown (upper left of the screen) says "alsa"

Below that is a label that says 'Interface', click the button that looks like [>] to the right of it and select your device.  If you're using onboard not sure which device is right, it likely mentions "analog" in the name...

2.  Firewire users

Make sure that the Server->Driver dropdown (upper left of the screen) says "firewire"

Below that is a label that says 'Interface', click the button that looks like [>] to the right of it and select your device.

ALL USERS AGAIN:

Now that you've selected your device, click "OK" on the bottom right to save your settings.  Now, on the previous window, click "Start" to test your settings.  If it fails to start, you may have selected the wrong device, or real-time mode may not be working.  Try unchecking the "Realtime" box in the setup window, or selecting a different device.

Unfortunately some devices aren't going to work, but it's the same story in Windows, not every card works perfectly there as an ASIO4ALL interface...  but if you're serious about making PyDAW your main DAW, it's worthwhile to get a proper soundcard.

I personally own these 2:

Alesis IO2 Express (USB)
Focusrite Saffire Pro 14 (Firewire)

both of which **IN MY PERSONAL EXPERIENCE** work just fine with PyDAW.   However, I can't guarantee that they'll work on every PC out there...

Note:  I also have an M-Audio Audiophile 192 and a Numark soundcard laying around that both work in Linux, somebody remind me to test those if I don't post the results here later...

---------------------------------------------------------------------------
