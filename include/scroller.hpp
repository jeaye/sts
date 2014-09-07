#pragma once

#include "backlog.hpp"
#include "detail/util.hpp"

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
      void write(It const &begin, It end)
      {
        auto const size(std::distance(begin, end));
        if(::write(STDOUT_FILENO, &*begin, size) != size)
        { throw std::runtime_error{ "partial/failed write (stdout)" }; }

        end = filter(begin, end);
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
        --scroll_pos_;
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
        ++scroll_pos_;
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
      template <typename It>
      It filter(It const &begin, It end)
      {
        std::ofstream ofs{ ".filter", std::ios_base::app };
        auto distance(std::distance(begin, end));
        size_t d{};
        auto const predicates(detail::make_array<std::function<size_t (It)>>(
        [&](It const it)
        {
          d = detail::seq_eq<6>(distance, it, { 27, '[', '?', '4', '7', 'h' });
          if(d)
          { backlog_.impls_.emplace_back(backlog_.tty_, backlog_.limit_); }
          return d;
        },
        [&](It const it)
        {              
          d = detail::seq_eq<6>(distance, it, { 27, '[', '?', '4', '7', 'l' });
          if(d && backlog_.impls_.size() > 1)
          { backlog_.impls_.erase(backlog_.impls_.end() - 1); }
          return d;
        },
        [&](It const it)
        { return d = detail::seq_eq<4>(distance, it, { 27, '[', '2', 'J' }); }
        ));

        auto const pred_end(std::end(predicates));
        for(auto it(begin); it != end; ++it, --distance)
        {
          while(std::find_if(std::begin(predicates), pred_end,
            [&](std::function<size_t (It)> const &f)
            { return f(it); }) != pred_end)
          {
            std::copy(begin, end, std::ostream_iterator<int>(ofs, " "));
            ofs << std::endl;
            auto const sub_end(it + d);
            auto rit(begin);
            end = std::remove_if(begin, end, [&](char const)
            {
              auto const cur(rit++);
              return (cur >= it && cur < sub_end);
            });
            std::copy(begin, end, std::ostream_iterator<int>(ofs, " "));
            ofs << std::endl;

            if(it == end)
            { break; }
          }

          if(it == end)
          { break; }
        }

        return end;
      }

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

      backlog &backlog_;
      std::size_t scroll_pos_{};
      bool following_{ true };
  };
}
