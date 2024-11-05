/**
 * Copyright 2023 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */


#ifndef FRONTEND_UTILS_COPILOT_H_
#define FRONTEND_UTILS_COPILOT_H_

#define GEAX_PP_COMMA() ,
#define GEAX_PP_EMPTY()
#define GEAX_PP_CONCAT(a, b) GEAX_PP_CONCAT_I(a, b)
#define GEAX_PP_CONCAT_I(a, b) a ## b
#define GEAX_PP_FIRST(...) GEAX_PP_FIRST_I(__VA_ARGS__)
#define GEAX_PP_FIRST_I(first, ...) first
#define GEAX_PP_TAIL(...) GEAX_PP_TAIL_I(__VA_ARGS__)
#define GEAX_PP_TAIL_I(_, ...) __VA_ARGS__

#define GEAX_PP_COUNT(...) GEAX_PP_COUNT_I(_, ##__VA_ARGS__, 16, 15, 14, 13, 12, \
                                         11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)  /* NOLINT */
#define GEAX_PP_COUNT_I(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
                       _11, _12, _13, _14, _15, _16, N, ...) N  /* NOLINT */

#define GEAX_PP_EXPR_ARG(expr, delimeter, ...) GEAX_PP_CONCAT(GEAX_PP_EXPR_ARG_I_, \
                                                            GEAX_PP_COUNT(__VA_ARGS__))(expr,  /* NOLINT */ \
                                                                                       delimeter,  \
                                                                                       __VA_ARGS__)
#define GEAX_PP_EXPR_ARG_I_0(expr, delimeter, ...) GEAX_PP_EMPTY()
#define GEAX_PP_EXPR_ARG_I_1(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_2(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                  delimeter  /* NOLINT */ \
                                                  GEAX_PP_EXPR_ARG_I_1(expr, \
                                                                      delimeter, \
                                                                      GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_3(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                  delimeter  /* NOLINT */ \
                                                  GEAX_PP_EXPR_ARG_I_2(expr, \
                                                                      delimeter, \
                                                                      GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_4(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                  delimeter  /* NOLINT */ \
                                                  GEAX_PP_EXPR_ARG_I_3(expr, \
                                                                      delimeter, \
                                                                      GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_5(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                  delimeter  /* NOLINT */ \
                                                  GEAX_PP_EXPR_ARG_I_4(expr, \
                                                                      delimeter, \
                                                                      GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_6(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                  delimeter  /* NOLINT */ \
                                                  GEAX_PP_EXPR_ARG_I_5(expr, \
                                                                      delimeter, \
                                                                      GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_7(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                  delimeter  /* NOLINT */ \
                                                  GEAX_PP_EXPR_ARG_I_6(expr, \
                                                                      delimeter, \
                                                                      GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_8(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                  delimeter  /* NOLINT */ \
                                                  GEAX_PP_EXPR_ARG_I_7(expr, \
                                                                      delimeter, \
                                                                      GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_9(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                  delimeter  /* NOLINT */ \
                                                  GEAX_PP_EXPR_ARG_I_8(expr, \
                                                                      delimeter, \
                                                                      GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_10(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                   delimeter  /* NOLINT */ \
                                                   GEAX_PP_EXPR_ARG_I_9(expr, \
                                                                       delimeter, \
                                                                       GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_11(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                   delimeter  /* NOLINT */ \
                                                   GEAX_PP_EXPR_ARG_I_10(expr, \
                                                                        delimeter, \
                                                                        GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_12(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                   delimeter  /* NOLINT */ \
                                                   GEAX_PP_EXPR_ARG_I_11(expr, \
                                                                        delimeter, \
                                                                        GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_13(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                   delimeter  /* NOLINT */ \
                                                   GEAX_PP_EXPR_ARG_I_12(expr, \
                                                                        delimeter, \
                                                                        GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_14(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                   delimeter  /* NOLINT */ \
                                                   GEAX_PP_EXPR_ARG_I_13(expr, \
                                                                        delimeter, \
                                                                        GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_15(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                   delimeter  /* NOLINT */ \
                                                   GEAX_PP_EXPR_ARG_I_14(expr, \
                                                                        delimeter, \
                                                                        GEAX_PP_TAIL(__VA_ARGS__))
#define GEAX_PP_EXPR_ARG_I_16(expr, delimeter, ...) expr(GEAX_PP_FIRST(__VA_ARGS__)) \
                                                   delimeter  /* NOLINT */ \
                                                   GEAX_PP_EXPR_ARG_I_15(expr, \
                                                                        delimeter, \
                                                                        GEAX_PP_TAIL(__VA_ARGS__))

#ifndef GEAX_LIKELY
#define GEAX_LIKELY(x)                   __builtin_expect(!!(x), 1)
#endif

#ifndef GEAX_UNLIKELY
#define GEAX_UNLIKELY(x)                 __builtin_expect(!!(x), 0)
#endif

#ifndef GEAX_UNLIKELY_NE
#define GEAX_UNLIKELY_NE(x, y)           GEAX_UNLIKELY((x) != (y))
#endif

#ifndef GEAX_IS_NULL
#define GEAX_IS_NULL(x)                  (GEAX_UNLIKELY(nullptr == (x)))
#endif

#ifndef GEAX_NOT_NULL
#define GEAX_NOT_NULL(x)                 (GEAX_LIKELY(nullptr != (x)))
#endif

#ifndef GEAX_ANY_NULL
#define GEAX_EQUAL_TO_NULL(x) GEAX_EQUAL_TO_NULL_I(x)
#define GEAX_EQUAL_TO_NULL_I(x) (x) == nullptr
#define GEAX_PP_NULL_DELIMETER GEAX_PP_NULL_DELIMETER_I
#define GEAX_PP_NULL_DELIMETER_I ||
#define GEAX_ANY_NULL_I(...) \
GEAX_PP_EXPR_ARG(GEAX_EQUAL_TO_NULL, GEAX_PP_NULL_DELIMETER, __VA_ARGS__)
#define GEAX_ANY_NULL(...) GEAX_UNLIKELY(GEAX_ANY_NULL_I(__VA_ARGS__))
#endif

#endif  // FRONTEND_UTILS_COPILOT_H_
