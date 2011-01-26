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

#include <iostream>
#include <sys/wait.h>

#include "helper.hpp"
#include "log.hpp"
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

  // connect the processor to the controller to allow rumble
  m_processor->set_ff_callback(boost::bind(&XboxGenericController::set_rumble, m_controller.get(), _1, _2));
}

XboxdrvThread::~XboxdrvThread()
{
  if (m_thread.get())
  {
    log_info("waiting for thread to join: " << m_thread->get_id());
    stop_thread(); 
    log_info("thread joined");
  }
}

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
        log_error("error: " << m_child_exec[0] << ": " << strerror(errno));
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
          log_error("child program has stopped with exit status " << WEXITSTATUS(status));
        }
        else
        {
          log_info("child program exited successful");
        }
        global_exit_xboxdrv = true;
      }
      else if (WIFSIGNALED(status))
      {
        log_error("child program was terminated by " << WTERMSIG(status));
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

        // output current Xbox gamepad state to stdout
        if (!opts.silent)
        {
          std::cout << msg << std::endl;
        }
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

      m_processor->send(msg, msec_delta);

      if (opts.rumble)
      {
        m_controller->set_rumble(get_axis(msg, XBOX_AXIS_LT), 
                                 get_axis(msg, XBOX_AXIS_RT));
      }
        
      watch_chid_process();
    }
  }
  catch(const std::exception& err)
  {
    // catch read errors from USB and other stuff that can go wrong
    log_error(err.what());
  }

  {
    // send a event with everything set to zero, so that the input
    // device doesn't end up with a non-centered state
    XboxGenericMsg msg;
    msg.type = XBOX_MSG_XBOX360;
    memset(&msg.xbox360, 0, sizeof(msg.xbox360));
    m_processor->send(msg, 0);
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
