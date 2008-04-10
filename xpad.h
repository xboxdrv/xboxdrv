/*
 * Xbox Controller driver for Linux - v0.1.5
 *
 *	header file containing ioctl definitions
 *
 * Copyright (c)  2003  Marko Friedemann <mfr@bmx-chemnitz.de>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
 
#ifndef __XPAD_h
#define __XPAD_h


/*********** ioctl stuff, can be used outside of the driver ***********/
#define USB_XPAD_IOC_MAGIC 	'x'

#define USB_XPAD_IOCRESET 	_IO(  USB_XPAD_IOC_MAGIC, 0 )
#define USB_XPAD_IOCSRUMBLE 	_IOW( USB_XPAD_IOC_MAGIC, 3, int )
#define USB_XPAD_IOCGRUMBLE 	_IOR( USB_XPAD_IOC_MAGIC, 4, int )

#define USB_XPAD_IOCSIR 	_IOW( USB_XPAD_IOC_MAGIC, 5, int )
#define USB_XPAD_IOCGIR 	_IOR( USB_XPAD_IOC_MAGIC, 6, int )

#define USB_XPAD_IOC_MAXNR 	6

/************************* driver internals ***************************/
#ifdef __KERNEL__

#include <linux/input.h>
#include <linux/circ_buf.h>

/****************** driver description and version ********************/
#define DRIVER_VERSION		"v0.1.7"
#define DRIVER_AUTHOR		"Marko Friedemann <mfr@bmx-chemnitz.de>,\
 Oliver Schwartz <Oliver.Schwartz@gmx.de>, Georg Lukas <georg@op-co.de>,\
 Thomas Pedley <Gentoox@shallax.com>, Edgar Hucek <hostmaster@ed-soft.at>"

#define DRIVER_DESC		"driver for Xbox controllers"

/****************************** constants *****************************/
#define XPAD_MAX_DEVICES	4
#define XPAD_PKT_LEN		32	/* input packet size */
#define XPAD_PKT_LEN_FF		6	/* output packet size - rumble */

#define XPAD_TX_BUFSIZE		XPAD_PKT_LEN_FF * 8	/* max. 8 requests */

/************************* the device struct **************************/
struct usb_xpad {
	struct input_dev *dev;			/* input device interface */
	struct usb_device *udev;		/* usb device */

	struct urb *irq_in;			/* urb for int. in report */
	unsigned char *idata;			/* input data */
	dma_addr_t idata_dma;

	char phys[65];				/* physical input dev path */

	unsigned char offsetset_compensation;
	int left_offset_x;
	int left_offset_y;
	int right_offset_x;
	int right_offset_y;

	int isMat;				/* is this a dancepad/mat? */
	int is360;				/* is this a Xbox 360 Controller */
	int isWireless;			/* is this a Xbox 360 _Wireless_ Controller
						   NOTE: if this is set, is360 should be also */
	int isConnected;			/* is this controller connected */

#ifdef CONFIG_USB_XPAD_RUMBLE
	int rumble_enabled;			/* ioctl can toggle rumble */

	int ep_out_adr;				/* number of out endpoint */
	unsigned char tx_data[XPAD_PKT_LEN_FF];	/* output data (rumble) */
	int strong_rumble, play_strong;		/* strong rumbling */
	int weak_rumble, play_weak;		/* weak rumbling */
	struct timer_list rumble_timer;		/* timed urb out retry */
	wait_queue_head_t wait;			/* wait for URBs on queue */

	spinlock_t tx_lock;
	struct circ_buf tx;
	unsigned char tx_buf[XPAD_TX_BUFSIZE];
	long tx_flags[1];			/* transmit flags */
#endif
};

/* for the list of know devices */
struct xpad_device {
	u16 idVendor;
	u16 idProduct;
	char *name;
	u8  type;
};

/* gamepad type */
#define GAMEPAD_XBOX			0
#define GAMEPAD_XBOX_MAT		1
#define GAMEPAD_XBOX360			2
#define GAMEPAD_XBOX360_WIRELESS	3


/************************ rumble function stubs ***********************/
#ifndef CONFIG_USB_XPAD_RUMBLE
 #define xpad_rumble_ioctl(dev, cmd, arg) -ENOTTY
 #define xpad_rumble_open(xpad) {}
 #define xpad_rumble_probe(udev, xpad, ifnum) 0
 #define xpad_rumble_close(xpad) {}
 #define xpad_rumble_disconnect(xpad) {}
#else /* CONFIG_USB_XPAD_RUMBLE */

 #define XPAD_TX_RUNNING	0
 #define XPAD_TX_INC(var, n)	(var) += n; (var) %= XPAD_TX_BUFSIZE

 #ifndef __USB_XPAD_RUMBLE
  extern int  xpad_rumble_ioctl(struct input_dev *dev, unsigned int cmd, unsigned long arg);
  extern void xpad_rumble_open(struct usb_xpad *xpad);
  extern int  xpad_rumble_probe(struct usb_device *udev, struct usb_xpad *xpad, unsigned int ifnum);
  extern void xpad_rumble_close(struct usb_xpad *xpad);
  extern void xpad_rumble_disconnect(struct usb_xpad *xpad);
 #endif /* __USB_XPAD_RUMBLE */
#endif /* CONFIG_USB_XPAD_RUMBLE */

#endif /* __KERNEL__ */

#endif /* __XPAD_h */
