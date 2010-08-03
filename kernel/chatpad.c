#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/module.h>
#include <linux/usb/input.h>

#define DRIVER_AUTHOR "Ingo Ruhnke <grumbel@gmx.de>"
#define DRIVER_DESC "Xbox360 Chatpad Linux Driver"

static struct usb_device_id chatpad_table [] = {
  { USB_DEVICE_INTERFACE_PROTOCOL( 0x045e, 0x028e, 2 ) },
  { }
};

MODULE_DEVICE_TABLE (usb, chatpad_table);

struct usb_chatpad {
  struct usb_device* udev;

  struct urb*    chatpad_urb;
  unsigned char* chatpad_idata;
  dma_addr_t     chatpad_idata_dma;

  struct usb_ctrlrequest control_cr;
  struct urb*    control_urb;

  struct delayed_work worker;

  int ctrl_ready;
  int flip_flop;
};

static void chatpad_chatpad_cb(struct urb *urb)
{
  printk(KERN_INFO "chatpad_chatpad_cb()\n");
  switch (urb->status) 
  {
    case 0:
      {
        int i = 0;
        printk(KERN_INFO "chatpad_chatpad_cb(): XXXXXXXXXXXXXXXXXXXXX ");
        for(i = 0; i < urb->actual_length; ++i)
        {
          printk("0x%02x ", (int)(((unsigned char*)urb->transfer_buffer)[i]));
        }
        printk("\n");
      }
      break;

    case -ECONNRESET:
    case -ENOENT:
    case -ESHUTDOWN: // triggered when the module get unloaded or device disconnected
      printk(KERN_INFO "chatpad_chatpad_cb(): fail1 %d\n", urb->status);
      return;

    default:
      printk(KERN_INFO "chatpad_chatpad_cb(): fail2 %d\n", urb->status);
      break;
  }

  {
    int retval = usb_submit_urb(urb, GFP_ATOMIC);
    if (retval)
      err("%s - usb_submit_urb failed with result %d",
          __func__, retval);
  }
}

static void chatpad_setup_chatpad_readloop(struct usb_chatpad* chatpad, struct usb_interface *intf)
{
  printk(KERN_INFO "chatpad_setup_chatpad_readloop()\n");
  printk(KERN_INFO "chatpad: num endpoints: %d", (int)intf->cur_altsetting->desc.bNumEndpoints);

  {
    struct usb_device* udev = interface_to_usbdev(intf);
    struct usb_endpoint_descriptor* ep = &intf->cur_altsetting->endpoint[0].desc;

    chatpad->chatpad_urb   = usb_alloc_urb(0, GFP_ATOMIC);
    chatpad->chatpad_idata = usb_buffer_alloc(chatpad->udev, 32, GFP_KERNEL, 
                                              &chatpad->chatpad_idata_dma);

    usb_fill_int_urb(chatpad->chatpad_urb, udev,
                     usb_rcvintpipe(udev, ep->bEndpointAddress),
                     chatpad->chatpad_idata, 32,
                     chatpad_chatpad_cb, chatpad, ep->bInterval);

    chatpad->chatpad_urb->transfer_dma = chatpad->chatpad_idata_dma;
    chatpad->chatpad_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    { // resubmit the urb, thus loop
      int retval = usb_submit_urb(chatpad->chatpad_urb, GFP_ATOMIC);
      if (retval)
        err("%s - usb_submit_urb failed with result %d",
            __func__, retval);
    }
  }
}

static void chatpad_send_ctrl_msg(struct usb_chatpad* chatpad, int value)
{
  printk(KERN_INFO "usb_control_msg(): try send\n");
  {
    int retval = usb_control_msg(chatpad->udev, usb_sndctrlpipe(chatpad->udev, 0),
                                 USB_REQ_GET_STATUS,
                                 USB_TYPE_VENDOR | USB_RECIP_INTERFACE,
                                 value,
                                 2,
                                 NULL, 0,
                                 0 /* timeout */); // doesn't response

    switch(retval)
    {
      case -ETIMEDOUT:
        printk(KERN_INFO "usb_control_msg(): ETIMEDOUT\n");
        break;

      case -ETIME:
        printk(KERN_INFO "usb_control_msg(): ETIME\n");
        break;

      default:
        printk(KERN_INFO "usb_control_msg(): %d\n", (int)retval);
        break;
    }
  }
}

static void chatpad_send_ctrl_msg_cb(struct urb *urb)
{
  printk(KERN_INFO "chatpad_send_ctrl_msg_cb()\n");

  switch (urb->status) 
  {
    default:
      printk(KERN_INFO "chatpad_send_ctrl_msg_cb(): status = %d\n", urb->status);
  break;
  }
}

static void chatpad_send_ctrl_msg_async(struct usb_chatpad* chatpad, int value)
{
  chatpad->control_cr.bRequestType = USB_TYPE_VENDOR | USB_RECIP_INTERFACE;
  chatpad->control_cr.bRequest     = USB_REQ_GET_STATUS;
  chatpad->control_cr.wValue       = cpu_to_le16(value);
  chatpad->control_cr.wIndex       = cpu_to_le16(2);
  chatpad->control_cr.wLength      = cpu_to_le16(0);

  // FIXME: must use different urb!
  usb_fill_control_urb(chatpad->control_urb,
                       chatpad->udev,
                       usb_sndctrlpipe(chatpad->udev, 0),
                       (unsigned char*)&chatpad->control_cr,
                       NULL, // transfer_buffer
                       0,    // buffer_length
                       chatpad_send_ctrl_msg_cb,
                       chatpad);

  {
    int retval = usb_submit_urb(chatpad->control_urb, GFP_ATOMIC);
    if (retval)
    {
      printk(KERN_INFO "RETVAL: %d\n", retval);
    }
  }
}

static void chatpad_chatpad_keepalive(struct work_struct *work)
{
  struct usb_chatpad* chatpad =  container_of(work, struct usb_chatpad, worker.work);

  if (chatpad->flip_flop)
  {
    printk(KERN_INFO "chatpad_sending: 0x1f()\n");
    chatpad_send_ctrl_msg(chatpad, 0x1f);
  }
  else
  {
    printk(KERN_INFO "chatpad_sending: 0x1e()\n");
    chatpad_send_ctrl_msg(chatpad, 0x1e);
  }

  chatpad->flip_flop = !chatpad->flip_flop;

  schedule_delayed_work(&chatpad->worker, msecs_to_jiffies(1000));
}

static void chatpad_setup_chatpad_keepalive(struct usb_chatpad* chatpad, struct usb_interface *intf)
{
  printk(KERN_INFO "chatpad_setup_chatpad_keepalive()\n");

  chatpad->control_urb = usb_alloc_urb(0, GFP_ATOMIC);
  
  // launch the worker thread that sends 0x1f, 0x1e
  INIT_DELAYED_WORK(&chatpad->worker, chatpad_chatpad_keepalive);
  schedule_delayed_work(&chatpad->worker, msecs_to_jiffies(1000));

  // activate the chatpad
  //chatpad_send_ctrl_msg(chatpad, 0x1b);
}

static int chatpad_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
  struct usb_device *udev = interface_to_usbdev(intf);

  printk(KERN_INFO "chatpad_probe() called: %04x:%04x - %d - %d %d %d\n",
         (int)id->idVendor, (int)id->idProduct, intf->minor,
         (int)intf->cur_altsetting->desc.bInterfaceClass, 
         (int)intf->cur_altsetting->desc.bInterfaceSubClass, 
         (int)intf->cur_altsetting->desc.bInterfaceProtocol);
  
  printk(KERN_INFO "found chatpad chatpad\n");
  {
    struct usb_chatpad* chatpad = kzalloc(sizeof(struct usb_chatpad), GFP_KERNEL);
    chatpad->udev = udev;
    usb_set_intfdata(intf, chatpad);
    
    chatpad_setup_chatpad_readloop(chatpad, intf);
    chatpad_setup_chatpad_keepalive(chatpad, intf);
  }
  return 0;
}

static void chatpad_disconnect(struct usb_interface *intf)
{
  printk(KERN_INFO "chatpad_disconnect()\n");
  {
    struct usb_chatpad* chatpad = usb_get_intfdata(intf);

    // stop the keepalive messages
    cancel_delayed_work(&chatpad->worker);
    flush_delayed_work(&chatpad->worker);

    // kill the urb
    usb_kill_urb(chatpad->chatpad_urb);
    usb_free_urb(chatpad->chatpad_urb);

    // kill the urb
    usb_kill_urb(chatpad->control_urb);
    usb_free_urb(chatpad->control_urb);

    // deallocate memory
    usb_buffer_free(chatpad->udev, 32, chatpad->chatpad_idata, chatpad->chatpad_idata_dma);
    kfree(chatpad);
  }
}

static struct usb_driver chatpad_driver = {
  .name		= "chatpad",
  .probe        = chatpad_probe,
  .disconnect	= chatpad_disconnect,
  .id_table	= chatpad_table,
};

static int __init usb_chatpad_init(void)
{ 
  // FIXME: called once for each interface?!
  int result = usb_register(&chatpad_driver);

  printk(KERN_INFO "usb_chatpad_init() called\n");

  if (result == 0)
    printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_DESC "\n");

  return result;
}

static void __exit usb_chatpad_exit(void)
{
  printk(KERN_INFO "usb_chatpad_exit() called\n");
  usb_deregister(&chatpad_driver);
}

module_init(usb_chatpad_init);
module_exit(usb_chatpad_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

/* EOF */
