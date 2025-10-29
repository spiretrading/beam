#ifndef BEAM_PREPROCESSOR_HPP
#define BEAM_PREPROCESSOR_HPP

#define BEAM_PP_STR_IMPL(x) #x
#define BEAM_PP_STR(x) BEAM_PP_STR_IMPL(x)

#define BEAM_PP_PRIMITIVE_CAT(a, b) a##b
#define BEAM_PP_CAT(a, b) BEAM_PP_PRIMITIVE_CAT(a, b)
#define BEAM_PP_EXPAND(x) x
#define BEAM_PP_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12,  \
  _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27,   \
  _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42,   \
  _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57,   \
  _58, _59, _60, _61, _62, _63, _64, N, ...) N

#define BEAM_PP_NARGS(...) BEAM_PP_NARGS_IMPL(__VA_ARGS__,                     \
  64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,              \
  48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,              \
  32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,              \
  16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1, 0)

#define BEAM_PP_FOR_EACH_0(M)
#define BEAM_PP_FOR_EACH_1(M, A1)                                              \
  M(A1)
#define BEAM_PP_FOR_EACH_2(M, A1, ...)                                         \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_1(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_3(M, A1, ...)                                         \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_2(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_4(M, A1, ...)                                         \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_3(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_5(M, A1, ...)                                         \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_4(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_6(M, A1, ...)                                         \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_5(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_7(M, A1, ...)                                         \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_6(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_8(M, A1, ...)                                         \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_7(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_9(M, A1, ...)                                         \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_8(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_10(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_9(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_11(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_10(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_12(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_11(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_13(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_12(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_14(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_13(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_15(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_14(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_16(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_15(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_17(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_16(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_18(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_17(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_19(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_18(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_20(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_19(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_21(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_20(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_22(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_21(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_23(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_22(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_24(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_23(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_25(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_24(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_26(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_25(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_27(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_26(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_28(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_27(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_29(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_28(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_30(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_29(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_31(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_30(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_32(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_31(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_33(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_32(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_34(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_33(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_35(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_34(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_36(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_35(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_37(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_36(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_38(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_37(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_39(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_38(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_40(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_39(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_41(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_40(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_42(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_41(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_43(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_42(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_44(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_43(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_45(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_44(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_46(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_45(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_47(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_46(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_48(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_47(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_49(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_48(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_50(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_49(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_51(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_50(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_52(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_51(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_53(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_52(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_54(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_53(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_55(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_54(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_56(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_55(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_57(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_56(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_58(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_57(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_59(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_58(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_60(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_59(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_61(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_60(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_62(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_61(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_63(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_62(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_64(M, A1, ...)                                        \
  BEAM_PP_FOR_EACH_1(M, A1), BEAM_PP_FOR_EACH_63(M, __VA_ARGS__)
#define BEAM_PP_FOR_EACH(M, ...)                                               \
  BEAM_PP_EXPAND(BEAM_PP_CAT(BEAM_PP_FOR_EACH_, BEAM_PP_NARGS(__VA_ARGS__))(M, __VA_ARGS__))

#define BEAM_PP_PAIR_FIRST_(a, b) a
#define BEAM_PP_PAIR_SECOND_(a, b) b
#define BEAM_PP_PAIR_FIRST(P) BEAM_PP_PAIR_FIRST_ P
#define BEAM_PP_PAIR_SECOND(P) BEAM_PP_PAIR_SECOND_ P

#define BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                    \
  M(Name, BEAM_PP_PAIR_FIRST(P), BEAM_PP_PAIR_SECOND(P))
#define BEAM_PP_FOR_EACH_PAIR_2(M, Name, P, ...)                               \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_3(M, Name, P, ...)                               \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_2(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_4(M, Name, P, ...)                               \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_3(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_5(M, Name, P, ...)                               \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_4(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_6(M, Name, P, ...)                               \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_5(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_7(M, Name, P, ...)                               \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_6(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_8(M, Name, P, ...)                               \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_7(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_9(M, Name, P, ...)                               \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_8(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_10(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_9(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_11(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_10(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_12(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_11(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_13(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_12(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_14(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_13(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_15(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_14(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_16(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_15(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_17(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_16(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_18(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_17(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_19(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_18(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_20(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_19(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_21(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_20(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_22(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_21(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_23(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_22(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_24(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_23(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_25(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_24(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_26(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_25(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_27(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_26(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_28(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_27(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_29(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_28(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_30(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_29(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_31(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_30(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_32(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_31(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_33(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_32(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_34(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_33(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_35(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_34(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_36(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_35(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_37(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_36(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_38(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_37(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_39(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_38(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_40(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_39(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_41(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_40(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_42(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_41(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_43(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_42(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_44(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_43(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_45(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_44(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_46(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_45(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_47(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_46(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_48(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_47(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_49(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_48(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_50(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_49(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_51(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_50(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_52(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_51(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_53(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_52(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_54(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_53(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_55(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_54(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_56(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_55(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_57(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_56(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_58(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_57(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_59(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_58(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_60(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_59(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_61(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_60(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_62(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_61(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_63(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_62(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIR_64(M, Name, P, ...)                              \
  BEAM_PP_FOR_EACH_PAIR_1(M, Name, P)                                          \
  BEAM_PP_FOR_EACH_PAIR_63(M, Name, __VA_ARGS__)
#define BEAM_PP_FOR_EACH_PAIRS(M, Name, ...)                                   \
  BEAM_PP_CAT(BEAM_PP_FOR_EACH_PAIR_, BEAM_PP_NARGS(__VA_ARGS__))              \
    (M, Name, __VA_ARGS__)

#endif
