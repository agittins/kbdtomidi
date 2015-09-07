# kbdtomidi
turn any old PC Keyboard into a midi controller

## what?

kbdtomidi takes over exclusive control (ie: completely hijacks) an input device such as a usb keyboard and instead of allowing it to create keypress events it translates them into midi events instead.

When you run this program (or install it so the system runs it for you) it will take over the usb device you tell it to control, and every time a key is pressed on that keyboard it will generate a note-on and a controller-up midi event. When the key is released it sends a note-off and controller-down event. This means you can easily bind to either the controller signal or the note signal depending on how you want your midi client to behave. The downside is that some clients which self-learn their controls might get confused about whether to bind to the note, controller or both.

## when?

I probably first wrote this in 2005 or so.

## why?

This is particularly useful for using a cheap usb keyboard as a simple transport control surface or in particular as a midi foot controller - rip off most of the keys so your big hooves don't press all the keys at once, throw it on the floor and bingo!

Much cheaper than buying a "proper" midi footcontroller, and a lot easier than building one too.

If you use something like sooperlooper, lupp, guitarix etc this makes things a lot more enjoyable - you can easily control effects, loops etc with your feet for the cost of the cheapest usb keyboard you can find!

## how?

There are no pre-built packages so you'll need to build it on your system. 
 - Download or clone the repository
 - you'll need a c compiler (gcc) and also the development libraries for ALSA (probably alsa-devel or similar). Each distro has its own names for these packages. Let me know what you learn so I can update this with something more user-friendly :-)
 - sh$ make
 - If your system uses systemd (seems to be the forerunner for world domnination currently) then:
  - plug in your usb keyboard, and explore /dev/input to find out what its device name is. ** it is important you do this - you do not want this daemon to hijack the keyboard you are using to type! **
  - copy the dist/kbdtomidi.service to /etc/systemd/system/
  - edit the file and fix the path to your kbdtomidi executable and the path to your keyboard device node.
  - sh# systemctl enable kbdtomidi
  - sh# systemctl start kbdtomidi
  - sh# systemctl status kbdtomidi
  - check the status shows that it's running.
 - if you change anything you can just do a make, then a make restart to kill the existing daemon, allowing the system to restart the new one.

## no, technically, how?

This program is very very basic (in fact, this readme file is probably bigger than the sourcecode). What it does is:
 - creates a new rawmidi ALSA device that you can connect to
 - opens the given input device for exclusive use (thus preventing it from propogating input events to the rest of the system)
 - waits for input events to arrive
 - every time a keypress is received, we emit two midi messages:
  - note on for that keyid
  - controller up (127) for that keyid
 - every time a key-release event arrives we do the same but with a note-off and a controller down (0) message.
 - yes, that is a bit of a daft way to do it. The point is I could get it going and working for the majority of cases without having to mess about with creating/parsing a config file. It actually works rather well.
