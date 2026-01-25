#include "format_utils.hpp"
#include <mutex>
#include <print>

class MyClassA
{
  public:
    MyClassA(int i, std::string s)
      : m_i(i)
      , m_s(s)
    {
    }
    int getI() const { return m_i; }
    std::string getS() const { return m_s; }

  private:
    int m_i;
    std::string m_s;
};

template<>
struct fmtu::Adapter<MyClassA>
{
    using Fields = std::tuple<fmtu::Field<"i", &MyClassA::getI>, fmtu::Field<"s", &MyClassA::getS>>;
};

struct Nested
{
    int x = 1;
    MyClassA my_class_a{ -265, "test" };
};

class MyClassB
{
  public:
    MyClassB(int i, std::string s)
      : m_i(i)
      , m_s(s)
      , m_nested()
    {
    }

    int m_i;
    std::string m_s;
    std::mutex m_mutex;

    const Nested& getNested() const { return m_nested; }

  private:
    Nested m_nested;
};

template<>
struct fmtu::Adapter<MyClassB>
{
    using Fields = std::tuple<fmtu::Field<"i", &MyClassB::m_i>,
                              fmtu::Field<"s", &MyClassB::m_s>,
                              fmtu::Field<"nested", &MyClassB::getNested>>;
};

int main()
{
    MyClassA a(42, "hello");
    MyClassB b(44, "hello2");
    std::println("JSON: {:pj}", a);
    std::println("JSON: {:pj}", b);
    std::println("JSON: {:pj}", Nested{});
}