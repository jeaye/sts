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

    template <size_t N, typename It,
              typename T = typename std::iterator_traits<It>::value_type>
    size_t seq(It it, std::array<T, N> const &arr)
    { return std::equal(it, it + N, std::begin(arr)) * N; }
  }
}
