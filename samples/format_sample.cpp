#include "format_utils.hpp"
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

class MyClassB
{
  public:
    MyClassB(int i, std::string s)
      : m_i(i)
      , m_s(s)
    {
    }

    int m_i;
    std::string m_s;
};

template<>
struct fmtu::Adapter<MyClassB>
{
    using Fields = std::tuple<fmtu::Field<"i", &MyClassB::m_i>, fmtu::Field<"s", &MyClassB::m_s>>;
};

struct Nested
{
    int x = 1;
    MyClassA my_class{ 42, "hello" };
};

int main()
{
    MyClassA a(42, "hello");
    MyClassB b(44, "hello2");
    std::println("JSON: {:pj}", a);
    std::println("JSON: {:pj}", b);
}