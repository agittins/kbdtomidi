all:
	/usr/bin/gcc -lasound kbdtomidi.c -o kbdtomidi

reload: all
	killall -9 kbdtomidi
	# You'd normally want the process re-spawned by an init script, but you could just:
	# ./kbdtomidi /dev/input/by-id/usb-_USB_Keyboard-event-kbd virtual >> out.log

clean:
	rm -f kbdtomidi
distclean: clean

