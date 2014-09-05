#pragma once

#include <vector>
#include <utility>
#include <stdexcept>
#include <fstream> /* TODO */

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "detail/resource.hpp"
#include "tty.hpp"

namespace sts
{
  class backlog
  {
    public:
      using marker_iterator = std::size_t;
      using marker_t = std::pair<marker_iterator, marker_iterator>;
      using limit_t = std::size_t;

      backlog() = delete;
      backlog(tty const &tty, std::string const &file, limit_t const limit)
        : tty_{ tty }
        , limit_{ limit }
      { }

      template <typename It>
      void write(It const &begin, It const &end)
      {
        mark_lines(begin, end);

        auto const size(std::distance(begin, end));
        if(::write(STDOUT_FILENO, &*begin, size) != size)
        { throw std::runtime_error{ "partial/failed write (stdout)" }; }

        std::copy(begin, end, std::back_inserter(buf_));
        trim();
      }

    private:
      template <typename It>
      void mark_lines(It const &begin, It const &end)
      {
        if(!std::distance(begin, end))
        { return; }

        if(line_markers_.empty())
        { line_markers_.emplace_back(0, 0); }
        else if(last_char_ == '\n')
        {
          line_markers_.emplace_back((line_markers_.end() - 1)->second + 1,
                                     (line_markers_.end() - 1)->second + 1);
        }

        marker_t *marker{ &*(line_markers_.end() - 1) };
        for(auto it(begin); it != end; ++it)
        {
          auto &i(marker->second);
          if(*it == '\n')
          {
            if(it + 1 != end)
            {
              line_markers_.emplace_back(i + 1, i + 1);
              marker = &*(line_markers_.end() - 1);
            }
          }
          else
          { ++i; }
          last_char_ = *it;
        }
      }

      void trim()
      {
        if(limit_ == 0 || line_markers_.size() <= tty_.size.ws_row + limit_)
        { return; }

        auto const excess(line_markers_.size() - (tty_.size.ws_row + limit_));
        auto const offset(line_markers_[excess].first);
        line_markers_.erase(std::begin(line_markers_),
                            std::begin(line_markers_) + excess);
        buf_.erase(0, offset);
        for(auto &marker : line_markers_)
        {
          marker.first -= offset;
          marker.second -= offset;
        }
      }

      friend class scroller;

      tty const &tty_;
      std::string buf_;
      std::vector<marker_t> line_markers_;
      char last_char_{};
      limit_t const limit_{};
  };
}
