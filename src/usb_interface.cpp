/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmail.com>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "usb_interface.hpp"

#include "raise_exception.hpp"
#include "usb_helper.hpp"

struct USBReadCallback
{
  USBInterface* iface;
  boost::function<bool (uint8_t*, int)> callback;

  USBReadCallback(USBInterface* iface_,
                  boost::function<bool (uint8_t*, int)> callback_) :
    iface(iface_),
    callback(callback_)
  {}

private:
  USBReadCallback(const USBReadCallback&);
  USBReadCallback& operator=(const USBReadCallback&);
};

struct USBWriteCallback
{
  USBInterface* iface;
  boost::function<bool (libusb_transfer*)> callback;

  USBWriteCallback(USBInterface* iface_,
                   boost::function<bool (libusb_transfer*)> callback_) :
    iface(iface_),
    callback(callback_)
  {}

private:
  USBWriteCallback(const USBWriteCallback&);
  USBWriteCallback& operator=(const USBWriteCallback&);
};

USBInterface::USBInterface(libusb_device_handle* handle, int interface, bool try_detach) :
  m_handle(handle),
  m_interface(interface),
  m_endpoints()
{
  int ret = libusb_claim_interface(handle, m_interface);
  if (ret == LIBUSB_SUCCESS)
  {
    // success
  }
  else if (ret == LIBUSB_ERROR_BUSY && try_detach)
  {
    // try to detach and then try to reopen
    ret = libusb_detach_kernel_driver(handle, interface);
    if (ret != LIBUSB_SUCCESS)
    {
      raise_exception(std::runtime_error, "error detaching kernel driver: "
                      << interface << ": " << usb_strerror(ret));
    }
    else
    {
      // kernel driver detached, try to claim it again
      ret = libusb_claim_interface(handle, interface);
      if (ret != LIBUSB_SUCCESS)
      {
        raise_exception(std::runtime_error, "error claiming interface: " << interface << ": " << usb_strerror(ret));
      }
    }
  }
  else
  {
    raise_exception(std::runtime_error, "error claiming interface: " << interface << ": " << usb_strerror(ret));
  }
}

USBInterface::~USBInterface()
{
  // cancel all transfer that might still be running
  for(Endpoints::iterator it = m_endpoints.begin(); it != m_endpoints.end(); ++it)
  {
    if (it->second)
    {
      libusb_cancel_transfer(it->second);
      libusb_free_transfer(it->second);
    }
  }
  m_endpoints.clear();

  libusb_release_interface(m_handle, m_interface);
}

void
USBInterface::submit_read(int endpoint, int len,
                          const boost::function<bool (uint8_t*, int)>& callback)
{
  assert(m_endpoints.find(endpoint) == m_endpoints.end());
  libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;

  uint8_t* data = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * len));

  libusb_fill_interrupt_transfer(transfer, m_handle,
                                 static_cast<unsigned char>(endpoint | LIBUSB_ENDPOINT_IN),
                                 data, len,
                                 &USBInterface::on_read_data_wrap,
                                 new USBReadCallback(this, callback),
                                 0); // timeout
  int ret;
  ret = libusb_submit_transfer(transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    libusb_free_transfer(transfer);
    raise_exception(std::runtime_error, "libusb_submit_transfer(): " << usb_strerror(ret));
  }
  else
  {
    // transfer is send on its way, so store it
    m_endpoints[endpoint | LIBUSB_ENDPOINT_IN] = transfer;
  }
}

void
USBInterface::submit_write(int endpoint, uint8_t* data_in, int len,
                           const boost::function<bool (libusb_transfer*)>& callback)
{
  libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;

  // copy data into a newly allocated buffer
  uint8_t* data = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * len));
  memcpy(data, data_in, len);

  libusb_fill_interrupt_transfer(transfer, m_handle,
                                 static_cast<unsigned char>(endpoint | LIBUSB_ENDPOINT_OUT),
                                 data, len,
                                 &USBInterface::on_write_data_wrap,
                                 new USBWriteCallback(this, callback),
                                 0); // timeout

  int ret;
  ret = libusb_submit_transfer(transfer);
  if (ret != LIBUSB_SUCCESS)
  {
    libusb_free_transfer(transfer);
    raise_exception(std::runtime_error, "libusb_submit_transfer(): " << usb_strerror(ret));
  }
  else
  {
    m_endpoints[endpoint | LIBUSB_ENDPOINT_OUT] = transfer;
  }
}

void
USBInterface::cancel_transfer(int endpoint)
{
  Endpoints::iterator it = m_endpoints.find(endpoint);
  if (it == m_endpoints.end())
  {
    raise_exception(std::runtime_error, "endpoint " << (endpoint & LIBUSB_ENDPOINT_ADDRESS_MASK) << "not found");
  }
  else
  {
    libusb_cancel_transfer(it->second);
    libusb_free_transfer(it->second);
    m_endpoints.erase(it);
  }
}

void
USBInterface::cancel_read(int endpoint)
{
  cancel_transfer(endpoint | LIBUSB_ENDPOINT_IN);
}

void
USBInterface::cancel_write(int endpoint)
{
  cancel_transfer(endpoint | LIBUSB_ENDPOINT_OUT);
}

void
USBInterface::on_read_data(USBReadCallback* callback, libusb_transfer* transfer)
{
  if (callback->callback(transfer->buffer, transfer->actual_length))
  {
    // callback returned true, thus resend the transfer
    int ret;
    ret = libusb_submit_transfer(transfer);
    if (ret != LIBUSB_SUCCESS)
    {
      libusb_free_transfer(transfer);
      raise_exception(std::runtime_error, "libusb_submit_transfer(): " << usb_strerror(ret));
    }
  }
  else
  {
    // callback returned false, thus doing cleanup
    delete callback;
    libusb_free_transfer(transfer);
    m_endpoints.erase(transfer->endpoint);
  }
}

void
USBInterface::on_write_data(USBWriteCallback* callback, libusb_transfer* transfer)
{
  if (callback->callback(transfer))
  {
    // callback returned true, thus resend the transfer (user is free
    // to fill it with new data)
    int ret;
    ret = libusb_submit_transfer(transfer);
    if (ret != LIBUSB_SUCCESS)
    {
      libusb_free_transfer(transfer);
      raise_exception(std::runtime_error, "libusb_submit_transfer(): " << usb_strerror(ret));
    }
  }
  else
  {
    // callback returned false, thus doing cleanup
    delete callback;
    libusb_free_transfer(transfer);
    m_endpoints.erase(transfer->endpoint);
  }
}

void
USBInterface::on_read_data_wrap(libusb_transfer* transfer)
{
  USBReadCallback* cb = static_cast<USBReadCallback*>(transfer->user_data);
  cb->iface->on_read_data(cb, transfer);
}

void
USBInterface::on_write_data_wrap(libusb_transfer* transfer)
{
  USBWriteCallback* cb = static_cast<USBWriteCallback*>(transfer->user_data);
  cb->iface->on_write_data(cb, transfer);
}

/* EOF */
