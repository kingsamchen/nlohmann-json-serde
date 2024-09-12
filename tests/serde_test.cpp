// Copyright (c) 2024 Kingsley Chen <kingsamchen at gmail dot com>
// This file is subject to the terms of license that can be found
// in the LICENSE file.

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "doctest/doctest.h"
#include "nlohmann/json.hpp"

#include "tests/stringification.h"

#include "nlohmann_serde/nlohmann_serde.h"

namespace serde = nlohmann::serde;

namespace {
struct message_basic {
    int seq_{0};
    std::int64_t ts_{0};
    std::vector<std::string> parts_;

    NLOHMANN_SERDE_DERIVE_TYPE(message_basic,
                               (seq_, "seq")
                               (ts_, "ts")
                               (parts_, "parts"))
};

TEST_CASE("Serialize and deserialize basic structs") {
    message_basic msg;
    msg.parts_.emplace_back("hello");
    msg.parts_.emplace_back("world");
    auto j_msg = nlohmann::json(msg);

    // struct -> json
    auto j = nlohmann::json::object();
    j["seq"] = msg.seq_;
    j["ts"] = msg.ts_;
    j["parts"] = msg.parts_;
    CHECK_EQ(j_msg, j);

    // json -> struct
    j_msg["seq"] = 1;
    j_msg["ts"] = 4096;
    j_msg["parts"].push_back("serde test");
    j_msg.get_to(msg);

    CHECK_EQ(msg.seq_, 1);
    CHECK_EQ(msg.ts_, 4096);
    auto parts = std::vector<std::string>{"hello", "world", "serde test"};
    CHECK_EQ(msg.parts_, parts);
}

struct digest {
    int length{0};
    std::string algorithm;

    NLOHMANN_SERDE_DERIVE_TYPE(digest,
                               (length, "length")
                               (algorithm, "algorithm"))
};

struct message_with_digest {
    int seq{0};
    std::int64_t ts{0};
    std::vector<std::string> parts;
    digest digest;

    NLOHMANN_SERDE_DERIVE_TYPE(message_with_digest,
                               (seq, "seq")
                               (ts, "ts")
                               (parts, "parts")
                               (digest, "digest"))
};

TEST_CASE("Support objects and its member objects") {
    auto j = R"({
        "seq": 42,
        "ts": 10240,
        "parts": ["hello", "world"],
        "digest": {
        "length": 1024,
        "algorithm": "SHA-256"
        }
    })"_json;

    message_with_digest msg;
    j.get_to(msg);
    CHECK_EQ(msg.seq, 42);
    CHECK_EQ(msg.ts, 10240);
    const std::vector<std::string> parts{"hello", "world"};
    CHECK_EQ(msg.parts, parts);
    CHECK_EQ(msg.digest.length, 1024);
    CHECK_EQ(msg.digest.algorithm, "SHA-256");
}

TEST_CASE("Automatically handle null values") {
    message_basic msg;
    msg.seq_ = 42;
    msg.ts_ = 1024;
    msg.parts_.emplace_back("this is a test text");

    SUBCASE("treat null json as empty object") {
        auto j_null = nlohmann::json();
        REQUIRE(j_null.is_null());
        j_null.get_to(msg);
        CHECK_EQ(msg.seq_, 42);
        CHECK_EQ(msg.ts_, 1024);
        CHECK_EQ(msg.parts_, std::vector<std::string>{"this is a test text"});
    }

    SUBCASE("automatically skip null fields") {
        // JSON is not null but one or more fields are null.
        auto j = R"({
            "seq": null,
            "ts": null,
            "parts": null
        })"_json;

        REQUIRE_FALSE(j.is_null());
        j.get_to(msg);
        CHECK_EQ(msg.seq_, 42);
        CHECK_EQ(msg.ts_, 1024);
        CHECK_EQ(msg.parts_, std::vector<std::string>{"this is a test text"});
    }
}

TEST_CASE("Ignore unknown JSON fields in deserialization") {
    auto j = R"({
        "seq": 42,
        "ts": 1234,
        "parts": ["hello", "world", "serde test"],
        "encrypted": false,
        "payload": {
        "length": "1024",
        "data": "this is a test text"
        }
    })"_json;

    message_basic msg;
    REQUIRE_EQ(msg.seq_, 0);
    REQUIRE_EQ(msg.ts_, 0);
    REQUIRE(msg.parts_.empty());

    // `encrypted` and `payload` should be ignored because `message_basic` doesn't know these
    // fields.
    j.get_to(msg);
    CHECK_EQ(msg.seq_, 42);
    CHECK_EQ(msg.ts_, 1234);
    CHECK_EQ(msg.parts_, std::vector<std::string>{"hello", "world", "serde test"});
}

TEST_CASE("JSON has fewer fields and struct's extra fields should remain unchanged") {
    message_basic msg;
    msg.seq_ = 42;
    msg.ts_ = 1024;
    msg.parts_.emplace_back("this is a test text");

    // JSON has only `parts` field.
    auto j = R"({
        "parts": ["hello", "world", "serde test"]
    })"_json;
    j.get_to(msg);

    CHECK_EQ(msg.seq_, 42);
    CHECK_EQ(msg.ts_, 1024);
    CHECK_EQ(msg.parts_, std::vector<std::string>{"hello", "world", "serde test"});
}

struct foobar_omit_empty {
    struct inner {
        std::string s;
        NLOHMANN_SERDE_DERIVE_TYPE(inner, (s, "s", serde::with(serde::omit_empty)))
    };

    int i{0};
    bool b{false};
    double d{0.};
    std::string str;
    std::vector<int> vec;
    std::unordered_map<int, std::string> map;
    inner inner;

    NLOHMANN_SERDE_DERIVE_TYPE(foobar_omit_empty,
                               (i, "int", serde::with(serde::omit_empty))
                               (b, "boolean", serde::with(serde::omit_empty))
                               (d, "double", serde::with(serde::omit_empty))
                               (str, "string", serde::with(serde::omit_empty))
                               (vec, "array", serde::with(serde::omit_empty))
                               (map, "objectFromMap", serde::with(serde::omit_empty))
                               (inner, "object", serde::with(serde::omit_empty)))
};

// The `omit_empty` option chooses to behave like Java's jackson omit empty annotation, which
// is kind of different from behavior of Golang & protobuf:
//  - ignored for number types, i.e. integer, boolean, floating points.
//  - omit fields whose value are empty string, empty array, or empty object

TEST_CASE("Omit non-trivial empty type when serialize a struct into JSON") {
    const foobar_omit_empty foobar;
    CHECK_EQ(foobar.i, 0);
    CHECK_EQ(foobar.b, false);
    CHECK_EQ(foobar.d, 0.);
    CHECK(foobar.str.empty());
    CHECK(foobar.vec.empty());
    CHECK(foobar.map.empty());
    CHECK(foobar.inner.s.empty());

    // By default nlohamnn json uses alphabet order.
    // Number primitives are preserved anyway, and since `Inner` has only one
    // string field and its also empty, so recursively, it will be omitted as
    // well.
    const std::string expected_json_str =
            R"({"boolean":false,"double":0.0,"int":0})";
    auto j = nlohmann::json(foobar);
    REQUIRE_EQ(j.dump(), expected_json_str);
}

struct foobar_stringify_int {
    std::int64_t n{0};

    NLOHMANN_SERDE_DERIVE_TYPE(foobar_stringify_int,
                               (n, "n", serde::with(serde::stringify_int64)))
};

TEST_CASE("Stringify int64 value in serialization and convert back in deserialization") {
    foobar_stringify_int foobar;
    REQUIRE_EQ(foobar.n, 0);

    const auto j = R"({"n": "10240"})"_json;
    j.get_to(foobar);
    CHECK_EQ(foobar.n, 10240);
}

// Use custom action

struct stringify_with_default_value_t : serde::action_base<stringify_with_default_value_t> {
    [[nodiscard]] static std::string serialize_impl(std::int64_t m_value) {
        return std::to_string(m_value);
    }

    [[nodiscard]] static std::int64_t deserialize_impl(const std::string& j_value) {
        try {
            return std::stoll(j_value);
        } catch (const std::exception& ex) {
            return 0;
        }
    }
};

inline const stringify_with_default_value_t stringify_with_default_value;

struct foobar_stringify_with_default {
    std::int64_t n{0};

    NLOHMANN_SERDE_DERIVE_TYPE(foobar_stringify_with_default,
                               (n, "n", serde::with(stringify_with_default_value)))
};

TEST_CASE("Use default value if error occurred in deserialization from string") {
    foobar_stringify_with_default foobar{42};
    REQUIRE_EQ(foobar.n, 42);

    const auto j = R"({"n": "abcd"})"_json;
    j.get_to(foobar);
    CHECK_EQ(foobar.n, 0);
}

} // namespace
