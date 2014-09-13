#pragma once

#include <array>
#include <type_traits>

namespace sts
{
  namespace detail
  {
    template <typename T, typename... Ts>
    std::array<std::decay_t<T>, sizeof...(Ts) + 1> make_array(T &&t, Ts &&... ts)
    { return { { t, ts... } }; }

    template <std::size_t N, typename It,
              typename T = typename std::iterator_traits<It>::value_type>
    std::size_t seq_eq(std::size_t const d, It it, std::array<T, N> const &arr)
    { return (d >= N && std::equal(it, it + N, std::begin(arr))) * N; }
  }
}
