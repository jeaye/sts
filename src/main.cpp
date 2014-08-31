#include <iostream>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>
#include <iterator>

#include "tty_state.hpp"
#include "pseudo_term.hpp"
#include "backlog.hpp"

int main(int const, char ** const)
try
{
  sts::tty_state tty;
  sts::pseudo_term pt{ tty };
  pt([]
  {
    char const *shell{ getenv("SHELL") };
    if(!shell || *shell == '\0')
    { shell = "/bin/sh"; }
    //std::string const cmd{ std::string{"-c \"printf '\\x1B[2J\\x1B[H'; "} + shell + "\"" };

    execlp(shell, shell, nullptr);
    throw std::runtime_error{ "shell failed to run" };
  });

  /* Parent: relay data between terminal and pty master */
  sts::backlog backlog{ tty, ".out_log" };

  /* Place terminal in raw mode so that we can pass all terminal
     input to the pseudoterminal master untouched */
  tty.enter_raw_mode();

  /* Clear the screen first. */
  std::string const clear{ "\x1B[H\x1B[2J" };
  if(write(STDOUT_FILENO, clear.c_str(), clear.size()) != clear.size())
  { throw std::runtime_error{ "unable to clear screen" }; }

  std::array<char, 256> buf{};
  ssize_t num_read{};
  fd_set in_fds{};
  int const master_fd{ pt.get_master() };
  std::ofstream ofs{ ".in_log" };

  while(true)
  {
    FD_ZERO(&in_fds);
    FD_SET(STDIN_FILENO, &in_fds);
    FD_SET(master_fd, &in_fds);

    if(select(master_fd + 1, &in_fds, nullptr, nullptr, nullptr) == -1)
    { throw std::runtime_error{ "failed to select" }; }

    if(FD_ISSET(STDIN_FILENO, &in_fds)) /* stdin --> pty */
    {
      num_read = read(STDIN_FILENO, buf.data(), buf.size());
      if(num_read <= 0)
      { break; }
      bool done{};
      for(size_t i{}; i < num_read; ++i)
      {
        if(buf[i] == 25)
        {
          ofs << "(" << num_read << ") " << "scroll up" << " ";
          done = true;
        }
        else if(buf[i] == 5)
        {
          ofs << "(" << num_read << ") " << "scroll down" << " ";
          done = true;
        }
        else if(buf[i] == 11)
        {
          ofs << "(" << num_read << ") " << "clearing" << " ";
          std::string const clear{ "printf '\\x1B[H\\x1B[2J'\n" };
          if(write(master_fd, clear.c_str(), clear.size()) != clear.size())
          { throw std::runtime_error{ "unable to clear screen" }; }
          done = true;
        }
        else
        { ofs << "(" << num_read << ") " << static_cast<int>(buf[i]) << " "; }
      }
      ofs << std::endl;
      if(done)
      { continue; }

      if(write(master_fd, buf.data(), num_read) != num_read)
      { throw std::runtime_error{ "partial/failed write (master)" }; }
    }

    if(FD_ISSET(master_fd, &in_fds)) /* pty --> stdout + log */
    {
      num_read = read(master_fd, buf.data(), buf.size());
      if(num_read <= 0)
      { break; }
      backlog.write(std::begin(buf), std::begin(buf) + num_read);
    }
  }
}
catch(std::exception const &e)
{ std::cout << "exception: " << e.what() << std::endl; }
catch(...)
{ std::cout << "unknown error occurred" << std::endl; }
