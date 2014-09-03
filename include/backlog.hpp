#pragma once

#include <vector>
#include <utility>
#include <stdexcept>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "resource.hpp"
#include "tty_state.hpp"

namespace sts
{
  class backlog
  {
    public:
      using marker_iterator = size_t;
      using marker_t = std::pair<marker_iterator, marker_iterator>;

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
        mark_lines(begin, end);

        auto const size(std::distance(begin, end));
        if(::write(file_.get(), &*begin, size) != size)
        { throw std::runtime_error{ "partial/failed write (log)" }; }
        std::copy(begin, end, std::back_inserter(buf_));

        std::string const clear{ "\x1B[H\x1B[2J" };
        ssize_t const clear_size(clear.size());
        if(::write(STDOUT_FILENO, clear.c_str(), clear.size()) != clear_size)
        { throw std::runtime_error{ "unable to clear screen" }; }

        for(size_t i{}; i < line_markers_.size(); ++i)
        {
          ssize_t const size((line_markers_[i].second - line_markers_[i].first) + 1);
          if(::write(STDOUT_FILENO, &buf_[line_markers_[i].first], size) != size)
          { throw std::runtime_error{ "partial/failed write (stdout)" }; }
        }
      }

      void scroll_up()
      { }

      void scroll_down()
      { }

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

      tty_state const &tty_;
      resource<int> file_;
      std::string buf_;
      std::vector<marker_t> line_markers_;
      char last_char_{};
  };
}
