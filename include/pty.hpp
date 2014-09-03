#pragma once

#include <string>
#include <stdexcept>

#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "tty.hpp"
#include "resource.hpp"

namespace sts
{
  class pty
  {
    public:
      struct error : std::runtime_error
      { using std::runtime_error::runtime_error; };

      pty() = delete;
      pty(tty const &state)
        : tty_{ state }, master_{ posix_openpt(O_RDWR | O_NOCTTY), &close }
      {
        if(master_.get() == -1)
        { throw error{ "failed to open pty" }; }
        auto const master(master_.get());

        if(grantpt(master) == -1)
        { throw error{ "failed to grant pty access" }; }

        if(unlockpt(master) == -1)
        { throw error{ "failed to unlock pty" }; }

        char const * const name{ ptsname(master) };
        if(!name)
        { throw error{ "failed to acquire pty name" }; }
        name_ = name;
      }

      void operator ()(std::function<void ()> const &func)
      {
        child_ = fork();

        if(child_ == -1)
        { throw error{ "failed to fork" }; }
        if(child_ != 0) /* Return back to parent. */
        { return; }

        /* Child falls through to here */
        if(setsid() == -1)
        { throw error{ "failed to setsid" }; }
        master_ = 0;

        /* Becomes controlling tty */
        slave_ = open(name_.c_str(), O_RDWR);
        if(slave_.get() == -1)
        { throw error{ "failed to open slave" }; }
        auto const slave(slave_.get());

        if(tcsetattr(slave, TCSANOW, &tty_.term) == -1)
        { throw error{ "failed to set slave terminal attributes" }; }

        if(ioctl(slave, TIOCSWINSZ, &tty_.size) == -1)
        { throw error{ "failed to set slave terminal size" }; }

        /* Duplicate pty slave to be child's stdin, stdout, and stderr. */
        if(dup2(slave, STDIN_FILENO) != STDIN_FILENO)
        { throw error{ "failed to duplicate stdin" }; }
        if(dup2(slave, STDOUT_FILENO) != STDOUT_FILENO)
        { throw error{ "failed to duplicate stdout" }; }
        if(dup2(slave, STDERR_FILENO) != STDERR_FILENO)
        { throw error{ "failed to duplicate stderr" }; }

        if(slave > STDERR_FILENO)
        { slave_ = 0; }

        func();
      }

      int get_master() const
      { return master_.get(); }

    private:
      tty const &tty_;
      resource<int> master_;
      resource<int> slave_{ &close };
      std::string name_;
      pid_t child_{};
  };
}
