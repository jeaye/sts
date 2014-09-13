#pragma once

#include <stdexcept>

#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

namespace sts
{
  struct tty
  {
    struct error : std::runtime_error
    { using std::runtime_error::runtime_error; };

    tty()
    {
      if(tcgetattr(STDIN_FILENO, &term) == -1)
      { throw error{ "failed to initialize termios" }; }
      if(ioctl(STDIN_FILENO, TIOCGWINSZ, &size) < 0)
      { throw error{ "failed to initialize term size" }; }
    }

    termios term;
    winsize size;
  };

  struct raw_mode
  {
    raw_mode() = delete;
    raw_mode(tty const &t)
      : tty_{ t }
    {
      auto term(tty_.term);

      if(tcgetattr(STDIN_FILENO, &term) == -1)
      { throw tty::error{ "failed to initialize termios" }; }
      termios raw_term(term);

      raw_term.c_lflag &= ~(ICANON | ISIG | IEXTEN | ECHO);

      /* Noncanonical mode, disable signals, extended
         input processing, and echoing */
      raw_term.c_iflag &= ~(BRKINT | ICRNL | IGNBRK | IGNCR | INLCR |
          INPCK | ISTRIP | IXON | PARMRK);

      /* Disable special handling of CR, NL, and BREAK.
         No 8th-bit stripping or parity error handling.
         Disable START/STOP output flow control. */
      raw_term.c_oflag &= ~OPOST; /* Disable all output processing */

      raw_term.c_cc[VMIN] = 1; /* Character-at-a-time input */
      raw_term.c_cc[VTIME] = 0; /* with blocking */

      if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_term) == -1)
      { throw tty::error{ "failed to enter raw mode" }; }
    }
    ~raw_mode()
    {
      if(tcsetattr(STDIN_FILENO, TCSANOW, &tty_.term) == -1)
      { throw tty::error{ "failed to reset terminal" }; }
    }

    tty const &tty_;
  };
}
