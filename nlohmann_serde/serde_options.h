// Copyright (c) 2024 Kingsley Chen <kingsamchen at gmail dot com>
// This file is subject to the terms of license that can be found
// in the LICENSE file.

#pragma once

#include <string>
#include <string_view>
#include <type_traits>

#include "nlohmann/json.hpp"

namespace nlohmann::serde {

//
// filters
//

template<typename Impl>
struct filter_base {
    template<typename T>
    [[nodiscard]] bool on_serialize(const T& m_value) const {
        return static_cast<const Impl*>(this)->on_serialize_impl(m_value);
    }

    [[nodiscard]] bool on_deserialize(const nlohmann::json& j_value) const {
        return static_cast<const Impl*>(this)->on_deserialize_impl(j_value);
    }
};

class omit_empty_t : public filter_base<omit_empty_t> {
    friend filter_base;

    template<typename T>
    [[nodiscard]] static bool on_serialize_impl(const T& m_value) {
        using value_type = std::remove_cv_t<T>;
        if constexpr (std::is_integral_v<value_type>) {
            return false;
        } else if constexpr (std::is_same_v<value_type, std::string>) {
            return m_value.empty();
        } else {
            // TODO(Kingsley): optimize for common containers.
            auto field = nlohmann::json(m_value);
            return field.empty();
        }
    }

    [[nodiscard]] static bool on_deserialize_impl(const nlohmann::json& /* j_value */) {
        return false;
    }
};

inline constexpr auto omit_empty = omit_empty_t{};

//
// actions
//

template<typename Impl>
struct action_base {
    template<typename T>
    [[nodiscard]] auto serialize(const T& m_value) const {
        return static_cast<const Impl*>(this)->serialize_impl(m_value);
    }

    [[nodiscard]] auto deserialize(const nlohmann::json& j_value) const {
        return static_cast<const Impl*>(this)->deserialize_impl(j_value);
    }
};

class stringify_int64_t : public action_base<stringify_int64_t> {
    friend action_base;

    [[nodiscard]] static std::string serialize_impl(std::int64_t m_value) {
        return std::to_string(m_value);
    }

    [[nodiscard]] static std::int64_t deserialize_impl(const nlohmann::json& j_value) {
        return std::stoll(j_value.get_ref<const std::string&>());
    }
};

inline constexpr auto stringify_int64 = stringify_int64_t{};

namespace detail {

template<typename T>
inline constexpr bool is_filter_v = std::is_base_of_v<filter_base<T>, T>;

template<typename T>
inline constexpr bool is_action_v = std::is_base_of_v<action_base<T>, T>;

struct dummy_filter {};
struct dummy_action {};

} // namespace detail

//
// options
//

template<typename Filter, typename Action>
class options {
public:
    template<typename = std::enable_if_t<detail::is_filter_v<Filter> &&
                                         detail::is_action_v<Action>>>
    options(Filter filter, Action action)
        : filter_(filter),
          action_(action) {}

    Filter& filter() const {
        return filter_;
    }

    Action& action() const {
        return action_;
    }

    static constexpr bool has_filter = true;
    static constexpr bool has_action = true;

private:
    Filter filter_;
    Action action_;
};

template<typename Filter>
class options<Filter, detail::dummy_action> {
public:
    template<typename = std::enable_if_t<detail::is_filter_v<Filter>>>
    explicit options(Filter filter)
        : filter_(filter) {}

    const Filter& filter() const {
        return filter_;
    }

    static constexpr bool has_filter = true;
    static constexpr bool has_action = false;

private:
    Filter filter_;
};

template<typename Action>
class options<detail::dummy_filter, Action> {
public:
    template<typename = std::enable_if_t<detail::is_action_v<Action>>>
    explicit options(Action action)
        : action_(action) {}

    const Action& action() const {
        return action_;
    }

    static constexpr bool has_filter = false;
    static constexpr bool has_action = true;

private:
    Action action_;
};

template<typename Filter, typename Action>
auto with(Filter filter, Action action) {
    return options<Filter, Action>(filter, action);
}

template<typename Filter>
auto with(Filter filter, std::enable_if_t<detail::is_filter_v<Filter>, int*> = 0) { // NOLINT
    return options<Filter, detail::dummy_action>(filter);
}

template<typename Action>
auto with(Action action, std::enable_if_t<detail::is_action_v<Action>, int*> = 0) { // NOLINT
    return options<detail::dummy_filter, Action>(action);
}

//
// serializer & deserializer
//

template<typename T, typename Filter, typename Action>
void serialize_with_opts(nlohmann::json& j,
                         std::string_view name,
                         const T& member_value,
                         options<Filter, Action> opts) {
    if constexpr (decltype(opts)::has_filter) {
        if (bool skip = opts.filter().on_serialize(member_value); skip) {
            return;
        }
    }

    if constexpr (decltype(opts)::has_action) {
        j[name] = nlohmann::json(opts.action().serialize(member_value));
    } else {
        j[name] = nlohmann::json(member_value);
    }
}

template<typename T, typename Filter, typename Action>
void deserialize_with_opts(const nlohmann::json& j,
                           std::string_view name,
                           T& member_value,
                           options<Filter, Action> opts) {
    if (!j.contains(name) || j[name].is_null()) {
        return;
    }

    if constexpr (decltype(opts)::has_filter) {
        if (bool skip = opts.filter().on_deserialize(j[name]); skip) {
            return;
        }
    }

    if constexpr (decltype(opts)::has_action) {
        member_value = opts.action().deserialize(j[name]);
    } else {
        j[name].get_to(member_value);
    }
}

} // namespace nlohmann::serde
