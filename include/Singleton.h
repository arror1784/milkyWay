#ifndef MILKYWAY_SINGLETON_H
#define MILKYWAY_SINGLETON_H

template<typename T>
class Singleton {
public:
  static T &getInstance() {
    static T instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }

  Singleton(Singleton const &) = delete;

  Singleton &operator=(Singleton const &) = delete;

  Singleton() {}

  ~Singleton() {}

};

#endif //MILKYWAY_SINGLETON_H
