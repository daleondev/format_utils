#pragma once

#ifdef FMTU_ENABLE_JSON
#include <glaze/json.hpp>
#endif
#ifdef FMTU_ENABLE_YAML
#include <glaze/yaml.hpp>
#endif
#ifdef FMTU_ENABLE_TOML
#include <glaze/toml.hpp>
#endif

#include <reflect>

#include <algorithm>
#include <array>
#include <concepts>
#include <format>
#include <functional>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

namespace fmtu
{
#ifdef FMTU_ENABLE_GLAZE
    static constexpr bool IS_GLAZE_ENABLED{ true };
#else
    static constexpr bool IS_GLAZE_ENABLED{ false };
#endif
#ifdef FMTU_ENABLE_JSON
    static constexpr bool IS_JSON_ENABLED{ true };
#else
    static constexpr bool IS_JSON_ENABLED{ false };
#endif
#ifdef FMTU_ENABLE_YAML
    static constexpr bool IS_YAML_ENABLED{ true };
#else
    static constexpr bool IS_YAML_ENABLED{ false };
#endif
#ifdef FMTU_ENABLE_TOML
    static constexpr bool IS_TOML_ENABLED{ true };
#else
    static constexpr bool IS_TOML_ENABLED{ false };
#endif

    namespace detail
    {
        // ---------- Constexpr data types ----------

        template<typename T, size_t Capacity>
        struct FixedVector
        {
            std::array<T, Capacity> data{};
            size_t size{ 0 };

            constexpr FixedVector() = default;

            template<size_t Size>
            constexpr FixedVector(std::array<T, Size>&& arr)
              : size{ Size }
            {
                static_assert(Size <= Capacity, "Input array exceeds FixedVector capacity");
                std::copy_n(std::make_move_iterator(arr.begin()), Size, data.begin());
            }

            constexpr auto add(T val) -> void
            {
                if (size >= Capacity) {
                    throw std::out_of_range("FixedMap is full!");
                }
                data[size++] = std::move(val);
            }

            constexpr auto begin() const { return data.begin(); }
            constexpr auto end() const { return data.begin() + size; }
        };

        template<typename Key, typename Value, size_t Capacity>
        struct FixedMap
        {
            std::array<std::pair<Key, Value>, Capacity> data{};
            size_t size = 0;

            template<typename... Args>
            constexpr void emplace(Args&&... args)
            {
                auto args_tuple = std::forward_as_tuple(args...);

                auto it{ std::ranges::find_if(
                  *this, [&args_tuple](const auto& pair) { return pair.first == std::get<0>(args_tuple); }) };

                if (it != end()) {
                    it->second = std::move(std::get<1>(args_tuple));
                    return;
                }

                if (size >= Capacity) {
                    throw std::out_of_range("FixedMap is full!");
                }

                std::construct_at(&data[size++], std::forward<Args>(args)...);
            }

            constexpr auto at(const Key& key) const -> std::optional<Value>
            {
                auto it{ std::find_if(
                  data.begin(), data.end(), [&key](const auto& pair) { return pair.first == key; }) };

                if (it != data.end()) {
                    return it->second;
                }

                return std::nullopt;
            }

            constexpr auto begin() { return data.begin(); }
            constexpr auto end() { return data.begin() + size; }
            constexpr auto begin() const { return data.begin(); }
            constexpr auto end() const { return data.begin() + size; }
        };

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

        template<typename T>
        consteval auto is_std_type() -> bool
        {
            return namespace_name<T>().starts_with("std::");
        }

        // ---------- General Concepts ----------

        template<typename T>
        concept ScopedEnum = std::is_scoped_enum_v<std::remove_cvref_t<T>>;

        template<typename T>
        concept VoidPtr = std::is_pointer_v<std::remove_cvref_t<T>> &&
                          std::is_void_v<std::remove_pointer_t<std::remove_cvref_t<T>>>;

        template<typename T>
        concept CharPtr =
          std::is_pointer_v<std::remove_cvref_t<T>> &&
          std::is_same_v<std::remove_cvref_t<std::remove_pointer_t<std::remove_cvref_t<T>>>, char>;

        template<typename T>
        concept ValuePtr = std::is_pointer_v<std::remove_cvref_t<T>> && !CharPtr<T> && !VoidPtr<T> &&
                           std::formattable<typename std::remove_pointer_t<std::remove_cvref_t<T>>, char>;

        template<typename T>
        concept SmartPtr = requires(std::remove_cvref_t<T> p) {
            typename std::remove_cvref_t<T>::element_type;
            { p.get() } -> std::convertible_to<const void*>;
            requires !std::is_aggregate_v<std::remove_cvref_t<T>>;
            requires std::formattable<typename std::remove_cvref_t<T>::element_type, char>;
        };

        // ---------- Adapter ----------

        template<typename T>
        struct get_class_type;

        template<typename MemberType, typename ClassType>
        struct get_class_type<MemberType ClassType::*>
        {
            using type = ClassType;
        };

        template<typename T>
        using get_class_type_t = typename get_class_type<T>::type;

        template<typename T>
        struct remove_member_pointer;

        template<typename Data, typename Class>
        struct remove_member_pointer<Data Class::*>
        {
            using type = Data;
        };

        template<typename T>
        using remove_member_pointer_t = typename remove_member_pointer<T>::type;
    }

    template<typename T>
    struct Adapter
    {
        using Fields = std::tuple<>;
    };

    template<reflect::fixed_string Name, auto Value>
        requires std::is_member_pointer_v<decltype(Value)>
    struct Field
    {
        using Type = std::conditional_t<
          std::is_member_object_pointer_v<decltype(Value)>,
          detail::remove_member_pointer_t<decltype(Value)>,
          std::invoke_result_t<decltype(Value), detail::get_class_type_t<decltype(Value)>&>>;
        static constexpr std::string_view NAME = Name;
        static constexpr auto VALUE = Value;
    };

    namespace detail
    {
        template<typename T>
        concept HasAdapter = std::is_class_v<std::remove_cvref_t<T>> &&
                             (std::tuple_size_v<typename Adapter<std::remove_cvref_t<T>>::Fields> > 0);

        template<HasAdapter T>
        consteval auto adapter_names()
        {
            using Type = std::remove_cvref_t<T>;
            return []<size_t... Is>(std::index_sequence<Is...>) {
                return std::array<std::string_view, sizeof...(Is)>{
                    std::tuple_element_t<Is, typename Adapter<Type>::Fields>::NAME...
                };
            }(std::make_index_sequence<std::tuple_size_v<typename Adapter<Type>::Fields>>{});
        }

        template<HasAdapter T>
        consteval auto adapter_types()
        {
            using Type = std::remove_cvref_t<T>;
            return []<size_t... Is>(std::index_sequence<Is...>) {
                return std::type_identity<
                  std::tuple<typename std::tuple_element_t<Is, typename Adapter<Type>::Fields>::Type...>>{};
            }(std::make_index_sequence<std::tuple_size_v<typename Adapter<Type>::Fields>>{});
        }

        template<HasAdapter T>
        using adapter_types_t = typename decltype(adapter_types<T>())::type;

        template<HasAdapter T>
        struct AdapterInfo
        {
            using Type = std::remove_cvref_t<T>;
            static constexpr std::string_view NAME{ reflect::type_name<T>() };
            using MemberTypes = adapter_types_t<T>;
            static constexpr std::array MEMBER_NAMES{ adapter_names<T>() };
            static consteval auto numMembers() -> size_t { return MEMBER_NAMES.size(); };
        };

        // ---------- Reflectable ----------

        template<typename T>
        concept Reflectable =
          std::is_class_v<std::remove_cvref_t<T>> && std::is_aggregate_v<std::remove_cvref_t<T>> &&
          !is_std_type<std::remove_cvref_t<T>>() && !HasAdapter<T>;

        template<Reflectable T>
        consteval auto reflect_names()
        {
            using Type = std::remove_cvref_t<T>;
            return []<size_t... Is>(std::index_sequence<Is...>) {
                return std::array<std::string_view, sizeof...(Is)>{ reflect::member_name<Is, Type>()... };
            }(std::make_index_sequence<reflect::size<Type>()>{});
        }

        template<Reflectable T>
        consteval auto reflect_types()
        {
            using Type = std::remove_cvref_t<T>;
            return []<size_t... Is>(std::index_sequence<Is...>) {
                return std::type_identity<std::tuple<reflect::member_type<Is, Type>...>>{};
            }(std::make_index_sequence<reflect::size<Type>()>{});
        }

        template<Reflectable T>
        using reflect_types_t = typename decltype(reflect_types<T>())::type;

        template<Reflectable T>
        struct ReflectableInfo
        {
            using Type = std::remove_cvref_t<T>;
            static constexpr std::string_view NAME{ reflect::type_name<T>() };
            using MemberTypes = reflect_types_t<T>;
            static constexpr std::array MEMBER_NAMES{ reflect_names<T>() };
            static consteval auto numMembers() -> size_t { return MEMBER_NAMES.size(); };
        };

        // ---------- Formatting ----------

        template<typename T>
        struct is_array : std::false_type
        {
        };

        template<typename T, size_t N>
        struct is_array<std::array<T, N>> : std::true_type
        {
        };

        template<typename A, typename T>
        concept ArrayOf = is_array<std::remove_cvref_t<A>>::value &&
                          std::convertible_to<typename std::remove_cvref_t<A>::value_type, T>;

        template<typename T>
        concept FormatInfo = requires {
            typename T::Type;
            { T::NAME } -> std::convertible_to<std::string_view>;
            typename T::MemberTypes;
            { T::MEMBER_NAMES } -> ArrayOf<std::string_view>;
            { T::numMembers() } -> std::convertible_to<size_t>;
        };

        template<FormatInfo Info>
        consteval auto class_format_size() -> size_t
        {
            auto size{ 0uz };
            size += 2;                 // "[ "
            size += Info::NAME.size(); // "<class>"
            size += 5;                 // ": {{ "

            for (auto i{ 0uz }; i < Info::numMembers(); ++i) {
                size += Info::MEMBER_NAMES[i].size(); // "<member>"
                size += 4;                            // ": {}"
                if (i < Info::numMembers() - 1) {
                    size += 2; // ", "
                }
            }

            size += 5; // " }} ]"
            return size;
        }

        template<FormatInfo Info>
        consteval auto class_format()
        {
            std::array<char, class_format_size<Info>()> fmt{};

            auto iter{ fmt.begin() };
            auto append = [&](std::string_view s) {
                for (char c : s) {
                    *iter++ = c;
                }
            };

            append("[ ");
            append(Info::NAME);
            append(": {{ ");

            for (auto i{ 0uz }; i < Info::numMembers(); ++i) {
                append(Info::MEMBER_NAMES[i]);
                append(": {}");
                if (i < Info::numMembers() - 1) {
                    append(", ");
                }
            }

            append(" }} ]");
            return reflect::fixed_string<char, fmt.size()>(fmt.data());
        }

        static constexpr std::string_view PRETTY_INDENT{ "  " };

        template<FormatInfo Info, size_t Level = 0>
        consteval auto class_pretty_format_size() -> size_t
        {
            auto size{ 0uz };
            if constexpr (Level == 0) {
                size += Info::NAME.size(); // "<class>"
                size += 5;                 // ": {{\n"
            }
            else {
                size += 3; // "{{\n"
            }

            [&]<size_t... Is>(std::index_sequence<Is...>) {
                ([&](auto i) {
                    constexpr size_t I = i;
                    using MemberType = std::tuple_element_t<I, typename Info::MemberTypes>;

                    size += (Level + 1) * PRETTY_INDENT.size();
                    size += Info::MEMBER_NAMES[I].size();
                    if constexpr (HasAdapter<MemberType>) {
                        size += 2; // ": "
                        size += class_pretty_format_size<AdapterInfo<MemberType>, Level + 1>();
                    }
                    else if constexpr (Reflectable<MemberType>) {
                        size += 2; // ": "
                        size += class_pretty_format_size<ReflectableInfo<MemberType>, Level + 1>();
                    }
                    else {
                        size += 4; // ": {}"
                    }
                    if constexpr (I < Info::numMembers() - 1) {
                        size += 1; // ","
                    }
                    size += 1; // "\n"
                }(std::integral_constant<size_t, Is>{}), ...);
            }(std::make_index_sequence<Info::numMembers()>{});

            size += Level * PRETTY_INDENT.size();
            size += 2; // "}}"
            return size;
            return 0;
        }

        template<FormatInfo Info, size_t Level = 0>
        consteval auto class_pretty_format()
        {
            std::array<char, class_pretty_format_size<Info, Level>()> fmt{};

            auto iter{ fmt.begin() };
            auto append = [&](std::string_view s) {
                for (char c : s) {
                    *iter++ = c;
                }
            };

            if constexpr (Level == 0) {
                append(Info::NAME);
                append(": {{\n");
            }
            else {
                append("{{\n");
            }

            [&]<size_t... Is>(std::index_sequence<Is...>) {
                ([&](auto i) {
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
                    append("\n");
                }(std::integral_constant<size_t, Is>{}), ...);
            }(std::make_index_sequence<Info::numMembers()>{});

            for (auto i{ 0uz }; i < Level; ++i) {
                append(PRETTY_INDENT);
            }
            append("}}");
            return reflect::fixed_string<char, fmt.size()>(fmt.data());
        }

        // ---------- Format arguments ----------

        template<typename T>
        constexpr auto check_arg(T&& field) -> decltype(auto)
        {
            if constexpr (std::formattable<T, char>) {
                return std::forward<T>(field);
            }
            else {
                return "-";
            }
        };

        template<typename... Args>
        constexpr auto make_args_tuple(Args&&... args)
        {
            using Tuple = std::tuple<
              std::conditional_t<std::is_lvalue_reference_v<Args>, Args, std::remove_reference_t<Args>>...>;
            return Tuple(std::forward<Args>(args)...);
        };

        template<typename T>
        constexpr auto make_flat_args_tuple(T&& val)
        {
            using Type = std::remove_cvref_t<T>;
            if constexpr (fmtu::detail::HasAdapter<Type>) {
                using Fields = typename fmtu::Adapter<Type>::Fields;
                return [&]<size_t... Is>(std::index_sequence<Is...>) {
                    return std::tuple_cat(
                      make_flat_args_tuple(std::invoke(std::tuple_element_t<Is, Fields>::VALUE, val))...);
                }(std::make_index_sequence<std::tuple_size_v<Fields>>{});
            }
            else if constexpr (fmtu::detail::Reflectable<Type>) {
                return [&]<size_t... Is>(std::index_sequence<Is...>) {
                    return std::tuple_cat(make_flat_args_tuple(reflect::get<Is>(val))...);
                }(std::make_index_sequence<reflect::size<Type>()>{});
            }
            else {
                return fmtu::detail::make_args_tuple(fmtu::detail::check_arg(std::forward<T>(val)));
            }
        };

        template<ScopedEnum T>
        consteval auto underlying_enumerators()
        {
            constexpr auto min{ reflect::enum_min(T{}) };
            constexpr auto max{ reflect::enum_max(T{}) };
            return reflect::detail::enum_cases<T, min, max>;
        }

        template<ScopedEnum T>
        consteval auto num_enumerators() -> size_t
        {
            return underlying_enumerators<T>().size();
        }

        // clang-format off
        template<ScopedEnum T>
        consteval auto enumerators() -> std::array<T, num_enumerators<T>()>
        {
            std::array<T, num_enumerators<T>()> result{};
            std::ranges::copy(underlying_enumerators<T>() |
                                std::views::transform([](std::underlying_type_t<T> u) { 
                                    return static_cast<T>(u); 
                                }),
                                result.begin());
            return result;
        }

        template<ScopedEnum T, size_t N>
        consteval auto enumerators_complement(const std::array<T, N>& arr) -> decltype(auto)
        {
            std::array<T, num_enumerators<T>() - arr.size()> result;
            std::ranges::copy(enumerators<T>() | 
                                std::views::filter([&arr](T e) { 
                                    return std::ranges::none_of(arr, [e](T t) {
                                        return e == t;
                                    }); 
                                }),
                              result.begin());
            return result;
        }
        // clang-format on

        template<typename T, size_t N>
        consteval auto is_array_unique(std::array<T, N> arr) -> bool
        {
            std::ranges::sort(arr);
            return std::ranges::adjacent_find(arr) == arr.end();
        }

        enum class FmtSpecs : char
        {
            Verbose = 'v',
            Pretty = 'p',
            Json = 'j',
            Yaml = 'y',
            Toml = 't'
        };
        static constexpr auto NUM_FMT_SPECS{ num_enumerators<FmtSpecs>() };

        struct FmtOpts
        {
            bool verbose;
            bool pretty;
            bool json;
            bool yaml;
            bool toml;

            constexpr bool operator==(const FmtOpts&) const = default;
            constexpr auto has_opt() const -> bool { return *this != FmtOpts{}; }
            constexpr auto has_glaze() const -> bool;
        };

        static constexpr std::array GLAZE_FMT_SPECS{ FmtSpecs::Json, FmtSpecs::Yaml, FmtSpecs::Toml };

        // clang-format off
        static constexpr FixedMap<FmtSpecs, bool FmtOpts::*, NUM_FMT_SPECS> FMT_SPECS_TO_OPTS{{{ 
            std::make_pair(FmtSpecs::Verbose,   &FmtOpts::verbose),
            std::make_pair(FmtSpecs::Pretty,    &FmtOpts::pretty),
            std::make_pair(FmtSpecs::Json,      &FmtOpts::json),
            std::make_pair(FmtSpecs::Yaml,      &FmtOpts::yaml),
            std::make_pair(FmtSpecs::Toml,      &FmtOpts::toml) 
        }}};
        // clang-format on

        constexpr auto FmtOpts::has_glaze() const -> bool
        {
            return std::ranges::any_of(GLAZE_FMT_SPECS, [this](FmtSpecs spec) {
                auto opt{ FMT_SPECS_TO_OPTS.at(spec) };
                if (!opt.has_value()) {
                    return false;
                }
                return (*this).*opt.value();
            });
        }

        static constexpr std::array COMPATIBLE_FMT_SPEC_PAIRS{
            std::make_pair(FmtSpecs::Verbose, FmtSpecs::Pretty),
            std::make_pair(FmtSpecs::Pretty, FmtSpecs::Json)
        };

        static_assert(is_array_unique(COMPATIBLE_FMT_SPEC_PAIRS),
                      "Compatible format specifier pairs not unique");

        consteval auto generate_incompatible_specs()
        {
            FixedMap<FmtSpecs, FixedVector<FmtSpecs, NUM_FMT_SPECS - 1>, NUM_FMT_SPECS> incompatible_specs;

            constexpr auto specs{ enumerators<FmtSpecs>() };
            for (auto spec : specs) {
                FixedVector<FmtSpecs, NUM_FMT_SPECS - 1> incompatible{};

                auto incompatible_view{ specs | std::views::filter([spec](auto other) {
                    if (spec == other) {
                        return false;
                    }
                    return std::ranges::none_of(COMPATIBLE_FMT_SPEC_PAIRS, [&](const auto& pair) {
                        return (pair.first == spec && pair.second == other) ||
                               (pair.first == other && pair.second == spec);
                    });
                }) };

                for (auto incompatible_spec : incompatible_view) {
                    incompatible.add(incompatible_spec);
                }

                incompatible_specs.emplace(spec, incompatible);
            }

            return incompatible_specs;
        }

        static constexpr auto FMT_INCOMPATIBEL_SPECS{ generate_incompatible_specs() };

        template<FmtOpts AllowedOpts, typename Ctx>
        constexpr auto parse_fmt_opts(Ctx& ctx, FmtOpts& active_opts) -> Ctx::iterator
        {
            auto it{ ctx.begin() };

            std::vector<FmtSpecs> incompatibel_specs{};
            while (it != ctx.end()) {
                auto spec_char{ *it };
                if (spec_char == '}') {
                    return it;
                }

                auto spec{ static_cast<FmtSpecs>(spec_char) };
                if (std::ranges::contains(incompatibel_specs, spec)) {
                    throw std::format_error("Invalid format specifier");
                }

                auto opt{ FMT_SPECS_TO_OPTS.at(spec) };
                if (!opt.has_value() || !(AllowedOpts.*opt.value())) {
                    throw std::format_error("Invalid format specifier");
                }
                active_opts.*opt.value() = true;

                if (auto inc_specs{ FMT_INCOMPATIBEL_SPECS.at(spec) }; inc_specs.has_value()) {
                    incompatibel_specs.insert(incompatibel_specs.end(), inc_specs->begin(), inc_specs->end());
                }

                ++it;
            }

            return it;
        }

#ifdef FMTU_ENABLE_GLAZE
        template<typename T>
        consteval bool is_type_glaze_serializable();

        template<FormatInfo Info>
        consteval bool is_info_glaze_serializable()
        {
            using MemberTypes = typename Info::MemberTypes;
            return []<size_t... Is>(std::index_sequence<Is...>) {
                return (is_type_glaze_serializable<std::tuple_element_t<Is, MemberTypes>>() && ...);
            }(std::make_index_sequence<Info::numMembers()>{});
        }

        template<typename T>
        consteval bool is_type_glaze_serializable()
        {
            if constexpr (HasAdapter<T>) {
                return is_info_glaze_serializable<AdapterInfo<T>>();
            }
            else if constexpr (Reflectable<T>) {
                return is_info_glaze_serializable<ReflectableInfo<T>>();
            }
            else {
                return glz::write_supported<T, glz::JSON>;
            }
        }

        template<typename Field>
        consteval auto glaze_field_value() -> decltype(auto)
        {
            if constexpr (std::is_member_function_pointer_v<decltype(Field::VALUE)>) {
                return glz::custom<nullptr, Field::VALUE>;
            }
            else {
                return Field::VALUE;
            }
        }

        template<typename T>
        struct GlazeAdapter
        {
            using Fields = typename Adapter<T>::Fields;
            static constexpr auto value = []<size_t... Is>(std::index_sequence<Is...>) {
                return std::apply(
                  [](auto&&... args) { return glz::object(std::forward<decltype(args)>(args)...); },
                  std::tuple_cat(std::make_tuple(std::tuple_element_t<Is, Fields>::NAME,
                                                 glaze_field_value<std::tuple_element_t<Is, Fields>>())...));
            }(std::make_index_sequence<std::tuple_size_v<Fields>>{});
        };
#else
        template<typename T>
        consteval bool is_type_glaze_serializable()
        {
            return false;
        }
#endif

        template<typename T>
        concept GlazeSerializable = is_type_glaze_serializable<T>();

        template<FormatInfo Info, typename Ctx, typename T>
        auto handle_class_opts(Ctx& ctx, const T& t, const FmtOpts& fmt_opts)
          -> std::optional<typename Ctx::iterator>
        {
            if constexpr (GlazeSerializable<T>) {
#ifdef FMTU_ENABLE_JSON
                if (fmt_opts.json) {
                    auto json_str{ (fmt_opts.pretty ? glz::write<glz::opts{ .prettify = true }>(t)
                                                    : glz::write_json(t))
                                     .value_or("JSON Error") };
                    return std::format_to(ctx.out(), "{}", json_str);
                }
#endif
#ifdef FMTU_ENABLE_YAML
                if (fmt_opts.yaml) {
                    // TODO: uncomment when glaze is fixed
                    // auto yaml_str{ glz::write_yaml(t).value_or("YAML Error") };
                    auto yaml_str{ "" };
                    return std::format_to(ctx.out(), "{}", yaml_str);
                }
#endif
#ifdef FMTU_ENABLE_TOML
                if (fmt_opts.toml) {
                    auto toml_str{ glz::write_toml(t).value_or("TOML Error") };
                    return std::format_to(ctx.out(), "{}", toml_str);
                }
#endif
            }
            if (fmt_opts.pretty) {
                auto args{ fmtu::detail::make_flat_args_tuple(t) };
                static constexpr auto fmt{ fmtu::detail::class_pretty_format<Info>() };
                return std::apply(
                  [&ctx](const auto&... args) { return std::format_to(ctx.out(), fmt, args...); }, args);
            }

            return std::nullopt;
        }
    }
}

#ifdef FMTU_ENABLE_GLAZE
template<fmtu::detail::HasAdapter T>
struct glz::meta<T> : fmtu::detail::GlazeAdapter<T>
{
};
#endif

template<fmtu::detail::HasAdapter T>
struct std::formatter<T>
{
    using Info = fmtu::detail::AdapterInfo<T>;

    // clang-format off
    static constexpr fmtu::detail::FmtOpts ALLOWED_FMT_OPTS{
        .verbose = true,
        .pretty = true,
        .json = fmtu::IS_JSON_ENABLED,
        .yaml = fmtu::IS_YAML_ENABLED,
        .toml = fmtu::IS_TOML_ENABLED
    };
    // clang-format on

    fmtu::detail::FmtOpts fmt_opts{};

    template<typename Ctx>
    constexpr auto parse(Ctx& ctx) -> Ctx::iterator
    {
        auto it{ fmtu::detail::parse_fmt_opts<ALLOWED_FMT_OPTS>(ctx, fmt_opts) };
        if constexpr (fmtu::IS_GLAZE_ENABLED) {
            if (fmt_opts.has_glaze()) {
                static_assert(fmtu::detail::GlazeSerializable<T>,
                              "Formatting not possible: Type is not glaze serializable");
            }
        }
        return it;
    }

    template<typename Ctx>
    auto format(const T& t, Ctx& ctx) const -> Ctx::iterator
    {
        if (fmt_opts.has_opt()) {
            if (auto it{ fmtu::detail::handle_class_opts<Info>(ctx, t, fmt_opts) }; it.has_value()) {
                return it.value();
            }
        }

        auto args{ [&]<size_t... Is>(std::index_sequence<Is...>) {
            using Fields = typename fmtu::Adapter<std::remove_cvref_t<T>>::Fields;
            return fmtu::detail::make_args_tuple(
              fmtu::detail::check_arg(std::invoke(std::tuple_element_t<Is, Fields>::VALUE, t))...);
        }(std::make_index_sequence<Info::numMembers()>{}) };

        static constexpr auto fmt{ fmtu::detail::class_format<Info>() };
        return std::apply([&ctx](const auto&... args) { return std::format_to(ctx.out(), fmt, args...); },
                          args);
    }
};

template<fmtu::detail::Reflectable T>
struct std::formatter<T>
{
    static_assert(
      requires { T{}; },
      "Type T contains reference members or other non-value-initializable members, which are not "
      "supported "
      "for automatic formatting. Consider removing references or providing default initializers.");

    using Info = fmtu::detail::ReflectableInfo<T>;

    // clang-format off
    static constexpr fmtu::detail::FmtOpts ALLOWED_FMT_OPTS{ 
        .verbose = true,
        .pretty = true,
        .json = fmtu::IS_JSON_ENABLED,
        .yaml = fmtu::IS_YAML_ENABLED,
        .toml = fmtu::IS_TOML_ENABLED
    };
    // clang-format on

    fmtu::detail::FmtOpts fmt_opts{};

    template<typename Ctx>
    constexpr auto parse(Ctx& ctx) -> Ctx::iterator
    {
        auto it{ fmtu::detail::parse_fmt_opts<ALLOWED_FMT_OPTS>(ctx, fmt_opts) };
        if constexpr (fmtu::IS_GLAZE_ENABLED) {
            if (fmt_opts.has_glaze()) {
                static_assert(fmtu::detail::GlazeSerializable<T>,
                              "Formatting not possible: Type is not glaze serializable");
            }
        }
        return it;
    }

    template<typename Ctx>
    auto format(const T& t, Ctx& ctx) const -> Ctx::iterator
    {
        if (fmt_opts.has_opt()) {
            if (auto it{ fmtu::detail::handle_class_opts<Info>(ctx, t, fmt_opts) }; it.has_value()) {
                return it.value();
            }
        }

        auto args{ [&]<size_t... Is>(std::index_sequence<Is...>) {
            return fmtu::detail::make_args_tuple(fmtu::detail::check_arg(reflect::get<Is>(t))...);
        }(std::make_index_sequence<reflect::size<T>()>{}) };

        static constexpr auto fmt{ fmtu::detail::class_format<Info>() };
        return std::apply([&ctx](const auto&... args) { return std::format_to(ctx.out(), fmt, args...); },
                          args);
    }
};

template<fmtu::detail::ScopedEnum T>
struct std::formatter<T>
{
    static constexpr fmtu::detail::FmtOpts ALLOWED_FMT_OPTS{ .verbose = true };
    fmtu::detail::FmtOpts fmt_opts{};

    template<typename Ctx>
    constexpr auto parse(Ctx& ctx) -> Ctx::iterator
    {
        return fmtu::detail::parse_fmt_opts<ALLOWED_FMT_OPTS>(ctx, fmt_opts);
    }

    template<typename Ctx>
    auto format(T t, Ctx& ctx) const -> Ctx::iterator
    {
        if (fmt_opts.verbose) {
            return std::format_to(ctx.out(), "{}:{}", reflect::type_name(t), reflect::enum_name(t));
        }
        return std::format_to(ctx.out(), "{}", reflect::enum_name(t));
    }
};

template<std::formattable<char> T>
struct std::formatter<std::optional<T>> : std::formatter<T>
{
    using Base = std::formatter<T>;

    template<typename Ctx>
    auto format(const std::optional<T>& t, Ctx& ctx) const -> Ctx::iterator
    {
        if (!t) {
            return std::ranges::copy("[ null ]"sv, ctx.out()).out;
        }

        ctx.advance_to(std::ranges::copy("[ "sv, ctx.out()).out);
        ctx.advance_to(Base::format(*t, ctx));
        return std::ranges::copy(" ]"sv, ctx.out()).out;
    }
};

template<fmtu::detail::ValuePtr T>
struct std::formatter<T> : std::formatter<std::remove_pointer_t<std::remove_cvref_t<T>>>
{
    using Base = std::formatter<std::remove_pointer_t<std::remove_cvref_t<T>>>;

    template<typename Ctx>
    auto format(T t, Ctx& ctx) const -> Ctx::iterator
    {
        if (!t) {
            return std::format_to(ctx.out(), "[ ({}) -> {} ]", static_cast<const void*>(t), "null");
        }

        ctx.advance_to(std::format_to(ctx.out(), "[ ({}) -> ", static_cast<const void*>(t)));
        ctx.advance_to(Base::format(*t, ctx));
        return std::ranges::copy(" ]"sv, ctx.out()).out;
    }
};

template<fmtu::detail::SmartPtr T>
struct std::formatter<T> : std::formatter<typename T::element_type*>
{
    template<typename Ctx>
    auto format(const T& t, Ctx& ctx) const -> Ctx::iterator
    {
        return std::formatter<typename T::element_type*>::format(t.get(), ctx);
    }
};
