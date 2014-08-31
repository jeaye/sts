#pragma once

#include "resource.hpp"
#include "tty_state.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

namespace sts
{
  class backlog
  {
    public:
      backlog() = delete;
      backlog(tty_state const &tty, std::string const &file)
        : tty_{ tty }
        , file_{ open(file.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH),
                 &close }
      {
        if(file_.get() == -1)
        { throw std::runtime_error{ "failed to open log file" }; }
      }

      template <typename It>
      void write(It const &begin, It const &end)
      {
        auto const size(std::distance(begin, end));
        if(::write(file_.get(), &*begin, size) != size)
        { throw std::runtime_error{ "partial/failed write (log)" }; }
      }

    private:
      tty_state const &tty_;
      resource<int> file_;
  };
}
