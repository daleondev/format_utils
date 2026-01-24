#pragma once

#include <reflect>

#include <format>
#include <array>
#include <concepts>
#include <format>
#include <functional>
#include <tuple>

namespace fmtu
{
    namespace detail
    {
        // ---------- Namespace Std ----------

        // clang-format off
        template<typename T>
        consteval auto namespace_name() noexcept -> std::string_view
        {
            using type_name_info = reflect::detail::type_name_info<std::remove_pointer_t<std::remove_cvref_t<T>>>;
            constexpr std::string_view function_name{
                reflect::detail::function_name<std::remove_pointer_t<std::remove_cvref_t<T>>>()
            };
            constexpr std::string_view qualified_type_name{ 
                function_name.substr(type_name_info::begin, function_name.find(type_name_info::end) - type_name_info::begin) 
            };
            constexpr std::string_view tmp_type_name{ 
                qualified_type_name.substr(0, qualified_type_name.find_first_of("<", 1)) 
            };
            constexpr std::string_view namespace_name{ 
                tmp_type_name.substr(0, tmp_type_name.find_last_of("::") + 1) 
            };
            return namespace_name;
        }
        // clang-format on

        template <typename T>
        consteval auto is_std_type() -> bool
        {
            return namespace_name<T>().starts_with("std::");
        }

        // ---------- General Concepts ----------

        template <typename T>
        concept ScopedEnum = std::is_scoped_enum_v<std::remove_cvref_t<T>>;

        template <typename T>
        concept VoidPtr = std::is_pointer_v<std::remove_cvref_t<T>> &&
                          std::is_void_v<std::remove_pointer_t<std::remove_cvref_t<T>>>;

        template <typename T>
        concept CharPtr =
            std::is_pointer_v<std::remove_cvref_t<T>> &&
            std::is_same_v<std::remove_cvref_t<std::remove_pointer_t<std::remove_cvref_t<T>>>, char>;

        template <typename T>
        concept ValuePtr = std::is_pointer_v<std::remove_cvref_t<T>> && !CharPtr<T> && !VoidPtr<T> &&
                           std::formattable<typename std::remove_pointer_t<std::remove_cvref_t<T>>, char>;

        template <typename T>
        concept SmartPtr = requires(std::remove_cvref_t<T> p) {
            typename std::remove_cvref_t<T>::element_type;
            { p.get() } -> std::convertible_to<const void *>;
            requires !std::is_aggregate_v<std::remove_cvref_t<T>>;
            requires std::formattable<typename std::remove_cvref_t<T>::element_type, char>;
        };

        // ---------- Adapter ----------

        template <typename T>
        struct get_class_type;

        template <typename MemberType, typename ClassType>
        struct get_class_type<MemberType ClassType::*>
        {
            using type = ClassType;
        };

        template <typename T>
        using get_class_type_t = typename get_class_type<T>::type;

        template <typename T>
        struct remove_member_pointer;

        template <typename Data, typename Class>
        struct remove_member_pointer<Data Class::*>
        {
            using type = Data;
        };

        template <typename T>
        using remove_member_pointer_t = typename remove_member_pointer<T>::type;
    }

    template <typename T>
    struct Adapter
    {
        using Fields = std::tuple<>;
    };

    template <reflect::fixed_string Name, auto Value>
        requires std::is_member_pointer_v<decltype(Value)>
    struct Field
    {
        using Type = std::conditional_t<
            std::is_member_object_pointer_v<decltype(Value)>,
            detail::remove_member_pointer_t<decltype(Value)>,
            std::invoke_result_t<decltype(Value), detail::get_class_type_t<decltype(Value)> &>>;
        static constexpr std::string_view NAME = Name;
        static constexpr auto VALUE = Value;
    };

    namespace detail
    {
        template <typename T>
        concept HasAdapter = std::is_class_v<std::remove_cvref_t<T>> &&
                             (std::tuple_size_v<typename Adapter<std::remove_cvref_t<T>>::Fields> > 0);

        template <HasAdapter T>
        consteval auto adapter_names()
        {
            using Type = std::remove_cvref_t<T>;
            return []<size_t... Is>(std::index_sequence<Is...>)
            {
                return std::array<std::string_view, sizeof...(Is)>{
                    std::tuple_element_t<Is, typename Adapter<Type>::Fields>::NAME...};
            }(std::make_index_sequence<std::tuple_size_v<typename Adapter<Type>::Fields>>{});
        }

        template <HasAdapter T>
        consteval auto adapter_types()
        {
            using Type = std::remove_cvref_t<T>;
            return []<size_t... Is>(std::index_sequence<Is...>)
            {
                return std::type_identity<
                    std::tuple<typename std::tuple_element_t<Is, typename Adapter<Type>::Fields>::Type...>>{};
            }(std::make_index_sequence<std::tuple_size_v<typename Adapter<Type>::Fields>>{});
        }

        template <HasAdapter T>
        using adapter_types_t = typename decltype(adapter_types<T>())::type;

        template <HasAdapter T>
        struct AdapterInfo
        {
            using Type = std::remove_cvref_t<T>;
            static constexpr std::string_view NAME{reflect::type_name<T>()};
            using MemberTypes = adapter_types_t<T>;
            static constexpr std::array MEMBER_NAMES{adapter_names<T>()};
            static consteval auto numMembers() -> std::size_t { return MEMBER_NAMES.size(); };
        };

        // ---------- Reflectable ----------

        template <typename T>
        concept Reflectable =
            std::is_class_v<std::remove_cvref_t<T>> && std::is_aggregate_v<std::remove_cvref_t<T>> &&
            !is_std_type<std::remove_cvref_t<T>>() && !HasAdapter<T>;

        template <Reflectable T>
        consteval auto reflect_names()
        {
            using Type = std::remove_cvref_t<T>;
            return []<size_t... Is>(std::index_sequence<Is...>)
            {
                return std::array<std::string_view, sizeof...(Is)>{reflect::member_name<Is, Type>()...};
            }(std::make_index_sequence<reflect::size<Type>()>{});
        }

        template <Reflectable T>
        consteval auto reflect_types()
        {
            using Type = std::remove_cvref_t<T>;
            return []<size_t... Is>(std::index_sequence<Is...>)
            {
                return std::type_identity<std::tuple<reflect::member_type<Is, Type>...>>{};
            }(std::make_index_sequence<reflect::size<Type>()>{});
        }

        template <Reflectable T>
        using reflect_types_t = typename decltype(reflect_types<T>())::type;

        template <Reflectable T>
        struct ReflectableInfo
        {
            using Type = std::remove_cvref_t<T>;
            static constexpr std::string_view NAME{reflect::type_name<T>()};
            using MemberTypes = reflect_types_t<T>;
            static constexpr std::array MEMBER_NAMES{reflect_names<T>()};
            static consteval auto numMembers() -> std::size_t { return MEMBER_NAMES.size(); };
        };

        // ---------- Formatting ----------

        template <typename R, typename T>
        concept RangeOf = std::ranges::range<R> && std::convertible_to<std::ranges::range_value_t<R>, T>;

        template <typename T>
        concept FormatInfo = requires {
            typename T::Type;
            { T::NAME } -> std::convertible_to<std::string_view>;
            typename T::MemberTypes;
            { T::MEMBER_NAMES } -> RangeOf<std::string_view>;
            { T::numMembers() } -> std::convertible_to<std::size_t>;
        };

        template <FormatInfo Info>
        consteval auto class_format_size() -> size_t
        {
            auto size{0uz};
            size += 2;                 // "[ "
            size += Info::NAME.size(); // "<class>"
            size += 5;                 // ": {{ "

            for (auto i{0uz}; i < Info::numMembers(); ++i)
            {
                size += Info::MEMBER_NAMES[i].size(); // "<member>"
                size += 4;                            // ": {}"
                if (i < Info::numMembers() - 1)
                {
                    size += 2; // ", "
                }
            }

            size += 5; // " }} ]"
            return size;
        }

        template <FormatInfo Info>
        consteval auto class_format()
        {
            std::array<char, class_format_size<Info>()> fmt{};

            auto iter{fmt.begin()};
            auto append = [&](std::string_view s)
            {
                for (char c : s)
                {
                    *iter++ = c;
                }
            };

            append("[ ");
            append(Info::NAME);
            append(": {{ ");

            for (auto i{0uz}; i < Info::numMembers(); ++i)
            {
                append(Info::MEMBER_NAMES[i]);
                append(": {}");
                if (i < Info::numMembers() - 1)
                {
                    append(", ");
                }
            }

            append(" }} ]");
            return reflect::fixed_string<char, fmt.size()>(fmt.data());
        }

        static constexpr std::string_view PRETTY_INDENT{"  "};

        template <FormatInfo Info, size_t Level = 0>
        consteval auto class_pretty_format_size() -> size_t
        {
            auto size{0uz};
            if constexpr (Level == 0)
            {
                size += Info::NAME.size(); // "<class>"
                size += 5;                 // ": {{\n"
            }
            else
            {
                size += 3; // "{{\n"
            }

            [&]<size_t... Is>(std::index_sequence<Is...>)
            {
                ([&](auto i)
                 {
                     constexpr size_t I = i;
                     using MemberType = std::tuple_element_t<I, typename Info::MemberTypes>;

                     size += (Level + 1) * PRETTY_INDENT.size();
                     size += Info::MEMBER_NAMES[I].size();
                     if constexpr (HasAdapter<MemberType>)
                     {
                         size += 2; // ": "
                         size += class_pretty_format_size<AdapterInfo<MemberType>, Level + 1>();
                     }
                     else if constexpr (Reflectable<MemberType>)
                     {
                         size += 2; // ": "
                         size += class_pretty_format_size<ReflectableInfo<MemberType>, Level + 1>();
                     }
                     else
                     {
                         size += 4; // ": {}"
                     }
                     if constexpr (I < Info::numMembers() - 1)
                     {
                         size += 1; // ","
                     }
                     size += 1; // "\n"
                 }(std::integral_constant<size_t, Is>{}),
                 ...);
            }(std::make_index_sequence<Info::numMembers()>{});

            size += Level * PRETTY_INDENT.size();
            size += 2; // "}}"
            return size;
            return 0;
        }

        template <FormatInfo Info, size_t Level = 0>
        consteval auto class_pretty_format()
        {
            std::array<char, class_pretty_format_size<Info, Level>()> fmt{};

            auto iter{fmt.begin()};
            auto append = [&](std::string_view s)
            {
                for (char c : s)
                {
                    *iter++ = c;
                }
            };

            if constexpr (Level == 0)
            {
                append(Info::NAME);
                append(": {{\n");
            }
            else
            {
                append("{{\n");
            }

            [&]<size_t... Is>(std::index_sequence<Is...>)
            {
                ([&](auto i)
                 {
                    constexpr size_t I = i;
                    using MemberType = std::tuple_element_t<I, typename Info::MemberTypes>;

                    for (auto i{ 0uz }; i < (Level + 1); ++i) {
                        append(PRETTY_INDENT);
                    }
                    append(Info::MEMBER_NAMES[I]);

                    if constexpr (HasAdapter<MemberType>) {
                        append(": ");
                        append(class_pretty_format<AdapterInfo<MemberType>, Level + 1>());
                    }
                    else if constexpr (Reflectable<MemberType>) {
                        append(": ");
                        append(class_pretty_format<ReflectableInfo<MemberType>, Level + 1>());
                    }
                    else {
                        append(": {}");
                    }

                    if constexpr (I < Info::numMembers() - 1) {
                        append(",");
                    }
                    append("\n"); }(std::integral_constant<size_t, Is>{}), ...);
            }(std::make_index_sequence<Info::numMembers()>{});

            for (auto i{0uz}; i < Level; ++i)
            {
                append(PRETTY_INDENT);
            }
            append("}}");
            return reflect::fixed_string<char, fmt.size()>(fmt.data());
        }

        // ---------- Format arguments ----------

        template <typename T>
        constexpr auto check_arg(T &&field) -> decltype(auto)
        {
            if constexpr (std::formattable<T, char>)
            {
                return std::forward<T>(field);
            }
            else
            {
                return "-";
            }
        };

        template <typename... Args>
        constexpr auto make_args_tuple(Args &&...args)
        {
            using Tuple = std::tuple<
                std::conditional_t<std::is_lvalue_reference_v<Args>, Args, std::remove_reference_t<Args>>...>;
            return Tuple(std::forward<Args>(args)...);
        };

        template <typename T>
        constexpr auto make_flat_args_tuple(T &&val)
        {
            using Type = std::remove_cvref_t<T>;
            if constexpr (fmtu::detail::HasAdapter<Type>)
            {
                using Fields = typename fmtu::Adapter<Type>::Fields;
                return [&]<size_t... Is>(std::index_sequence<Is...>)
                {
                    return std::tuple_cat(
                        make_flat_args_tuple(std::invoke(std::tuple_element_t<Is, Fields>::VALUE, val))...);
                }(std::make_index_sequence<std::tuple_size_v<Fields>>{});
            }
            else if constexpr (fmtu::detail::Reflectable<Type>)
            {
                return [&]<size_t... Is>(std::index_sequence<Is...>)
                {
                    return std::tuple_cat(make_flat_args_tuple(reflect::get<Is>(val))...);
                }(std::make_index_sequence<reflect::size<Type>()>{});
            }
            else
            {
                return fmtu::detail::make_args_tuple(
                    fmtu::detail::check_arg(std::forward<T>(val)));
            }
        };
    }
}

template <fmtu::detail::HasAdapter T>
struct std::formatter<T>
{
    using Info = fmtu::detail::AdapterInfo<T>;

    bool pretty{false};

    template <typename Ctx>
    constexpr auto parse(Ctx &ctx) -> Ctx::iterator
    {
        auto it{ctx.begin()};
        if (it == ctx.end())
        {
            return it;
        }

        if (*it == 'p')
        {
            pretty = true;
            ++it;
        }

        if (it != ctx.end() && *it != '}')
        {
            throw std::format_error("Invalid format args");
        }

        return it;
    }

    template <typename Ctx>
    auto format(const T &t, Ctx &ctx) const -> Ctx::iterator
    {
        if (pretty)
        {
            auto args{fmtu::detail::make_flat_args_tuple(t)};
            static constexpr auto fmt{fmtu::detail::class_pretty_format<Info>()};
            return std::apply([&ctx](const auto &...args)
                              { return std::format_to(ctx.out(), fmt, args...); },
                              args);
        }
        else
        {
            auto args{[&]<size_t... Is>(std::index_sequence<Is...>)
                      {
                          using Fields = typename fmtu::Adapter<std::remove_cvref_t<T>>::Fields;
                          return fmtu::detail::make_args_tuple(fmtu::detail::check_arg(
                              std::invoke(std::tuple_element_t<Is, Fields>::VALUE, t))...);
                      }(std::make_index_sequence<Info::numMembers()>{})};

            static constexpr auto fmt{fmtu::detail::class_format<Info>()};
            return std::apply([&ctx](const auto &...args)
                              { return std::format_to(ctx.out(), fmt, args...); },
                              args);
        }
        return ctx.out();
    }
};

template <fmtu::detail::Reflectable T>
struct std::formatter<T>
{
    static_assert(
        requires { T{}; },
        "Type T contains reference members or other non-value-initializable members, which are not supported "
        "for automatic formatting. Consider removing references or providing default initializers.");

    using Info = fmtu::detail::ReflectableInfo<T>;

    bool pretty{false};

    template <typename Ctx>
    constexpr auto parse(Ctx &ctx) -> Ctx::iterator
    {
        auto it{ctx.begin()};
        if (it == ctx.end())
        {
            return it;
        }

        if (*it == 'p')
        {
            pretty = true;
            ++it;
        }

        if (it != ctx.end() && *it != '}')
        {
            throw std::format_error("Invalid format args");
        }

        return it;
    }

    template <typename Ctx>
    auto format(const T &t, Ctx &ctx) const -> Ctx::iterator
    {
        if (pretty)
        {
            auto args{fmtu::detail::make_flat_args_tuple(t)};
            static constexpr auto fmt{fmtu::detail::class_pretty_format<Info>()};
            return std::apply([&ctx](const auto &...args)
                              { return std::format_to(ctx.out(), fmt, args...); },
                              args);
        }
        else
        {
            auto args{[&]<size_t... Is>(std::index_sequence<Is...>)
                      {
                          return fmtu::detail::make_args_tuple(
                              fmtu::detail::check_arg(reflect::get<Is>(t))...);
                      }(std::make_index_sequence<reflect::size<T>()>{})};

            static constexpr auto fmt{fmtu::detail::class_format<Info>()};
            return std::apply([&ctx](const auto &...args)
                              { return std::format_to(ctx.out(), fmt, args...); },
                              args);
        }
        return ctx.out();
    }
};

template <fmtu::detail::ScopedEnum T>
struct std::formatter<T>
{
    bool verbose{false};

    template <typename Ctx>
    constexpr auto parse(Ctx &ctx) -> Ctx::iterator
    {
        auto it{ctx.begin()};
        if (it == ctx.end())
        {
            return it;
        }

        if (*it == 'v')
        {
            verbose = true;
            ++it;
        }

        if (it != ctx.end() && *it != '}')
        {
            throw std::format_error("Invalid format args");
        }

        return it;
    }

    template <typename Ctx>
    auto format(T t, Ctx &ctx) const -> Ctx::iterator
    {
        if (verbose)
        {
            return std::format_to(ctx.out(), "{}:{}", reflect::type_name(t), reflect::enum_name(t));
        }
        return std::format_to(ctx.out(), "{}", reflect::enum_name(t));
    }
};

template <std::formattable<char> T>
struct std::formatter<std::optional<T>> : std::formatter<T>
{
    using Base = std::formatter<T>;

    template <typename Ctx>
    auto format(const std::optional<T> &t, Ctx &ctx) const -> Ctx::iterator
    {
        if (!t)
        {
            return std::ranges::copy("[ null ]"sv, ctx.out()).out;
        }

        ctx.advance_to(std::ranges::copy("[ "sv, ctx.out()).out);
        ctx.advance_to(Base::format(*t, ctx));
        return std::ranges::copy(" ]"sv, ctx.out()).out;
    }
};

template <fmtu::detail::ValuePtr T>
struct std::formatter<T> : std::formatter<std::remove_pointer_t<std::remove_cvref_t<T>>>
{
    using Base = std::formatter<std::remove_pointer_t<std::remove_cvref_t<T>>>;

    template <typename Ctx>
    auto format(T t, Ctx &ctx) const -> Ctx::iterator
    {
        if (!t)
        {
            return std::format_to(ctx.out(), "[ ({}) -> {} ]", static_cast<const void *>(t), "null");
        }

        ctx.advance_to(std::format_to(ctx.out(), "[ ({}) -> ", static_cast<const void *>(t)));
        ctx.advance_to(Base::format(*t, ctx));
        return std::ranges::copy(" ]"sv, ctx.out()).out;
    }
};

template <fmtu::detail::SmartPtr T>
struct std::formatter<T> : std::formatter<typename T::element_type *>
{
    template <typename Ctx>
    auto format(const T &t, Ctx &ctx) const -> Ctx::iterator
    {
        return std::formatter<typename T::element_type *>::format(t.get(), ctx);
    }
};
