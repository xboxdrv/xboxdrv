#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/module.h>
#include <linux/usb/input.h>

#define DRIVER_AUTHOR "Ingo Ruhnke <grumbel@gmx.de>"
#define DRIVER_DESC "Xboxdrv kernel module"

static struct usb_device_id xboxdrv_table [] = {
  //{ USB_DEVICE( 0x045e, 0x028e ) },
  { USB_DEVICE_INTERFACE_PROTOCOL( 0x045e, 0x028e, 1 ) },
  { USB_DEVICE_INTERFACE_PROTOCOL( 0x045e, 0x028e, 2 ) },
  { }
};

MODULE_DEVICE_TABLE (usb, xboxdrv_table);

struct usb_xboxdrv {
  struct usb_device* udev;

  struct urb*    controller_urb;
  unsigned char* controller_idata;
  dma_addr_t     controller_idata_dma;

  struct urb*    chatpad_urb;
  unsigned char* chatpad_idata;
  dma_addr_t     chatpad_idata_dma;

  struct delayed_work worker;

  int flip_flop;
};

static void xboxdrv_chatpad_cb(struct urb *urb)
{
  printk(KERN_INFO "xboxdrv_chatpad_cb()\n");
  switch (urb->status) {
    case 0:
      {
        int i = 0;
        printk(KERN_INFO "xboxdrv_chatpad_cb(): ");
        for(i = 0; i < urb->actual_length; ++i)
        {
          printk("0x%02x ", (int)(((unsigned char*)urb->transfer_buffer)[i]));
        }
        printk("\n");
      }
      break;

    case -ECONNRESET:
    case -ENOENT:
    case -ESHUTDOWN:
      printk(KERN_INFO "xboxdrv_chatpad_cb(): fail1 %d\n", urb->status);
      return;

    default:
      printk(KERN_INFO "xboxdrv_chatpad_cb(): fail2 %d\n", urb->status);
      goto exit;
  }

exit:
  {
    int retval = usb_submit_urb(urb, GFP_ATOMIC);
    if (retval)
      err("%s - usb_submit_urb failed with result %d",
          __func__, retval);
  }
}

static void xboxdrv_controller_cb(struct urb *urb)
{
  printk(KERN_INFO "xboxdrv_controller_cb()\n");
  switch (urb->status) {
    case 0:
      {
        int i = 0;
        printk(KERN_INFO "xboxdrv_controller_cb(): ");
        for(i = 0; i < urb->actual_length; ++i)
        {
          printk("0x%02x ", (int)(((unsigned char*)urb->transfer_buffer)[i]));
        }
        printk("\n");
      }
      break;

    case -ECONNRESET:
    case -ENOENT:
    case -ESHUTDOWN:
      printk(KERN_INFO "xboxdrv_controller_cb(): fail1\n");
      /* this urb is terminated, clean up */
      dbg("%s - urb shutting down with status: %d", __func__, urb->status);
      return;

    default:
      printk(KERN_INFO "xboxdrv_controller_cb(): fail2\n");
      dbg("%s - nonzero urb status received: %d", __func__, urb->status);
      goto exit;
  }

exit:
  {
    int retval = usb_submit_urb(urb, GFP_ATOMIC);
    if (retval)
      err("%s - usb_submit_urb failed with result %d",
          __func__, retval);
  }
}

static void xboxdrv_setup_controller_readloop(struct usb_xboxdrv* xboxdrv, struct usb_interface *intf)
{
  printk(KERN_INFO "xboxdrv_setup_controller_readloop()\n");
  {
    struct usb_endpoint_descriptor* ep = &intf->cur_altsetting->endpoint[0].desc;

    xboxdrv->controller_urb   = usb_alloc_urb(0, GFP_ATOMIC);
    xboxdrv->controller_idata = usb_buffer_alloc(xboxdrv->udev, 32, GFP_KERNEL, 
                                                 &xboxdrv->controller_idata_dma);

    usb_fill_int_urb(xboxdrv->controller_urb, xboxdrv->udev,
                     usb_rcvintpipe(xboxdrv->udev, ep->bEndpointAddress),
                     xboxdrv->controller_idata, 32,
                     xboxdrv_controller_cb, xboxdrv, ep->bInterval);

    xboxdrv->controller_urb->transfer_dma = xboxdrv->controller_idata_dma;
    xboxdrv->controller_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    {
      int retval = usb_submit_urb(xboxdrv->controller_urb, GFP_ATOMIC);
      if (retval)
        err("%s - usb_submit_urb failed with result %d",
            __func__, retval);
    }
  }
}

static void xboxdrv_setup_chatpad_readloop(struct usb_xboxdrv* xboxdrv, struct usb_interface *intf)
{
  printk(KERN_INFO "xboxdrv_setup_chatpad_readloop()\n");
  printk(KERN_INFO "chatpad: num endpoints: %d", (int)intf->cur_altsetting->desc.bNumEndpoints);

  {
    struct usb_device* udev = interface_to_usbdev(intf);
    struct usb_endpoint_descriptor* ep = &intf->cur_altsetting->endpoint[0].desc;

    xboxdrv->chatpad_urb   = usb_alloc_urb(0, GFP_ATOMIC);
    xboxdrv->chatpad_idata = usb_buffer_alloc(xboxdrv->udev, 32, GFP_KERNEL, 
                                              &xboxdrv->chatpad_idata_dma);

    usb_fill_int_urb(xboxdrv->chatpad_urb, udev,
                     usb_rcvintpipe(udev, ep->bEndpointAddress),
                     xboxdrv->chatpad_idata, 32,
                     xboxdrv_chatpad_cb, xboxdrv, ep->bInterval);

    xboxdrv->chatpad_urb->transfer_dma = xboxdrv->chatpad_idata_dma;
    xboxdrv->chatpad_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    {
      int retval = usb_submit_urb(xboxdrv->chatpad_urb, GFP_ATOMIC);
      if (retval)
        err("%s - usb_submit_urb failed with result %d",
            __func__, retval);
    }
  }
}

static void xboxdrv_send_ctrl_msg(struct usb_xboxdrv* xboxdrv, int value)
{
  int retval = usb_control_msg(xboxdrv->udev, usb_sndctrlpipe(xboxdrv->udev, 0),
                               USB_REQ_GET_STATUS,
                               USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
                               value,
                               2,
                               NULL, 0,
                               0 /* timeout */);

  printk(KERN_INFO "usb_control_msg(): %d\n", (int)retval);
}

static void xboxdrv_chatpad_keepalive(struct work_struct *work)
{
  struct usb_xboxdrv* xboxdrv =  container_of(work, struct usb_xboxdrv, worker.work);

  if (xboxdrv->flip_flop)
  {
    printk(KERN_INFO "xboxdrv_sending: 0x1f()\n");
    //xboxdrv_send_ctrl_msg(xboxdrv, 0x1f);
  }
  else
  {
    printk(KERN_INFO "xboxdrv_sending: 0x1e()\n");
  }

  xboxdrv->flip_flop = !xboxdrv->flip_flop;

  schedule_delayed_work(&xboxdrv->worker, msecs_to_jiffies(1000));
}

static void xboxdrv_setup_chatpad_keepalive(struct usb_xboxdrv* xboxdrv, struct usb_interface *intf)
{
  printk(KERN_INFO "xboxdrv_setup_chatpad_keepalive()\n");
  {
    xboxdrv_send_ctrl_msg(xboxdrv, 0x1b);
    {
      INIT_DELAYED_WORK(&xboxdrv->worker, xboxdrv_chatpad_keepalive);
    
      schedule_delayed_work(&xboxdrv->worker, msecs_to_jiffies(1000));
    }
  }
}

static int xboxdrv_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
  struct usb_device *udev = interface_to_usbdev(intf);

  printk(KERN_INFO "xboxdrv_probe() called: %04x:%04x - %d - %d %d %d\n",
         (int)id->idVendor, (int)id->idProduct, intf->minor,
         (int)intf->cur_altsetting->desc.bInterfaceClass, 
         (int)intf->cur_altsetting->desc.bInterfaceSubClass, 
         (int)intf->cur_altsetting->desc.bInterfaceProtocol);
  
  if (intf->cur_altsetting->desc.bInterfaceProtocol == 1) // controller
  {
    printk(KERN_INFO "found xboxdrv controller\n");
    {
      struct usb_xboxdrv* xboxdrv = kzalloc(sizeof(struct usb_xboxdrv), GFP_KERNEL);
      xboxdrv->udev = udev;
      usb_set_intfdata(intf, xboxdrv);

      xboxdrv_setup_controller_readloop(xboxdrv, intf);
    }
    return 0;
  }
  else if (intf->cur_altsetting->desc.bInterfaceProtocol == 2) // chatpad
  {
    printk(KERN_INFO "found xboxdrv chatpad\n");
    {
      struct usb_xboxdrv* xboxdrv = kzalloc(sizeof(struct usb_xboxdrv), GFP_KERNEL);
      xboxdrv->udev = udev;
      usb_set_intfdata(intf, xboxdrv);
    
      xboxdrv_setup_chatpad_readloop(xboxdrv, intf);
      xboxdrv_setup_chatpad_keepalive(xboxdrv, intf);
    }
    return 0;
  }
  else
  {
    // unknown
    return -ENODEV;
  }
}

static void xboxdrv_disconnect(struct usb_interface *intf)
{
  printk(KERN_INFO "xboxdrv_disconnect()\n");
  {
    struct usb_xboxdrv* xboxdrv = usb_get_intfdata(intf);
    usb_set_intfdata(intf, NULL);

    cancel_delayed_work(&xboxdrv->worker);
    flush_delayed_work(&xboxdrv->worker);

    kfree(xboxdrv);
  }
}

static struct usb_driver xboxdrv_driver = {
  .name		= "xboxdrv",
  .probe        = xboxdrv_probe,
  .disconnect	= xboxdrv_disconnect,
  .id_table	= xboxdrv_table,
};

static int __init usb_xboxdrv_init(void)
{ 
  // FIXME: called once for each interface?!
  int result = usb_register(&xboxdrv_driver);

  printk(KERN_INFO "usb_xboxdrv_init() called\n");

  if (result == 0)
    printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_DESC "\n");

  return result;
}

static void __exit usb_xboxdrv_exit(void)
{
  printk(KERN_INFO "usb_xboxdrv_exit() called\n");
  usb_deregister(&xboxdrv_driver);
}

module_init(usb_xboxdrv_init);
module_exit(usb_xboxdrv_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

/* EOF */
