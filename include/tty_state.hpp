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

    void enter_raw_mode()
    {
      term.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);

      /* Noncanonical mode, disable signals, extended
         input processing, and echoing */
      term.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR |
          INPCK | ISTRIP | IXON | PARMRK);

      /* Disable special handling of CR, NL, and BREAK.
         No 8th-bit stripping or parity error handling.
         Disable START/STOP output flow control. */
      term.c_oflag &= ~OPOST; /* Disable all output processing */

      term.c_cc[VMIN] = 1; /* Character-at-a-time input */
      term.c_cc[VTIME] = 0; /* with blocking */

      if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &term) == -1)
      { throw error{ "failed to enter raw mode" }; }
    }

    termios term;
    winsize size;
  };
}
