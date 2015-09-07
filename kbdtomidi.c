#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <linux/input.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <alsa/asoundlib.h>


static int stop=0;

static int fd;
static int own_dev = 1;
int grab = 1;

void catch_signal(int signal)
{
  stop=1;
  switch ( signal ) {
  case SIGINT:
    /* release device if needed */
    if ( own_dev ) { 
      ioctl(fd, EVIOCGRAB, NULL); 
      own_dev = 0;
    } else {
      ioctl(fd, EVIOCGRAB, &grab);
      own_dev = 1;
    }
    break;
  case SIGSTOP:
  case SIGQUIT:
    exit(0);
    break;
  }
}

/* Midi info:
 *
 * looks like there are a few blocks of midi controller numbers allocated to general use.
 * 		16 to 17 are "general purpose" 1 through 4.
 * 		48 to 51 are "general purpose" 1 through 4.
 *		70 to 79 are "sound controller" 1 through 10, but the first 5 are:
 *			70	sound variation
 *			71	harmonic content
 *			72	release time
 *			73	attack time
 *			74	brightness
 *		80 to 83 are "general purpose 5 through 8 (!)
 *
 * A more generalised (and perhaps more accurate) page lists the following:
 *		3		undefined
 *		9		undefined
 *		12-15	undefined (3)
 *		16-19	general purpose 1-4
 *		20-31	undefined (12)
 *		32-63	LSB for controllers 0-31 (!)
 *		70-79	undefined (10)
 *		80-83	general purpose 5-8
 *		84-91	undefined (7)
 *		102-121	undefined
 *		122+	Channel Mode Messages
 *
 *
 *
 *	To send a midi controller message:
 *
 *		0xBX 0xYY 0xZZ
 *
 *		Where X is the channel number (in zero-based form), YY is the controller number and ZZ is the value.
 *
 *		If you stay on the same channel, you can omit the leading byte for subsequent controller updates
 *
 *
 */


int main(int argc, char *argv[])
{
  fd_set rfds; 
  int res;
  int err;
  int version = -1, ioret = -1; 
  unsigned numevs, c;
  unsigned char read_buffer[sizeof(struct input_event)*3]; /* max 3 events per read */
  unsigned char ch;
  unsigned char cCommand[3];
  unsigned char cStates[255];
  struct input_event *currev;
  char device_name[1024];

  char *device_out = NULL;
  device_out = argv[2];

  int fd_in = -1,fd_out = -1;
  snd_rawmidi_t *handle_in = 0,*handle_out = 0;

  memset(&cStates, 0, sizeof(cStates));

  /* struct sigaction sighandler;
  memset(&sighandler, 0, sizeof(sighandler));
  sighandler.sa_handler = catch_signal;
  sigaction(SIGINT, &sighandler, NULL);
  sigaction(SIGQUIT, &sighandler, NULL);

  */

  signal(SIGINT,catch_signal);


  if ( argc < 2 ) { 
    fprintf(stderr, "Device needed\n");
    return -1;
  }

  FD_ZERO(&rfds);
  fd = open(argv[1], O_RDONLY);
  if ( -1 == fd ) {
    fprintf(stderr, "unable to read from mice - please give me a keyboard.\n");
    return -1;
  }

  ioret = ioctl(fd, EVIOCGVERSION, &version);
  ioret = ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name);
  ioret = ioctl(fd, EVIOCGRAB, &grab);
  if ( -1 == ioret ) {
    perror("ioctl()");
  }
  fprintf(stdout, "ver: %d, ret = %d\n", version, ioret);
  printf("device name is: %s\n", device_name);
  printf("EVIOCGRAB is: %d\n", EVIOCGRAB);


  if (device_out) {
	err = snd_rawmidi_open(NULL,&handle_out,device_out,0);
    if (err) {
		fprintf(stderr,"snd_rawmidi_open %s failed: %d\n",device_out,err);
    }
  }


  /*
	to read the config file:

    char *option;
    char *value;

	fscanf( "%20[^#=]s=%80[^\n]", &option, &value );


  */


  FD_SET(fd, &rfds);
  while ( 1 ) {
    res = select(fd + 1, &rfds, NULL, NULL, NULL);
    if ( -1 == res && EINTR == errno ) {
      continue;
    }
    if ( -1 == res ) {
      perror("select() failed");
      fprintf(stderr, "failed to select, fd is %d\n", fd);
      return -1;
    }
    if ( FD_ISSET(fd, &rfds) ) {
//      fprintf(stdout, "got some data\n");
      res = read(fd, read_buffer, sizeof(read_buffer));
      if ( -1 == res) {
        fprintf(stderr, "error reading data\n");
        return -1;
      }
//      fprintf(stdout, "got %d bytes\n", res);
      numevs = ( res / sizeof(struct input_event) ); /* get how many input events we got */
//      fprintf(stdout, "got %u events\n", numevs);
      for ( c = 0; c < numevs; c++ ) {
        currev = (struct input_event *)(read_buffer + (sizeof(struct input_event) * c));
//        fprintf(stdout, "event time %ld/%ld\n", currev->time.tv_sec, currev->time.tv_usec);
//        fprintf(stdout, "event type = %hd, code = %hd, value = %d\n", currev->type, currev->code, currev->value);
		if ( 1 == currev->type ) {
			if ( 1 == currev->value ) { // press
				fprintf(stdout, " %hd \n", currev->code);

				// note on
				ch=0x90; 			snd_rawmidi_write(handle_out,&ch,1);
				ch=currev->code;	snd_rawmidi_write(handle_out,&ch,1);
				ch=127;    			snd_rawmidi_write(handle_out,&ch,1);
				// controller up
				ch=0xB0; 			snd_rawmidi_write(handle_out,&ch,1);
				ch=currev->code;	snd_rawmidi_write(handle_out,&ch,1);
				ch=127;    			snd_rawmidi_write(handle_out,&ch,1);

				if ( 1 == cStates[currev->code] ) { // toggle was on already
					cStates[currev->code] = 0;
					// send a note-off
					ch=0x91;			snd_rawmidi_write(handle_out,&ch,1);
					ch=currev->code;	snd_rawmidi_write(handle_out,&ch,1);
					ch=0;    			snd_rawmidi_write(handle_out,&ch,1);				
					// and a controller down
					ch=0xB1;			snd_rawmidi_write(handle_out,&ch,1);
					ch=currev->code;	snd_rawmidi_write(handle_out,&ch,1);
					ch=0;    			snd_rawmidi_write(handle_out,&ch,1);				
				} else { // toggle was off or uninitialised
					cStates[currev->code] = 1;
					// send a note-on
					ch=0x91;			snd_rawmidi_write(handle_out,&ch,1);
					ch=currev->code;	snd_rawmidi_write(handle_out,&ch,1);
					ch=127;    			snd_rawmidi_write(handle_out,&ch,1);				
					// and controller up 
					ch=0xB1;			snd_rawmidi_write(handle_out,&ch,1);
					ch=currev->code;	snd_rawmidi_write(handle_out,&ch,1);
					ch=127;    			snd_rawmidi_write(handle_out,&ch,1);				
				}

				snd_rawmidi_drain(handle_out);

			} else if ( 2 == currev->value ) { // repeat
				fprintf(stdout, "*%hd*\n", currev->code);
			} else if ( 0 == currev->value ) { // release
				fprintf(stdout, "[%hd]\n", currev->code);


				// note off
				ch=0x90; 			snd_rawmidi_write(handle_out,&ch,1);
				ch=currev->code;   	snd_rawmidi_write(handle_out,&ch,1);
				ch=0;  				snd_rawmidi_write(handle_out,&ch,1);
				// controller down
				ch=0xB0; 			snd_rawmidi_write(handle_out,&ch,1);
				ch=currev->code;   	snd_rawmidi_write(handle_out,&ch,1);
				ch=0;  				snd_rawmidi_write(handle_out,&ch,1);

				snd_rawmidi_drain(handle_out);
				// sleep(1);

			} else { // dunno
				fprintf(stdout, "?%hd? - What is value %d?\n", currev->code, currev->value);
			}
		}
      } 
    } else {
      fprintf(stderr, "odd ... no data and we only listen in 1 fd\n");
    }
  }
  return 0;
}
