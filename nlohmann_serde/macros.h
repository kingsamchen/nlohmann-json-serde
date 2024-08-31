// Copyright (c) 2024 Kingsley Chen <kingsamchen at gmail dot com>
// This file is subject to the terms of license that can be found
// in the LICENSE file.

#pragma once

#define NLOHMANN_F_SER_OPTS(member, name, opts) \
    {}

#define NLOHMANN_F_DE_OPTS(member, name, opts) \
    {}

#define NLOHMANN_F_SER(member, name) \
    { j[name] = m.member; }

#define NLOHMANN_F_DE(member, name)                   \
    {                                                 \
        if (j.contains(name) && !j[name].is_null()) { \
            j[name].get_to(m.member);                 \
        }                                             \
    }

#define NLOHMANN_SERDE_EXPAND_OP_3ARGS(fn, x, y, opts, next) \
    NLOHMANN_##fn##_OPTS(x, y, opts) NLOHMANN_SERDE_EXPAND_##fn##_##next

#define NLOHMANN_SERDE_EXPAND_OP_2ARGS(fn, x, y, next) \
    NLOHMANN_##fn(x, y) NLOHMANN_SERDE_EXPAND_##fn##_##next

#define NLOHMANN_SERDE_OP_3ARGS_A(fn, x, y, opts) NLOHMANN_SERDE_EXPAND_OP_3ARGS(fn, x, y, opts, B)
#define NLOHMANN_SERDE_OP_3ARGS_B(fn, x, y, opts) NLOHMANN_SERDE_EXPAND_OP_3ARGS(fn, x, y, opts, A)

#define NLOHMANN_SERDE_OP_2ARGS_A(fn, x, y) NLOHMANN_SERDE_EXPAND_OP_2ARGS(fn, x, y, B)
#define NLOHMANN_SERDE_OP_2ARGS_B(fn, x, y) NLOHMANN_SERDE_EXPAND_OP_2ARGS(fn, x, y, A)

#define NLOHMANN_SERDE_OP_1ARGS(fn, x)
#define NLOHMANN_SERDE_OP_0ARGS(fn)

#define NLOHMANN_SERDE_CHOOSE_OP(_0, _1, _2, _3, name, ...) name

//
// Serialization: struct to json.
//

#define NLOHMANN_SERDE_EXPAND_F_SER_A(...)              \
    NLOHMANN_SERDE_CHOOSE_OP(NLOHMANN_SERDE_OP_0ARGS,   \
                             ##__VA_ARGS__,             \
                             NLOHMANN_SERDE_OP_3ARGS_A, \
                             NLOHMANN_SERDE_OP_2ARGS_A, \
                             NLOHMANN_SERDE_OP_1ARGS,   \
                             NLOHMANN_SERDE_OP_0ARGS)   \
    (F_SER, ##__VA_ARGS__)

#define NLOHMANN_SERDE_EXPAND_F_SER_B(...)              \
    NLOHMANN_SERDE_CHOOSE_OP(NLOHMANN_SERDE_OP_0ARGS,   \
                             ##__VA_ARGS__,             \
                             NLOHMANN_SERDE_OP_3ARGS_B, \
                             NLOHMANN_SERDE_OP_2ARGS_B, \
                             NLOHMANN_SERDE_OP_1ARGS,   \
                             NLOHMANN_SERDE_OP_0ARGS)   \
    (F_SER, ##__VA_ARGS__)

//
// Deserialization: struct from json.
//

#define NLOHMANN_SERDE_EXPAND_F_DE_A(...)               \
    NLOHMANN_SERDE_CHOOSE_OP(NLOHMANN_SERDE_OP_0ARGS,   \
                             ##__VA_ARGS__,             \
                             NLOHMANN_SERDE_OP_3ARGS_A, \
                             NLOHMANN_SERDE_OP_2ARGS_A, \
                             NLOHMANN_SERDE_OP_1ARGS,   \
                             NLOHMANN_SERDE_OP_0ARGS)   \
    (F_DE, ##__VA_ARGS__)

#define NLOHMANN_SERDE_EXPAND_F_DE_B(...)               \
    NLOHMANN_SERDE_CHOOSE_OP(NLOHMANN_SERDE_OP_0ARGS,   \
                             ##__VA_ARGS__,             \
                             NLOHMANN_SERDE_OP_3ARGS_B, \
                             NLOHMANN_SERDE_OP_2ARGS_B, \
                             NLOHMANN_SERDE_OP_1ARGS,   \
                             NLOHMANN_SERDE_OP_0ARGS)   \
    (F_DE, ##__VA_ARGS__)

//
// Derive type.
//

#define NLOHMANN_SERDE_DERIVE_TYPE(type, ...)                 \
    friend void to_json(nlohmann::json& j, const type& m) {   \
        NLOHMANN_SERDE_EXPAND_F_SER_A __VA_ARGS__();          \
        if (j.is_null()) {                                    \
            j = nlohmann::json::object();                     \
        }                                                     \
    }                                                         \
                                                              \
    friend void from_json(const nlohmann::json& j, type& m) { \
        if (j.is_null() || (j.is_object() && j.empty())) {    \
            return;                                           \
        }                                                     \
        NLOHMANN_SERDE_EXPAND_F_DE_A __VA_ARGS__()            \
    }
