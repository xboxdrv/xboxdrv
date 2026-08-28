/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011-2026 Ingo Ruhnke <grumbel@gmail.com>
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
#include <sys/time.h>
#include <string.h>
#include <stdexcept>

#include <format>

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
      throw std::runtime_error(std::format("error detaching kernel driver: {}: {}", interface, libusb_strerror(err)));
    }
    else
    {
      // kernel driver detached, try to claim it again
      err = libusb_claim_interface(handle, interface);
      if (err != LIBUSB_SUCCESS)
      {
        throw std::runtime_error(std::format("error claiming interface: {}: {}", interface, libusb_strerror(err)));
      }
    }
  }
  else
  {
    throw std::runtime_error(std::format("error claiming interface: {}: {}", interface, libusb_strerror(err)));
  }
}

USBInterface::~USBInterface()
{
  // Cancel outstanding transfers and drain completion callbacks. Do not
  // free_transfer immediately after cancel — libusb still owns the transfer
  // until the callback runs.
  for (auto& [endpoint, transfer] : m_endpoints)
  {
    if (transfer)
    {
      int ret = libusb_cancel_transfer(transfer);
      if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_NOT_FOUND)
      {
        // best-effort
      }
    }
  }

  for (int i = 0; !m_endpoints.empty() && i < 100; ++i)
  {
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    int ret = libusb_handle_events_timeout_completed(nullptr, &tv, nullptr);
    if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_INTERRUPTED)
    {
      break;
    }
  }

  // Abandon any transfers that never completed (device gone).
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

    throw std::runtime_error(std::format("libusb_submit_transfer(): {}", libusb_strerror(err)));
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

    throw std::runtime_error(std::format("libusb_submit_transfer(): {}", libusb_strerror(err)));
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
    throw std::runtime_error(std::format("endpoint {} not found", (endpoint & LIBUSB_ENDPOINT_ADDRESS_MASK)));
  }

  libusb_transfer* transfer = it->second;
  int ret = libusb_cancel_transfer(transfer);
  if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_NOT_FOUND)
  {
    throw std::runtime_error(std::format("libusb_cancel_transfer(): {}", libusb_strerror(ret)));
  }

  // Completion callback removes the entry from m_endpoints and frees the
  // transfer. Drain briefly so callers can rely on the cancel finishing.
  for (int i = 0; m_endpoints.count(endpoint) && i < 100; ++i)
  {
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    ret = libusb_handle_events_timeout_completed(nullptr, &tv, nullptr);
    if (ret != LIBUSB_SUCCESS && ret != LIBUSB_ERROR_INTERRUPTED)
    {
      break;
    }
  }

  if (m_endpoints.count(endpoint))
  {
    m_endpoints.erase(endpoint);
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
  if (transfer->status == LIBUSB_TRANSFER_CANCELLED ||
      transfer->status == LIBUSB_TRANSFER_NO_DEVICE)
  {
    delete userdata;
    m_endpoints.erase(transfer->endpoint);
    libusb_free_transfer(transfer);
    return;
  }

  if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
  {
    delete userdata;
    m_endpoints.erase(transfer->endpoint);
    libusb_free_transfer(transfer);
    return;
  }

  if (userdata->callback(transfer->buffer, transfer->actual_length))
  {
    int err = libusb_submit_transfer(transfer);
    if (err != LIBUSB_SUCCESS)
    {
      delete userdata;
      m_endpoints.erase(transfer->endpoint);
      libusb_free_transfer(transfer);
    }
  }
  else
  {
    delete userdata;
    m_endpoints.erase(transfer->endpoint);
    libusb_free_transfer(transfer);
  }
}

void
USBInterface::on_write_data(USBWriteData* userdata, libusb_transfer* transfer)
{
  if (transfer->status == LIBUSB_TRANSFER_CANCELLED ||
      transfer->status == LIBUSB_TRANSFER_NO_DEVICE)
  {
    delete userdata;
    m_endpoints.erase(transfer->endpoint);
    libusb_free_transfer(transfer);
    return;
  }

  if (transfer->status != LIBUSB_TRANSFER_COMPLETED)
  {
    delete userdata;
    m_endpoints.erase(transfer->endpoint);
    libusb_free_transfer(transfer);
    return;
  }

  if (userdata->callback(transfer))
  {
    int err = libusb_submit_transfer(transfer);
    if (err != LIBUSB_SUCCESS)
    {
      delete userdata;
      m_endpoints.erase(transfer->endpoint);
      libusb_free_transfer(transfer);
    }
  }
  else
  {
    delete userdata;
    m_endpoints.erase(transfer->endpoint);
    libusb_free_transfer(transfer);
  }
}

} // namespace unsebu

/* EOF */
