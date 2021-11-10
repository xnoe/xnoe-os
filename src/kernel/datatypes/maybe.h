#ifndef MAYBE_H
#define MAYBE_H

namespace xnoe {
  template<typename T>
  class Maybe {
  private:
    T t;
    bool ok;

  public:
    Maybe() {
      this->ok = false;
    }

    Maybe(T t) {
      this->ok = true;
      this->t = t;
    }

    T get() {
      return t;
    }

    bool is_ok() {
      return ok;
    }
  };
}

#endif