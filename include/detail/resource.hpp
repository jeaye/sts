#pragma once

#include <algorithm>
#include <functional>

namespace sts
{
  namespace detail
  {
    template <typename T>
    class resource
    {
      public:
        using dtor_t = std::function<void (T&)>;

        resource() = delete;
        resource(dtor_t &&dtor)
          : dtor_{ std::move(dtor) }
        { }
        resource(resource const&) = delete;
        resource(resource &&) = default;
        resource(T &&t, dtor_t const &dtor) 
          : data_(std::move(t)), dtor_{ dtor }
        { }
        ~resource()
        { dtor_(data_); }

        resource& operator =(resource const&) = delete;
        resource& operator =(resource &&r) noexcept
        {
          dtor_(data_);
          data_ = std::move(r.data_);
          r.data_ = {};
          return *this;
        }
        resource& operator =(T &&t)
        {
          dtor_(data_);
          data_ = std::move(t);
          return *this;
        }

        T& get()
        { return data_; }
        T const& get() const
        { return data_; }

      private:
        T data_{};
        dtor_t dtor_;
    };
  }
}
