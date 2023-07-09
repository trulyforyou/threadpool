#ifndef FUNCTION_WRAPPER_HPP_
#define FUNCTION_WRAPPER_HPP_

#include <functional>
#include <type_traits>

namespace lyc {

class function_wrapper {
 private:
  class impl_base {
   public:
    virtual void call() = 0;
    virtual ~impl_base() = default;
  };
  template <typename Fn, typename... Args>
  class impl_type : public impl_base {
   public:
    explicit impl_type(Fn&& f, Args&&... a)
        : f_(std::forward<Fn>(f)), args_(std::forward<Args>(a)...) {}
    void call() override { std::apply(f_, args_); }

   private:
    Fn f_;
    std::tuple<Args&&...> args_;
  };

 private:
  std::unique_ptr<impl_base> impl_;

 public:
  template <typename F, typename... Args,
            typename = std::enable_if_t<
                !std::is_same_v<std::decay_t<F>, function_wrapper>>>
  explicit function_wrapper(F&& f, Args&&... a)
      : impl_(new impl_type<decltype(f), decltype(a)...>(
            std::forward<F>(f), std::forward<Args>(a)...)) {}
  function_wrapper() = default;
  function_wrapper(function_wrapper&& other) noexcept
      : impl_(std::move(other.impl_)) {}
  function_wrapper& operator=(function_wrapper&& other) noexcept {
    impl_ = std::move(other.impl_);
    return *this;
  }
  function_wrapper(const function_wrapper&) = delete;
  function_wrapper& operator=(const function_wrapper&) = delete;
  void operator()() { impl_->call(); }
};

}  // namespace lyc
#endif  // FUNCTION_WRAPPER_HPP_