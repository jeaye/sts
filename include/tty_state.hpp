#pragma once

#include <stdexcept>

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

namespace sts
{
  struct tty_state
  {
    struct error : std::runtime_error
    { using std::runtime_error::runtime_error; };

    tty_state()
    {
      if(tcgetattr(STDIN_FILENO, &term) == -1)
      { throw error{ "failed to initialize termios" }; }
      if(ioctl(STDIN_FILENO, TIOCGWINSZ, &size) < 0)
      { throw error{ "failed to initialize term size" }; }
    }
    ~tty_state()
    {
      /* XXX: May throw in dtor; should terminate program. */
      if(tcsetattr(STDIN_FILENO, TCSANOW, &term) == -1)
      { throw error{ "failed to reset terminal" }; }
    }

    termios term;
    winsize size;
  };
}
