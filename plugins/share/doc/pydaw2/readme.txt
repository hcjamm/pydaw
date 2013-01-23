Thank you for trying the PyDAW live DVD/USB image, below are some pointers to help get you going if you're not familiar with how to use Linux audio.


---------------------------------------------------------------------------
1.  Soundcard setup.

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
