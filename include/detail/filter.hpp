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

      std::regex const regex;
      func_t const func;
    };

    template <typename T, typename It>
    It filter(T &self, It const begin, It end)
    {
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
        predicate<T>{ "\x1B\\[([[:digit:]]){0,3}(A|B|C|D|E|F|G|J|K|S|T)" },
        /* move cursor */
        predicate<T>{ "\x1B\\[([[:digit:]]){0,3};([[:digit:]]){0,3}(H|f)" },
        /* report device */
        predicate<T>{ "\x1B\\[[[:digit:]]n" },
        /* save/restore cursor */
        predicate<T>{ "\x1B\\[(s|u)" },
        /* save/restore cursor attributes */
        predicate<T>{ "\x1B(7|8)" },
        /* show/hide cursor */
        predicate<T>{ "\x1B\\[\\?25(h|l)" },
        /* scroll screen */
        predicate<T>{ "\x1B\\[([[:digit:]]){0,3};([[:digit:]]){0,3}r" },
        /* scroll down/up */
        predicate<T>{ "\x1B(D|M)" },
        /* set tab */
        predicate<T>{ "\x1BH" },
        /* clear tab */
        predicate<T>{ "\x1B\\[3?g" },
        /* [re]set mode */
        predicate<T>{ "\x1B\\[=([[:digit:]]){0,2}(h|l)" }
      ));

      std::cmatch match;
      for(auto const &pred : predicates)
      {
        while(std::regex_search<char const*>(begin, end, match, pred.regex))
        {
          if(pred.func)
          { pred.func(self); }

          auto const str(match.str());
          auto const it(begin + match.position());
          auto const sub_end(it + match.length());
          auto rit(begin);
          end = std::remove_if(begin, end, [&](char const)
          {
            auto const cur(rit++);
            return (cur >= it && cur < sub_end);
          });
        }
      }

      return end;
    }
  }
}
