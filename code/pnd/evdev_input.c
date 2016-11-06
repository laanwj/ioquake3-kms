#include "../client/client.h"
#include "../renderer/tr_local.h"
#include "../qcommon/q_shared.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/input.h>

/* See linux/input-event-codes.h */
static keyNum_t keymap[128] =
{
    0,			/* 0 - Reserved */
    K_ESCAPE,		/* 1 - Escape */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', /* 2..13 */
    K_BACKSPACE,	/* 14 - Backspace */
    K_TAB,		/* 15 - Tab */
    'q', 'w', 'e', 'r',	't', 'y', 'u', 'i', 'o', 'p', '[', ']', /* 16..27 */
    K_ENTER,		/* 28 - Enter */
    K_CTRL,		/* 29 - Left Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', K_CONSOLE, /* 30..41 */
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
static keyNum_t keymap_shift[128] =
{
    0,			/* 0 - Reserved */
    K_ESCAPE,		/* 1 - Escape */
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', /* 2..13 */
    K_BACKSPACE,	/* 14 - Backspace */
    K_TAB,		/* 15 - Tab */
    'Q', 'W', 'E', 'R',	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', /* 16..27 */
    K_ENTER,		/* 28 - Enter */
    K_CTRL,		/* 29 - Left Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', K_CONSOLE, /* 30..41 */
    K_SHIFT,		/* 42 - Left shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',	'M', '<', '>', '?', /* 43..53 */
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

#define MAX_INPUT_FDS 16

static int input_fds[MAX_INPUT_FDS];
static int num_input_fds = 0;
static int shift_pressed = 0;

static void IN_Setup_Controls(void);
static void IN_Close_Controls(void);
static void IN_SendAllEvents(void);
static void IN_CheckEvent(struct input_event *event, int device);
static int IN_OpenEventDeviceByPath(const char *path);

static void IN_Setup_Controls(void)
{
	char event_name[100];
	struct dirent entry;
	struct dirent *result;

	printf("Setting up Evdev Controls\n");

	/* Try to open all input devices */
	DIR* dirp = opendir("/dev/input");
	if (dirp == NULL) {
		printf("Could not open /dev/input directory\n");
		return;
	}
        num_input_fds = 0;

	while (!readdir_r(dirp, &entry, &result) && result != NULL && num_input_fds < MAX_INPUT_FDS) {
		if (strncmp(result->d_name, "event", 5) ==  0) {
			if (snprintf(event_name, sizeof(event_name), "/dev/input/%s", result->d_name) >= sizeof(event_name)) {
				printf("Input device name would overflow buffer: %s\n", result->d_name);
			}
			int fd = IN_OpenEventDeviceByPath(event_name);
			if (fd >= 0)
				input_fds[num_input_fds++] = fd;
		}
	}

	closedir(dirp);

        shift_pressed = 0;
}

static void IN_Close_Controls(void)
{
	int i;
	printf("Closing Evdev Controls\n");

	for (i = 0; i < num_input_fds; ++i) {
		close(input_fds[i]);
	}
}

static void IN_SendAllEvents(void)
{
	int i, j, rd;
	struct input_event ev[64];

	for (j = 0; j < num_input_fds; ++j) {
		if ((rd = read(input_fds[j], ev, sizeof(struct input_event) * 64)) >= 0)
		{
			for (i = 0; i < rd / sizeof(struct input_event); i++)
			{
				IN_CheckEvent(&ev[i], j);
			}
		}
	}
}

static void IN_CheckEvent( struct input_event *event, int device )
{
	int sym, sym_shift, value;
	long rel_x, rel_y;

	//printf( "Device %d Type %d Code %d Value %d\n", device, event->type, event->code, event->value );

	rel_x	= 0;
	rel_y	= 0;
	sym	= 0;
        sym_shift = 0;
	value	= event->value;
	switch(event->type)
	{
	case EV_KEY:
		switch(event->code)
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
		case BTN_SIDE:
		case BTN_FORWARD:
			sym = K_MOUSE4;
			break;
		case BTN_EXTRA:
		case BTN_BACK:
			sym = K_MOUSE5;
			break;
		default:
			if( event->code < 128 && keymap[event->code]>0 ) {
				sym = keymap[event->code];
                                sym_shift = keymap_shift[event->code];
                        }
		}
		break;
	case EV_REL:
		switch(event->code)
		{
		case REL_X:
			rel_x = value;
			break;
		case REL_Y:
			rel_y = value;
			break;
		}
		break;
	case EV_ABS:
		// How to handle absolute events?
		break;
	}

	if(sym > 0) {
		Com_QueueEvent(0, SE_KEY, sym, value, 0, NULL);
                if (sym == K_SHIFT) {
                    shift_pressed = value;
                }
	}
	if(sym > 0 && sym < 128 && value) {
		Com_QueueEvent(0, SE_CHAR, shift_pressed ? sym_shift : sym, 0, 0, NULL);
	}
	if(rel_x != 0 || rel_y != 0) {
		Com_QueueEvent(0, SE_MOUSE, rel_x, rel_y, 0, NULL);
	}

}

static int IN_OpenEventDeviceByPath( const char *event_name )
{
	int fd;
	int version;
	unsigned short id[4];
	char dev_name[256] = "Unknown";

	printf("Device: %s\n", event_name);
	if ((fd = open(event_name, O_RDONLY | O_NDELAY)) < 0) {
		perror("ERROR: Could not open device");
		return -1;
	}

	if (ioctl(fd, EVIOCGVERSION, &version)) {
		perror("evtest: can't get version");
		return -1;
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


void IN_Frame(void)
{
	IN_SendAllEvents();
}

void IN_Init(void)
{
	Com_DPrintf("\n------- Input Initialization -------\n");

	IN_Setup_Controls();

	Com_DPrintf("------------------------------------\n");
}

void IN_Shutdown(void)
{
	IN_Close_Controls();
}

void IN_Restart(void)
{
	IN_Close_Controls();
	IN_Init();
}
