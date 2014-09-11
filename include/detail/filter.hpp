#pragma once

#include "detail/util.hpp"

#include <regex>
#include <algorithm>

namespace sts
{
  namespace detail
  {
    template <typename T>
    struct predicate
    {
      using func_t = std::function<void (T&)>;

      predicate() = delete;
      predicate(std::string &&r)
        : regex{ std::move(r) }
      { }
      predicate(std::string &&r, func_t &&f)
        : regex{ std::move(r) }
        , func{ std::move(f) }
      { }

      std::regex regex;
      func_t func;
    };

    template <typename T, typename It>
    It filter(T &self, It const &begin, It end)
    {
      std::ofstream ofs{ ".filter", std::ios_base::app };
      static auto const predicates(detail::make_array
      (
        /* smcup */
        predicate<T>
        {
          "\x1B\\[\\?47h",
          [](T &self)
          {
            self.backlog_.impls_.emplace_back(self.backlog_.tty_,
                                              self.backlog_.limit_);
          }
        },
        /* rmcup */
        predicate<T>
        {
          "\x1B\\[\\?47l",
          [](T &self)
          {
            if(self.backlog_.impls_.size() > 1)
            { self.backlog_.impls_.erase(self.backlog_.impls_.end() - 1); }
          }
        },
        /* clear */
        predicate<T>{ "\x1B\\[2J" },
        /* move cursor/scroll */
        predicate<T>{ "\x1B\\[([[:digit:]]){0,3}(A|B|C|D|E|F|G|J|K|S|T)", },
        /* move cursor */
        predicate<T>{ "\x1B\\[([[:digit:]]){0,3};([[:digit:]]){0,3}(H|f)", },
        /* report cursor */
        predicate<T>{ "\x1B\\[6n", },
        /* save/restore cursor */
        predicate<T>{ "\x1B\\[(s|u)" },
        /* show/hide cursor */
        predicate<T>{ "\x1B\\[\\?25(h|l)" }
      ));

      std::smatch match;
      for(auto const &pred : predicates)
      {
        /* TODO: string */
        while(std::regex_search(std::string{ begin, end }, match, pred.regex))
        {
          if(pred.func)
          { pred.func(self); }

          std::copy(begin, end, std::ostream_iterator<int>(ofs, " "));
          ofs << std::endl;
          ofs << "filtering (" << match.position() << ")"
              << "[" << match.length() << "]: ";
          auto const str(match.str());
          std::copy(std::begin(str), std::end(str), std::ostream_iterator<int>(ofs, " "));
          ofs << std::endl;

          auto const it(begin + match.position());
          auto const sub_end(it + match.length());
          auto rit(begin);
          end = std::remove_if(begin, end, [&](char const)
          {
            auto const cur(rit++);
            return (cur >= it && cur < sub_end);
          });

          std::copy(begin, end, std::ostream_iterator<int>(ofs, " "));
          ofs << std::endl;
        }
      }

      return end;
    }
  }
}
