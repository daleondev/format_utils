#include "format_utils.hpp"

#include <print>
#include <mutex>

class MyClassInner
{
public:
    int getX() const { return m_x; }

    int m_y = 10;

private:
    int m_x = 5;
};

struct Inner
{
    std::string s = "lol";
    double d = 2346.347;
};

class MyClass
{
public:
    int getI() const { return m_i; }
    const MyClassInner &getInner() const { return m_inner; }
    const Inner &getInnerReflectable() const { return m_innerReflectable; }

    int m_j = 10;

private:
    MyClassInner m_inner{};
    Inner m_innerReflectable{};
    int m_i = 5;
};

struct TestStruct
{
    int i = 1;
    float f = 5.5f;
    Inner inner{};
    const char *str = "Test";
    MyClassInner innerClass{};
    std::mutex m;
};

template <>
struct fmtu::Adapter<MyClassInner>
{
    using Fields = std::tuple<Field<"x", &MyClassInner::getX>, Field<"y", &MyClassInner::m_y>>;
};

template <>
struct fmtu::Adapter<MyClass>
{
    using Fields = std::tuple<Field<"i", &MyClass::getI>,
                              Field<"inner", &MyClass::getInner>,
                              Field<"innerReflectable", &MyClass::getInnerReflectable>,
                              Field<"j", &MyClass::m_j>>;
};

int main(int argc, char *argv[])
{
    std::println("{:p}", MyClass{});
    std::println("{:p}", TestStruct{});
}