// Copyright (c) 2024 Kingsley Chen <kingsamchen at gmail dot com>
// This file is subject to the terms of license that can be found
// in the LICENSE file.

#include "doctest/doctest.h"

#include "nlohmann_serde/serde_options.h"

namespace serde = nlohmann::serde;
namespace detail = nlohmann::serde::detail;

namespace {

struct foobar {};

TEST_CASE("type traits for identifying filter and action") {
    SUBCASE("type trait for identifying a filter") {
        static_assert(detail::is_filter_v<serde::omit_empty_t>);

        // Not a filter.
        static_assert(!detail::is_filter_v<serde::stringify_int64_t>);
        static_assert(!detail::is_filter_v<foobar>);
    }

    SUBCASE("type trait for identifying an action") {
        static_assert(detail::is_action_v<serde::stringify_int64_t>);

        // Not an action.
        static_assert(!detail::is_action_v<serde::omit_empty_t>);
        static_assert(!detail::is_action_v<foobar>);
    }
}

TEST_CASE("options requires true filter and/or action type") {
    SUBCASE("options with both filter and action") {
        using opt_t = serde::options<serde::omit_empty_t, serde::stringify_int64_t>;
        static_assert(opt_t::has_filter && opt_t::has_action);
    }

    SUBCASE("options with only filter") {
        using opt_t = serde::options<serde::omit_empty_t, detail::dummy_action>;
        static_assert(opt_t::has_filter && !opt_t::has_action);
    }

    SUBCASE("options with only action") {
        using opt_t = serde::options<detail::dummy_filter, serde::stringify_int64_t>;
        static_assert(!opt_t::has_filter && opt_t::has_action);
    }
}

TEST_CASE("with overloads for various use cases") {
    static_assert(std::is_same_v<
                  decltype(serde::with(serde::omit_empty_t{}, serde::stringify_int64_t{})),
                  serde::options<serde::omit_empty_t, serde::stringify_int64_t>>);

    static_assert(std::is_same_v<
                  decltype(serde::with(serde::omit_empty_t{})),
                  serde::options<serde::omit_empty_t, serde::detail::dummy_action>>);

    static_assert(std::is_same_v<
                  decltype(serde::with(serde::stringify_int64_t{})),
                  serde::options<serde::detail::dummy_filter, serde::stringify_int64_t>>);
}

} // namespace
