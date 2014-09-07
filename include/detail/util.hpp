#pragma once

#include <array>
#include <type_traits>

namespace sts
{
  namespace detail
  {
    template <typename T, typename... Ts>
    std::array<typename std::decay<T>::type, sizeof...(Ts) + 1> make_array(T &&t, Ts &&... ts)
    { return { { t, ts... } }; }
  }
}
