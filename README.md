# nlohmann-json-serde

A light-weight header-only util library that enables convenient struct serializaton & deserialization on top of [nlohmann/json](https://github.com/nlohmann/json).

This util library works well in a production-grade codebase and is proven to be effective.

## Feature Highlights

### User-specified key name for JSON field

```cpp
struct message_basic {
    int seq_{0};
    std::int64_t ts_{0};
    std::vector<std::string> parts_;

    NLOHMANN_SERDE_DERIVED(message_basic,
                           (seq_, "seq")
                           (ts_, "ts")
                           (parts_, "parts"))
};

message_basic msg;
msg.parts_.emplace_back("hello");
msg.parts_.emplace_back("world");
auto j_msg = nlohmann::json(msg);
auto j = nlohmann::json(foobar);
REQUIRE_EQ(j.dump(), R"({
    "parts": [
        "hello",
        "world"
    ],
    "seq": 0,
    "ts": 0
})");
```

No force mapping between a member name and its JSON field name.

### Omit empty fields of non-trivial types when serializing a JSON

NOTE: `omit_empty` has no effect on integers and floating points, because they won't be in _empty_ state.

The behavior intentionally keeps consistent with Java's conventional _**ignore empty values**_ annotation.

```cpp
struct foobar_omit_empty {
    struct inner {
        std::string s;
        NLOHMANN_SERDE_DERIVED(inner, (s, "s", serde::with(serde::omit_empty)))
    };

    int i{0};
    bool b{false};
    double d{0.};
    std::string str;
    std::vector<int> vec;
    std::unordered_map<int, std::string> map;
    inner inner;

    NLOHMANN_SERDE_DERIVED(foobar_omit_empty,
                           (i, "int", serde::with(serde::omit_empty))
                           (b, "boolean", serde::with(serde::omit_empty))
                           (d, "double", serde::with(serde::omit_empty))
                           (str, "string", serde::with(serde::omit_empty))
                           (vec, "array", serde::with(serde::omit_empty))
                           (map, "objectFromMap", serde::with(serde::omit_empty))
                           (inner, "object", serde::with(serde::omit_empty)))
};

const foobar_omit_empty foobar;
CHECK_EQ(foobar.i, 0);
CHECK_EQ(foobar.b, false);
CHECK_EQ(foobar.d, 0.);
CHECK(foobar.str.empty());
CHECK(foobar.vec.empty());
CHECK(foobar.map.empty());
CHECK(foobar.inner.s.empty());

auto j = nlohmann::json(foobar);
REQUIRE_EQ(j.dump(), R"({"boolean":false,"double":0.0,"int":0})");
```

### Allow user customized serde filter/action

```cpp
struct stringify_with_default_value_t : serde::action_base<stringify_with_default_value_t> {
    [[nodiscard]] static std::string serialize_impl(std::int64_t m_value) {
        return std::to_string(m_value);
    }

    [[nodiscard]] static std::int64_t deserialize_impl(const nlohmann::json& j_value) {
        try {
            return std::stoll(j_value.get_ref<const std::string&>());
        } catch (const std::exception&) {
            return 0;
        }
    }
};

inline const stringify_with_default_value_t stringify_with_default_value;

struct foobar_stringify_with_default {
    std::int64_t n{0};

    NLOHMANN_SERDE_DERIVED(foobar_stringify_with_default,
                           (n, "n", serde::with(stringify_with_default_value)))
};

const auto j = R"({"n": "42"})"_json;
j.get_to(foobar);
CHECK_EQ(foobar.n, 42);
```

### ...

More details of functionalities can be found at [tests](https://github.com/kingsamchen/nlohmann-json-serde/blob/master/tests)

## License

nlohmann-json-serde is licensed under the terms of the MIT license. see [LICENSE](https://github.com/kingsamchen/nlohmann-json-serde/blob/master/LICENSE)
