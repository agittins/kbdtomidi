# kbdtomidi
turn any old PC Keyboard into a midi controller

kbdtomidi takes over exclusive control (ie: completely hijacks) an input device such as a usb keyboard and
instead of allowing it to create keypress events it translates them into midi events instead.

This is particularly useful for using a cheap usb keyboard as a simple transport control surface or in
particular as a midi foot controller - rip off most of the keys so your big hooves don't press all the keys
at once, throw it on the floor and bingo!

See the files in conf/ for pre-written systemd and (upstart?) config files. Make sure you edit them to have the correct path to your usb keyboard, otherwise you might lose the ability to type if the daemon takes control of your main keyboard. YOU HAVE BEEN WARNED!


