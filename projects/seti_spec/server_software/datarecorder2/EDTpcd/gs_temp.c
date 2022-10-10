#include "edtinc.h"

#include "lib_adt7461.h"

#ifndef WIN32

#ifdef __linux__

#include <asm/ioctls.h>

#include <curses.h>


#elif defined(VXWORKS)

#include "ioLib.h"

#else

#include <sys/filio.h>		/* for kbhitedt() */

#endif

int kbhitedt()
{
    int numchars, n, ch ;
    ioctl(0, FIONREAD, (int)&numchars) ;

    /* for (n=0; n<numchars; n++)  ch=getchar() ;*/
    return(numchars);
}
#else
/* #include <conio.h> */
int _kbhit();

int getch();

int kbhitedt()
{
    return(_kbhit()) ;
}
#endif /* _NT_ */



void
adt7461_usage(char *s)
{
    printf("Usage: %s\n", s) ;
    printf("-u unit		unit to test\n") ;
	printf("-s			single sample\n");
	printf("-a <value>  set alert high value\n");
	printf("-e			get the alert event\n");
	printf("-t <value>  set THERM value in degrees\n");
	printf("-r <1 or 0> set / unset THERM reset flag\n");
	printf("-h <value>  set THERM hysteresis value\n");
	
}

int
got_alert(void *p)

{
	EdtDev *edt_p = (EdtDev *) p;

	printf("Got Alert Temp = %d\n", edt_adt7461_read_extern(edt_p, 0));
	return 0;
}

int
main(int argc, char **argv)
{
	EdtDev *edt_p ;
	int unit = 0 ;
	int program_alive = 1;
	int loop_delay = 1000; /* 1 second */
	int status_once				= 0,
		status_always			= 0,
		external_limits_once	= 0,
		external_limits_always	= 0;
	int temp1, temp2, key;
	int alert ;
	int alert_last = 0;
	int loop_once = 0;

	int alert_high = -1;
	int therm_high = -1;
	int reset_therm = -1;
	int hysteresis = -1;

	int do_event = 0;

    char *program = argv[0];

	--argc;
	++argv;
	while (argc	&& argv[0][0] == '-')
	{
		switch (argv[0][1])
		{
			case 'u':
				++argv;
				--argc;
				unit = atoi(argv[0]);
				break;
			case 'e':
				do_event  = !do_event;
				break;
			case 's':
				program_alive = 0;
				break;
			case 't':
				++argv;
				--argc;
				therm_high = atoi(argv[0]);
				break;
			case 'a':
				++argv;
				--argc;
				alert_high = atoi(argv[0]);
				break;
			case 'r':
				++argv;
				--argc;
				reset_therm = atoi(argv[0]);
				break;
			case 'h':
				++argv;
				--argc;
				hysteresis = atoi(argv[0]);
				break;
				
			default:
				adt7461_usage(program);
				exit(0);
		}
		argc--;
		argv++;
	}

	if ((edt_p = edt_open(EDT_INTERFACE, unit)) == NULL)
	{
		char str[64] ;
		sprintf(str, "%s%d", EDT_INTERFACE, unit) ;
		perror(str) ;
		exit(1) ;
	}

	if (do_event) 
		printf("Ret from set_event_func %d\n",
			edt_set_event_func(edt_p, EDT_EVENT_TEMP,
		(EdtEventFunc) got_alert, edt_p, 0));

	edt_adt7461_send_bus_reset(edt_p);

	if (alert_high != -1)
		edt_adt7461_set_alert_high(edt_p, alert_high);

	if (therm_high != -1)
		edt_adt7461_set_therm(edt_p, therm_high);

	if (reset_therm != -1)
		edt_adt7461_set_therm_reset(edt_p, reset_therm);

	if (hysteresis != -1)
		edt_adt7461_set_therm_hysteresis(edt_p, hysteresis);


	edt_adt4761_print_status(edt_p, 1);
			
	while(program_alive) {
		/* print out updated information */

		printf("\n");
		/* wait delay period */
		/* handle key presses */
		while (kbhitedt()) {
			key = getch();
			switch (key) {
				case 'a': /* alert address read */
					temp1 = edt_adt7461_read_alert_address(edt_p);
					if (temp1 != -1) {
						printf("  Device Addressed 0x%02x Caused an Alert\n", temp1 >> 1);
					} else printf("  No Devices Responded\n");
					alert = 0;
					break;
				case 'w': /* Direct write */
					printf("  Set Register (hex): ");
					fscanf(stdin, "%x", &temp1);
					printf("  Set Reg 0x%02x (hex): ", temp1);
					fscanf(stdin, "%x", &temp2);
					edt_adt7461_reg_write(edt_p, temp1, temp2);
					break;
				case 'r': /* direct read */
					printf("  Read Register (hex): ");
					fscanf(stdin, "%x", &temp1);
					temp2 = edt_adt7461_reg_read(edt_p, temp1);
					printf("  Reg 0x%02x contains 0x%02x (%i)\n", temp1, temp2, temp2);
					break;
				case 'H': /* High temp set */
					printf("  Set High Temperature Limit: ");
					fscanf(stdin, "%i", &temp1);
					edt_adt7461_reg_write(edt_p, EDT_ADT7461_EXT_TEMP_HIGH_W, temp1);
					break;
				case 'L': /* Low temp set */
					printf("  Set Low Temperature Limit: ");
					fscanf(stdin, "%i", &temp1);
					edt_adt7461_reg_write(edt_p, EDT_ADT7461_EXT_TEMP_LOW_W, temp1);
					break;

				case 'U': /* Update rate change */
					printf("  Set Update Rate (%ims): ", loop_delay);
					fscanf(stdin, "%i", &loop_delay);
					break;
				case 't': /* toggle adt7461 reset */
					{
						int treset = !edt_adt7461_get_therm_reset(edt_p);
						printf("THERM reset  %s\n", (treset)?"ON":"OFF");
						edt_adt7461_set_therm_reset(edt_p, treset);
					}

					break;
				case 'p': /* pause until keypress */
					printf("  Paused");
					getch();
					printf("\n");
					break;
				case 'h': /* help */
					printf("HELP!\n");/* TODO: complete */
					break;
				case 'q': /* quit program */
					program_alive = 0;
					break;
				default:
					printf("  Unknown command: %c\n", key);
			}
		}
		if (program_alive)
			edt_msleep(loop_delay);
		edt_adt4761_print_status(edt_p, 0);
	}

	return(0) ;
}

