#pragma once

#include <sys/types.h>

namespace sts
{
  namespace input
  {
    enum class event
    {
      mouse_wheel_up = 25,
      mouse_wheel_down = 5,
      carriage_return = 13
    };

    template <typename S, typename T>
    bool parse(S &scroller, T const &buf, ssize_t const num_read)
    {
      std::ofstream ofs{ ".in", std::ios_base::app };
      bool consumed{};
      for(ssize_t i{}; i < num_read; ++i)
      {
        event const ev{ static_cast<event>(buf[i]) };

        switch(ev)
        {
          case event::mouse_wheel_up:
            scroller.up();
            consumed = true;
            ofs << "mouse wheel up" << std::endl;
            break;
          case event::mouse_wheel_down:
            scroller.down();
            consumed = true;
            ofs << "mouse wheel down" << std::endl;
            break;
          case event::carriage_return:
            scroller.follow();
            ofs << "carriage return" << std::endl;
            break;
          default:
            break;
        }
      }

      return consumed;
    }
  }
}
