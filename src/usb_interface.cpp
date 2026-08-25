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

#include <assert.h>
#include <string.h>
#include <stdexcept>

#include <fmt/format.h>

#include "usb_helper.hpp"

namespace unsebu {

struct USBReadData
{
  USBInterface* iface;
  std::function<bool (uint8_t*, int)> callback;
};

struct USBWriteData
{
  USBInterface* iface;
  std::function<bool (libusb_transfer*)> callback;
};

USBInterface::USBInterface(libusb_device_handle* handle, int interface, bool try_detach) :
  m_handle(handle),
  m_interface(interface),
  m_endpoints()
{
  int err = libusb_claim_interface(handle, m_interface);
  if (err == LIBUSB_SUCCESS)
  {
    // success
  }
  else if (err == LIBUSB_ERROR_BUSY && try_detach)
  {
    // try to detach and then try to reopen
    err = libusb_detach_kernel_driver(handle, interface);
    if (err != LIBUSB_SUCCESS)
    {
      throw std::runtime_error(fmt::format("error detaching kernel driver: {}: {}", interface, libusb_strerror(err)));
    }
    else
    {
      // kernel driver detached, try to claim it again
      err = libusb_claim_interface(handle, interface);
      if (err != LIBUSB_SUCCESS)
      {
        throw std::runtime_error(fmt::format("error claiming interface: {}: {}", interface, libusb_strerror(err)));
      }
    }
  }
  else
  {
    throw std::runtime_error(fmt::format("error claiming interface: {}: {}", interface, libusb_strerror(err)));
  }
}

USBInterface::~USBInterface()
{
  // cancel all transfer that might still be running
  for(auto it = m_endpoints.begin(); it != m_endpoints.end(); ++it)
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
                          std::function<bool (uint8_t*, int)> const& callback)
{
  assert(m_endpoints.find(endpoint) == m_endpoints.end());

  libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;

  uint8_t* data = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * len));

  libusb_fill_interrupt_transfer(transfer, m_handle,
                                 static_cast<unsigned char>(endpoint | LIBUSB_ENDPOINT_IN),
                                 data, len,
                                 [](libusb_transfer* transfer_) {
                                   static_cast<USBReadData*>(transfer_->user_data)->iface->on_read_data(
                                     static_cast<USBReadData*>(transfer_->user_data), transfer_);
                                 },
                                 new USBReadData{this, callback},
                                 0); // timeout

  int err = libusb_submit_transfer(transfer);
  if (err != LIBUSB_SUCCESS)
  {
    libusb_free_transfer(transfer);

    throw std::runtime_error(fmt::format("libusb_submit_transfer(): ", libusb_strerror(err)));
  }
  else
  {
    // transfer is send on its way, so store it
    m_endpoints[endpoint | LIBUSB_ENDPOINT_IN] = transfer;
  }
}

void
USBInterface::submit_write(int endpoint, uint8_t* data_in, int len,
                           std::function<bool (libusb_transfer*)> const& callback)
{
  libusb_transfer* transfer = libusb_alloc_transfer(0);
  transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;

  // copy data into a newly allocated buffer
  uint8_t* data = static_cast<uint8_t*>(malloc(sizeof(uint8_t) * len));
  memcpy(data, data_in, len);

  libusb_fill_interrupt_transfer(transfer, m_handle,
                                 static_cast<unsigned char>(endpoint | LIBUSB_ENDPOINT_OUT),
                                 data, len,
                                 [](libusb_transfer* xfer) {
                                   static_cast<USBWriteData*>(xfer->user_data)->iface->on_write_data(
                                     static_cast<USBWriteData*>(xfer->user_data), xfer);
                                 },
                                 new USBWriteData{this, callback},
                                 0); // timeout

  int err = libusb_submit_transfer(transfer);
  if (err != LIBUSB_SUCCESS)
  {
    libusb_free_transfer(transfer);

    throw std::runtime_error(fmt::format("libusb_submit_transfer(): ", libusb_strerror(err)));
  }
  else
  {
    m_endpoints[endpoint | LIBUSB_ENDPOINT_OUT] = transfer;
  }
}

void
USBInterface::cancel_transfer(int endpoint)
{
  auto const it = m_endpoints.find(endpoint);
  if (it == m_endpoints.end())
  {
    throw std::runtime_error(fmt::format("endpoint {} not found", (endpoint & LIBUSB_ENDPOINT_ADDRESS_MASK)));
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
USBInterface::on_read_data(USBReadData* userdata, libusb_transfer* transfer)
{
  if (userdata->callback(transfer->buffer, transfer->actual_length))
  {
    // callback returned true, thus resend the transfer
    int err;
    err = libusb_submit_transfer(transfer);
    if (err != LIBUSB_SUCCESS)
    {
      libusb_free_transfer(transfer);

      throw std::runtime_error(fmt::format("libusb_submit_transfer(): {}", libusb_strerror(err)));
    }
  }
  else
  {
    // callback returned false, thus doing cleanup
    delete userdata;
    libusb_free_transfer(transfer);
    m_endpoints.erase(transfer->endpoint);
  }
}

void
USBInterface::on_write_data(USBWriteData* userdata, libusb_transfer* transfer)
{
  if (userdata->callback(transfer))
  {
    // callback returned true, thus resend the transfer (user is free
    // to fill it with new data)
    int err = libusb_submit_transfer(transfer);
    if (err != LIBUSB_SUCCESS)
    {
      libusb_free_transfer(transfer);

      throw std::runtime_error(fmt::format("libusb_submit_transfer(): ", libusb_strerror(err)));
    }
  }
  else
  {
    // callback returned false, thus doing cleanup
    delete userdata;
    libusb_free_transfer(transfer);
    m_endpoints.erase(transfer->endpoint);
  }
}

} // namespace unsebu

/* EOF */
