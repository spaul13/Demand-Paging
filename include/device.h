/* device.h - isbaddev */

#define	DEVNAMLEN	16	/* maximum size of a device name	*/

/* Macro used to verify device ID is valid  */

#define isbaddev(f)  ( ((f) < 0) | ((f) >= NDEVS) )

#ifndef QEMU
#define WCONSOLE CONSOLE
#define WRDISK RDISK
#else
#define WCONSOLE QEMUCONSOLE
#define WRDISK QEMURDISK
#endif
