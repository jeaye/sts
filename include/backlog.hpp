#pragma once

#include <vector>
#include <utility>
#include <stdexcept>
#include <fstream> /* TODO */

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
        parse(begin, end);

        auto const size(std::distance(begin, end));
        if(::write(STDOUT_FILENO, &*begin, size) != size)
        { throw std::runtime_error{ "partial/failed write (stdout)" }; }

        get_impl().write(begin, end);
        get_impl().trim();
      }

    private:
      template <typename It>
      void parse(It const &begin, It const &end)
      {
        auto distance(std::distance(begin, end));

        std::ofstream ofs{ ".parse", std::ios_base::app };

        for(auto it(begin); it != end; ++it, --distance)
        {
          if(distance >= 6 && *it == 27 && *(it + 1) == 91 &&
             *(it + 2) == 63 && *(it + 3) == 52 && *(it + 4) == 55 &&
             *(it + 5) == 104)
          {
            impls_.emplace_back(tty_, limit_);
            get_impl().mark_lines(it + 6, end);
            //std::for_each(it, it + 6, [](char &c)
            //{ c = ' '; });
            ofs << "smcup" << std::endl;
            std::remove_if(begin, end, [rit = begin, it](char const) mutable
            {
              auto const d(rit++ - it);
              return d >= 0 && d < 6;
            });
          }
          else if(distance >= 6 && *it == 27 && *(it + 1) == 91 &&
                  *(it + 2) == 63 && *(it + 3) == 52 && *(it + 4) == 55 &&
                  *(it + 5) == 108)
          {
            if(impls_.size() > 1)
            {
              impls_.erase(impls_.end() - 1);
              get_impl().mark_lines(it + 6, end);
            }
            //std::for_each(it, it + 6, [](char &c)
            //{ c = ' '; });
            ofs << "rmcup" << std::endl;
            std::remove_if(begin, end, [rit = begin, it](char const) mutable
            {
              auto const d(rit++ - it);
              return d >= 0 && d < 6;
            });
          }
        }
      }

      detail::backlog_impl& get_impl()
      { return impls_.at(impls_.size() - 1); }

      friend class scroller;

      tty const &tty_;
      limit_t const limit_{};
      std::vector<detail::backlog_impl> impls_;
  };
}
