#pragma once

#include <string>
#include <vector>
#include <functional>

#include "tty.hpp"

namespace sts
{
  namespace detail
  {
    struct backlog_impl
    {
      using marker_iterator = std::size_t;
      using marker_t = std::pair<marker_iterator, marker_iterator>;
      using limit_t = std::size_t;

      backlog_impl() = delete;
      backlog_impl(tty const &tty, limit_t const limit)
        : tty_{ tty }
        , limit_{ limit }
      { }

      template <typename It>
      void write(It const &begin, It const &end)
      { std::copy(begin, end, std::back_inserter(buf_)); }

      template <typename It>
      void mark_lines(It const &begin, It const &end)
      {
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
        if(limit_ == 0 || line_markers_.size() <= tty_.get().size.ws_row + limit_)
        { return; }

        auto const excess(line_markers_.size() - (tty_.get().size.ws_row + limit_));
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

      std::reference_wrapper<tty const> tty_;
      std::string buf_;
      std::vector<marker_t> line_markers_;
      char last_char_{};
      limit_t limit_{};
    };
  }
}
