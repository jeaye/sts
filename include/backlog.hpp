#pragma once

#include <vector>
#include <utility>
#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "detail/resource.hpp"
#include "detail/backlog_impl.hpp"
#include "detail/filter.hpp"

namespace sts
{
  class backlog
  {
    public:
      using marker_iterator = detail::backlog_impl::marker_iterator;
      using marker_t = detail::backlog_impl::marker_t;
      using limit_t = detail::backlog_impl::limit_t;

      backlog() = delete;
      backlog(tty const &tty, limit_t const limit)
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
        get_impl().write(begin, end);
        get_impl().trim();
      }

    private:
      detail::backlog_impl& get_impl()
      { return impls_.at(impls_.size() - 1); }

      friend class scroller;
      template <typename T, typename It>
      friend It detail::filter(T&, It, It);

      tty const &tty_;
      limit_t const limit_{};
      std::vector<detail::backlog_impl> impls_;
  };
}
