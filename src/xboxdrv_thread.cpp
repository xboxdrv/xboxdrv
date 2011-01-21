/*
**  Xbox360 USB Gamepad Userspace Driver
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
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

#include "xboxdrv_thread.hpp"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <assert.h>

#include "controller_config.hpp"
#include "helper.hpp"
#include "log.hpp"
#include "modifier.hpp"
#include "options.hpp"
#include "uinput.hpp"
#include "xbox_generic_controller.hpp"
#include "message_processor.hpp"

extern bool global_exit_xboxdrv;

// FIXME: isolate problametic code to a separate file, instead of pragma
#pragma GCC diagnostic ignored "-Wold-style-cast"

XboxdrvThread::XboxdrvThread(std::auto_ptr<MessageProcessor> processor,
                             std::auto_ptr<XboxGenericController> controller,
                             const Options& opts) :
  m_thread(),
  m_processor(processor),
  m_controller(controller),
  m_loop(true),
  m_oldrealmsg(),
  m_child_exec(opts.exec),
  m_pid(-1),
  m_timeout(opts.timeout)
{
  memset(&m_oldrealmsg, 0, sizeof(m_oldrealmsg));
}

XboxdrvThread::~XboxdrvThread()
{
  if (m_thread.get())
  {
    log_info << "waiting for thread to join: " << m_thread->get_id() << std::endl;
    stop_thread(); 
    log_info << "joined thread: " << m_thread->get_id() << std::endl;
  }
}

// FIXME: duplicate code
namespace {

void set_rumble(XboxGenericController* controller, int gain, uint8_t lhs, uint8_t rhs)
{
  lhs = std::min(lhs * gain / 255, 255);
  rhs = std::min(rhs * gain / 255, 255);
  
  //std::cout << (int)lhs << " " << (int)rhs << std::endl;

  controller->set_rumble(lhs, rhs);
}

} // namespace

void
XboxdrvThread::launch_child_process()
{
  if (!m_child_exec.empty())
  { // launch program if one was given
    m_pid = fork();
    if (m_pid == 0) // child
    {
      char** argv = static_cast<char**>(malloc(sizeof(char*) * m_child_exec.size() + 1));
      for(size_t i = 0; i < m_child_exec.size(); ++i)
      {
        argv[i] = strdup(m_child_exec[i].c_str());
      }
      argv[m_child_exec.size()] = NULL;

      if (execvp(m_child_exec[0].c_str(), argv) == -1)
      {
        std::cout << "error: " << m_child_exec[0] << ": " << strerror(errno) << std::endl;
        // FIXME: must signal the parent process
        _exit(EXIT_FAILURE);
      }
    }
  }
}

void
XboxdrvThread::watch_chid_process()
{
  if (m_pid != -1)
  {
    int status = 0;
    int ret = waitpid(m_pid, &status, WNOHANG);

    // greater 0 means something changed with the process
    if (ret > 0)
    {
      if (WIFEXITED(status))
      {
        if (WEXITSTATUS(status) != 0)
        {
          std::cout << "error: child program has stopped with exit status " << WEXITSTATUS(status) << std::endl;
        }
        else
        {
          std::cout << "child program exited successful" << std::endl;
        }
        global_exit_xboxdrv = true;
      }
      else if (WIFSIGNALED(status))
      {
        std::cout << "error: child program was terminated by " << WTERMSIG(status) << std::endl;
        global_exit_xboxdrv = true;
      }
    }
  }
}

void
XboxdrvThread::controller_loop(const Options& opts)
{
  launch_child_process();

  try 
  {
    uint32_t last_time = get_time();
    while(m_loop && !global_exit_xboxdrv) // FIXME: should not directly depend on global_exit_xboxdrv
    {
      XboxGenericMsg msg;

      if (m_controller->read(msg, opts.verbose, m_timeout))
      {
        m_oldrealmsg = msg;
      }
      else
      {
        // no new data read, so copy the last read data
        msg = m_oldrealmsg;
      }

      // Calc changes in time
      uint32_t this_time = get_time();
      int msec_delta = this_time - last_time;
      last_time = this_time;

      // output current Xbox gamepad state to stdout
      if (!opts.silent)
      { // FIXME: only print stuff on change
        std::cout << msg << std::endl;
      }

      m_processor->send(msg, msec_delta);

#ifdef FIXME                 
      if (opts.rumble)
      { // FIXME: kind of ugly here, should be a filter, but filters
        // can't talk back to the controller
        if (type == GAMEPAD_XBOX)
        {
          set_rumble(m_controller.get(), opts.rumble_gain, msg.xbox.lt, msg.xbox.rt);
        }
        else if (type == GAMEPAD_XBOX360 ||
                 type == GAMEPAD_XBOX360_WIRELESS)
        {
          set_rumble(m_controller.get(), opts.rumble_gain, msg.xbox360.lt, msg.xbox360.rt);
        }
        else if (type == GAMEPAD_FIRESTORM ||
                 type == GAMEPAD_FIRESTORM_VSB)
        {
          set_rumble(m_controller.get(), opts.rumble_gain,
                     std::min(255, abs((msg.xbox360.y1>>8)*2)), 
                     std::min(255, abs((msg.xbox360.y2>>8)*2)));
        }
      }
#endif
        
      watch_chid_process();
    }
  }
  catch(const std::exception& err)
  {
    // catch read errors from USB and other stuff that can go wrong
    log_error << err.what() << std::endl;
  }
}

void
XboxdrvThread::start_thread(const Options& opts)
{
  assert(m_thread.get() == 0);
  m_thread.reset(new boost::thread(boost::bind(&XboxdrvThread::controller_loop, this, 
                                               boost::cref(opts))));
}

void
XboxdrvThread::stop_thread()
{
  assert(m_thread.get());

  m_loop = false;
  m_thread->join();
  m_thread.reset();
}

bool
XboxdrvThread::try_join_thread()
{
  bool got_joined = m_thread->timed_join(boost::posix_time::time_duration(0,0,0,0));
  if (got_joined)
  {
    m_thread.reset();
    return true;
  }
  else
  {
    return false;
  }
}

/* EOF */
