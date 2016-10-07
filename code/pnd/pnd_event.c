#include "pnd_event.h"
#include "pnd_type.h"

#include "../qcommon/q_shared.h"
#include "../client/keycodes.h"
#include "../qcommon/qcommon.h"

cvar_t* cvarKey;
cvar_t* cvarMouse;

/* See linux/input-event-codes.h */
keyNum_t keymap[128] =
{
    0,			/* 0 - Reserved */
    K_ESCAPE,		/* 1 - Escape */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', /* 2..13 */
    K_BACKSPACE,	/* 14 - Backspace */
    K_TAB,		/* 15 - Tab */
    'q', 'w', 'e', 'r',	't', 'y', 'u', 'i', 'o', 'p', '[', ']', /* 16..27 */
    K_ENTER,		/* 28 - Enter */
    K_CTRL,		/* 29 - Left Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', /* 30..41 */
    K_SHIFT,		/* 42 - Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',	'm', ',', '.', '/', /* 43..53 */
    K_SHIFT,		/* 54 - Right shift */
    K_KP_STAR,		/* 55 - Keypad asterisk */
    K_ALT,		/* 56 - Left alt */
    K_SPACE,		/* 57 - Space bar */
    K_CAPSLOCK,		/* 58 - Caps lock */
    K_F1, K_F2, K_F3, K_F4, K_F5, K_F6, K_F7, K_F8, K_F9, K_F10, /* 59..68 */
    K_KP_NUMLOCK,	/* 69 - Num lock*/
    K_SCROLLOCK,	/* 70 - Scroll Lock */
    K_KP_HOME,		/* 71 - Keypad Home key / 7 */
    K_KP_UPARROW,	/* 72 - Keypad Up Arrow / 8 */
    K_KP_PGUP,		/* 73 - Keypad Page Up / 9 */
    K_KP_MINUS,		/* 74 - Keypad - */
    K_KP_LEFTARROW,	/* 75 - Keypad Left Arrow / 4 */
    K_KP_5,		/* 76 - Keypad 5 */
    K_KP_RIGHTARROW,	/* 77 - Keypad Right Arrow / 6 */
    K_KP_PLUS,		/* 78 - Keypad + */
    K_KP_END,		/* 79 - Keypad End key / 1 */
    K_KP_DOWNARROW,	/* 80 - Keypad Down Arrow / 2 */
    K_KP_PGDN,		/* 81 - Keypad Page Down / 3 */
    K_KP_INS,		/* 82 - Keypad Insert Key / 0 */
    K_KP_DEL,		/* 83 - Keypad Delete Key / . */
    0,   0,   0,	/* 84, 85, 86 */
    K_F11,		/* 87 - F11 Key */
    K_F12,		/* 88 - F12 Key */
    0, 0, 0, 0, 0, 0, 0,/* 89 - 95 */
    K_KP_ENTER,		/* 96 - Keypad Enter */
    K_CTRL,		/* 97 - Right Control */
    K_KP_SLASH,		/* 98 - Keypad Slash */
    0,			/* 99 - Sysrq */
    K_ALT,		/* 100 - Right Alt */
    10,			/* 101 - Line Feed */
    K_HOME,		/* 102 - Home */
    K_UPARROW,		/* 103 - Up */
    K_PGUP,		/* 104 - Page Up */
    K_LEFTARROW,	/* 105 - Left */
    K_RIGHTARROW,	/* 106 - Right */
    K_END,		/* 107 - End */
    K_DOWNARROW,	/* 108 - Down */
    K_PGDN,		/* 109 - Page Down */
    K_INS,		/* 110 - Insert */
    K_DEL,		/* 111 - Delete */
    0,			/* 112 - Macro */
    0,			/* 113 - Mute */
    0,			/* 114 - Volume Down */
    0,			/* 115 - Volume Up */
    K_POWER,		/* 116 - Power */
    K_KP_EQUALS,	/* 117 - Keypad Equals */
    0,			/* 118 - Keypad Plusminus */
    K_PAUSE,		/* 119 - Pause */
    0,			/* 120 - Scale */
    0,			/* 121 - Keypad Comma */
    0,			/* 122 - Hangeul */
    0,			/* 123 - Hanja */
    0,			/* 124 - Yen */
    K_SUPER,		/* 125 - Left Meta */
    K_SUPER,		/* 126 - Right Meta */
    K_COMPOSE		/* 127 - Compose */
};

char event_name[30];
int fd_usbk, fd_usbm, rd, i, j, k;
struct input_event ev[64];
int version;
unsigned short id[4];
unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
char dev_name[256] = "Unknown";
int absolute[5];
int prevtime;

#define DEV_USBK 5
#define DEV_USBM 6

int mx, my, buttonstate, lasttime;

void PND_Setup_Controls( void )
{
	int event_key = 0;
	int event_mouse = 0;

	printf( "Setting up Evdev Controls\n" );

	cvarKey = Cvar_Get("usbk", "0", 0);
	cvarMouse = Cvar_Get("usbm", "0", 0);

	event_key = cvarKey->value;
	event_mouse = cvarMouse->value;

	// Dynamic Controls
	// USB keyboard
	if( event_key > 0 ) {
		fd_usbk = PND_OpenEventDeviceByID(event_key);
	} else {
		printf( "No device selected for USB keyboard\n" );
	}
	// USB mouse
	if( event_mouse > 0 ) {
		fd_usbm = PND_OpenEventDeviceByID(event_mouse);
	} else {
		printf( "No device selected for USB mouse\n" );
	}
}

void PND_Close_Controls( void )
{
	printf( "Closing Evdev Controls\n" );

	if( fd_usbk > 0 )
		close(fd_usbk );
	if( fd_usbm > 0 )
		close(fd_usbm );
}

void PND_SendAllEvents( void )
{
	// Read the events from the controls
	PND_SendKeyEvents();
	PND_SendRelEvents();
	PND_SendAbsEvents();
}

void PND_SendKeyEvents( void )
{
	PND_ReadEvents( fd_usbk, DEV_USBK );
}

void PND_SendRelEvents( void )
{
	PND_ReadEvents( fd_usbm, DEV_USBM );

	if ( (mx != 0 || my != 0) && (Sys_Milliseconds() - lasttime > 5) )
	{
		Com_QueueEvent( 0, SE_MOUSE, mx, my, 0, NULL ); 
	}
	lasttime = Sys_Milliseconds();
}

void PND_SendAbsEvents( void )
{
}

void PND_ReadEvents( int fd, int device )
{
	if( fd != 0 )
	{
		rd = read(fd, ev, sizeof(struct input_event) * 64);

		if (rd > (int) sizeof(struct input_event))
		{
			for (i = 0; i < rd / sizeof(struct input_event); i++)
			{
				PND_CheckEvent( &ev[i], device );
			}
		}
	}
}

void PND_CheckEvent( struct input_event *event, int device )
{
	int sym, value;
	long rel_x, rel_y;

	//printf( "Device %d Type %d Code %d Value %d\n", device, event->type, event->code, event->value );

	rel_x	= 0;
	rel_y	= 0;
	sym	= 0;
	value	= event->value;
	switch( event->type )
	{
		case EV_KEY:
			switch( event->code )
			{
				case BTN_LEFT:
					sym = K_MOUSE1;
					break;
				case BTN_RIGHT:
					sym = K_MOUSE2;
					break;
				case BTN_MIDDLE:
					sym = K_MOUSE3;
					break;
                                // Could handle more mouse buttons here
				default:
					if( event->code < 128 && keymap[event->code]>0 )
						sym = keymap[event->code];
					break;
			}
			break;
		case EV_REL:
			switch( device )
			{
				case DEV_USBM:
					switch( event->code )
					{
						case REL_X:
							rel_x = value;
							break;
						case REL_Y:
							rel_y = value;
							break;
					}
					break;
			}
			break;
		case EV_ABS:
			break;
	}

	if( sym > 0 ) {
		Com_QueueEvent(0, SE_KEY, sym, value, 0, NULL);
	}

	if( rel_x != 0 || rel_y != 0 )
	{
		Com_QueueEvent( 0, SE_MOUSE, rel_x, rel_y, 0, NULL ); 
	}

}

int PND_OpenEventDeviceByID( int event_id )
{
	int fd;

	snprintf( event_name, sizeof(event_name), "/dev/input/event%d", event_id );
	printf( "Device: %s\n", event_name );
	if ((fd = open(event_name, O_RDONLY |  O_NDELAY)) < 0) {
		perror("ERROR: Could not open device");
		return 0;
	}

	if (ioctl(fd, EVIOCGVERSION, &version)) {
		perror("evtest: can't get version");
		return 0;
	}

	printf("Input driver version is %d.%d.%d\n",
		version >> 16, (version >> 8) & 0xff, version & 0xff);

	ioctl(fd, EVIOCGID, id);
	printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n",
		id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);

	ioctl(fd, EVIOCGNAME(sizeof(dev_name)), dev_name);
	printf("Input device name: \"%s\"\n", dev_name);

	return fd;
}
