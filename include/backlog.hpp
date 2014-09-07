#pragma once

#include <vector>
#include <utility>
#include <stdexcept>
#include <fstream> /* TODO */
#include <iterator> /* TODO */

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "detail/resource.hpp"
#include "detail/backlog_impl.hpp"

namespace sts
{
  class backlog
  {
    public:
      using marker_iterator = detail::backlog_impl::marker_iterator;
      using marker_t = detail::backlog_impl::marker_t;
      using limit_t = detail::backlog_impl::limit_t;

      backlog() = delete;
      backlog(tty const &tty, std::string const &file, limit_t const limit)
        : tty_{ tty }
        , limit_{ limit }
        , impls_{ { tty_, limit_ } }
      { }

      template <typename It>
      void write(It const &begin, It const &end)
      {
        if(!std::distance(begin, end))
        { return; }

        get_impl().mark_lines(begin, end);

        auto const size(std::distance(begin, end));
        std::ofstream ofs{ ".out" + std::to_string(impls_.size()), std::ios_base::app };
        ofs.write(&*begin, size);

        get_impl().write(begin, end);
        get_impl().trim();
      }

    private:
      detail::backlog_impl& get_impl()
      { return impls_.at(impls_.size() - 1); }

      friend class scroller;

      tty const &tty_;
      limit_t const limit_{};
      std::vector<detail::backlog_impl> impls_;
  };
}
