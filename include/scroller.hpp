#pragma once

#include "backlog.hpp"
#include "detail/util.hpp"
#include "detail/filter.hpp"

namespace sts
{
  class scroller
  {
    public:
      scroller() = delete;
      scroller(backlog &bl, std::size_t const step)
        : backlog_{ bl }
        , step_{ step }
      { }

      template <typename It>
      void write(It const &begin, It end)
      {
        auto const size(std::distance(begin, end));
        if(::write(STDOUT_FILENO, &*begin, size) != size)
        { throw std::runtime_error{ "partial/failed write (stdout)" }; }

        end = detail::filter(*this, begin, end);
        backlog_.write(begin, end);

        auto &impl(backlog_.get_impl());
        auto const line_markers_size(impl.line_markers_.size());
        auto const rows(impl.tty_.get().size.ws_row);
        if(following_ && line_markers_size > rows)
        { scroll_pos_ = line_markers_size - rows; }
      }

      void up()
      {
        if(!scroll_pos_)
        { return; }

        following_ = false;
        scroll_pos_ -= std::min(step_, scroll_pos_);
        redraw();
      }

      void down()
      {
        auto &impl(backlog_.get_impl());
        if(scroll_pos_ + impl.tty_.get().size.ws_row >= impl.line_markers_.size())
        {
          following_ = true;
          return;
        }
        ssize_t const diff
        {
          static_cast<ssize_t>(scroll_pos_ + impl.tty_.get().size.ws_row) -
          static_cast<ssize_t>(impl.line_markers_.size())
        };
        scroll_pos_ += std::min(step_, static_cast<std::size_t>(std::abs(diff)));
        redraw();
      }

      void follow()
      {
        auto &impl(backlog_.get_impl());
        if(following_)
        { return; }

        scroll_pos_ = impl.line_markers_.size() - impl.tty_.get().size.ws_row;
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

        auto &impl(backlog_.get_impl());
        std::size_t const rows{ impl.tty_.get().size.ws_row };
        for(std::size_t i{ scroll_pos_ };
            i < scroll_pos_ + std::min(impl.line_markers_.size(), rows);
            ++i)
        {
          ssize_t size((impl.line_markers_[i].second -
                        impl.line_markers_[i].first) + 1);
          if(i == scroll_pos_ + std::min(impl.line_markers_.size() - 1,
                                         rows - 1))
          { --size; }
          if(::write(STDOUT_FILENO, &impl.buf_[impl.line_markers_[i].first],
                     size) != size)
          { throw std::runtime_error{ "partial/failed write (stdout)" }; }
        }
      }

      template <typename T, typename It>
      friend It detail::filter(T&, It, It);

      backlog &backlog_;
      std::size_t scroll_pos_{};
      std::size_t const step_{};
      bool following_{ true };
  };
}
