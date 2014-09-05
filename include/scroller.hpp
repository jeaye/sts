#pragma once

#include "backlog.hpp"

namespace sts
{
  class scroller
  {
    public:
      scroller() = delete;
      scroller(backlog &bl)
        : backlog_{ bl }
      { }

      template <typename It>
      void write(It const &begin, It const &end)
      {
        backlog_.write(begin, end);

        auto const line_markers_size(backlog_.line_markers_.size());
        auto const rows(backlog_.tty_.size.ws_row);
        if(following_ && line_markers_size > rows)
        { scroll_pos_ = line_markers_size - rows; }
      }

      void up()
      {
        if(!scroll_pos_)
        { return; }
        following_ = false;
        --scroll_pos_;
        redraw();
      }

      void down()
      {
        if(scroll_pos_ + backlog_.tty_.size.ws_row >= backlog_.line_markers_.size())
        {
          following_ = true;
          return;
        }
        ++scroll_pos_;
        redraw();
      }

      void follow()
      {
        if(following_)
        { return; }

        scroll_pos_ = backlog_.line_markers_.size() - backlog_.tty_.size.ws_row;
        following_ = true;
        redraw();
      }

      void clear()
      {
        static std::string const clear{ "\x1B[H\x1B[2J" };
        static ssize_t const clear_size(clear.size());
        if(::write(STDOUT_FILENO, clear.c_str(), clear.size()) != clear_size)
        { throw std::runtime_error{ "unable to clear screen" }; }
      }

    private:
      void redraw()
      {
        clear();
        std::size_t const rows{ backlog_.tty_.size.ws_row };
        for(std::size_t i{ scroll_pos_ };
            i < scroll_pos_ + std::min(backlog_.line_markers_.size(), rows);
            ++i)
        {
          ssize_t size((backlog_.line_markers_[i].second -
                        backlog_.line_markers_[i].first) + 1);
          if(i == scroll_pos_ + std::min(backlog_.line_markers_.size() - 1,
                                         rows - 1))
          { --size; }
          if(::write(STDOUT_FILENO, &backlog_.buf_[backlog_.line_markers_[i].first],
                     size) != size)
          { throw std::runtime_error{ "partial/failed write (stdout)" }; }
        }
      }

      backlog &backlog_;
      std::size_t scroll_pos_{};
      bool following_{ true };
  };
}
