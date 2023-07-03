#pragma once
#include <array>
#include <string.h>
#include "Matcheroni.h"

#ifdef MATCHERONI_USE_NAMESPACE
using namespace matcheroni;
#endif

//------------------------------------------------------------------------------
// Sorted string table matcher thing.

template<const auto& table>
struct SST;

template<typename T, auto N, const std::array<T, N>& table>
struct SST<table> {

  constexpr inline static int top_bit(int x) {
    for (int b = 31; b >= 0; b--) {
      if (x & (1 << b)) return (1 << b);
    }
    return 0;
  }

  constexpr static bool contains(const char* s) {
    for (auto t : table) {
      if (strcmp(t, s) == 0) return true;
    }
    return false;
  }

  template<typename atom>
  static bool contains(atom& a) {
    int bit = top_bit(N);
    int index = 0;

    // I'm not actually sure if 8 is the best tradeoff but it seems OK
    if (N > 8) {
      // Binary search for large tables
      while(1) {
        auto new_index = index | bit;
        if (new_index < N) {
          auto lit = table[new_index];
          auto c = atom_cmp(a, lit);
          if (c == 0) return lit;
          if (c > 0) index = new_index;
        }
        if (bit == 0) return false;
        bit >>= 1;
      }
    }
    else {
      // Linear scan for small tables
      for (auto lit : table) {
        if (atom_cmp(a, lit) == 0) return true;
      }
    }

    return false;
  }
};

//------------------------------------------------------------------------------
// MUST BE SORTED CASE-SENSITIVE

// This is all the reserved words from GCC. Will probably have to remove some
// for C99 compatibility...

constexpr std::array c99_keywords = {
"_Accum",
"_Alignas",
"_Alignof",
"_Atomic",
"_Bool",
"_Complex",
"_Decimal128",
"_Decimal32",
"_Decimal64",
"_Float128",
"_Float128x",
"_Float16",
"_Float32",
"_Float32x",
"_Float64",
"_Float64x",
"_Fract",
"_Generic",
"_Imaginary",
"_Noreturn",
"_Sat",
"_Static_assert",
"_Thread_local",
"__FUNCTION__",
"__GIMPLE",
"__PHI",
"__PRETTY_FUNCTION__",
"__RTL",
"__alignof",
"__alignof__",
"__asm",
"__asm__",
"__attribute",
"__attribute__",
"__auto_type",
"__bases",
"__builtin_addressof",
"__builtin_assoc_barrier",
"__builtin_bit_cast",
"__builtin_call_with_static_chain",
"__builtin_choose_expr",
"__builtin_complex",
"__builtin_convertvector",
"__builtin_has_attribute",
"__builtin_launder",
"__builtin_offsetof",
"__builtin_shuffle",
"__builtin_shufflevector",
"__builtin_tgmath",
"__builtin_types_compatible_p",
"__builtin_va_arg",
"__complex",
"__complex__",
"__const",
"__const__",
"__constinit",
"__declspec",
"__decltype",
"__direct_bases",
"__extension__",
"__func__",
"__has_nothrow_assign",
"__has_nothrow_constructor",
"__has_nothrow_copy",
"__has_trivial_assign",
"__has_trivial_constructor",
"__has_trivial_copy",
"__has_trivial_destructor",
"__has_unique_object_representations",
"__has_virtual_destructor",
"__imag",
"__imag__",
"__inline",
"__inline__",
"__is_abstract",
"__is_aggregate",
"__is_assignable",
"__is_base_of",
"__is_class",
"__is_constructible",
"__is_convertible",
"__is_deducible ",
"__is_empty",
"__is_enum",
"__is_final",
"__is_layout_compatible",
"__is_literal_type",
"__is_nothrow_assignable",
"__is_nothrow_constructible",
"__is_nothrow_convertible",
"__is_pod",
"__is_pointer_interconvertible_base_of",
"__is_polymorphic",
"__is_same",
"__is_same_as",
"__is_standard_layout",
"__is_trivial",
"__is_trivially_assignable",
"__is_trivially_constructible",
"__is_trivially_copyable",
"__is_union",
"__label__",
"__null",
"__real",
"__real__",
"__reference_constructs_from_temporary",
"__reference_converts_from_temporary",
"__remove_cv",
"__remove_cvref",
"__remove_reference",
"__restrict",
"__restrict__",
"__signed",
"__signed__",
"__thread",
"__transaction_atomic",
"__transaction_cancel",
"__transaction_relaxed",
"__type_pack_element",
"__typeof",
"__typeof__",
"__underlying_type",
"__volatile",
"__volatile__",
"alignas",
"alignof",
"asm",
"assign",
"atomic",
"atomic_cancel",
"atomic_commit",
"atomic_noexcept",
"auto",
"bool",
"break",
"bycopy",
"byref",
"case",
"catch",
"char",
"char16_t",
"char32_t",
"char8_t",
"class",
"co_await",
"co_return",
"co_yield",
"compatibility_alias",
"concept",
"const",
"const_cast",
"consteval",
"constexpr",
"constinit",
"continue",
"copy",
"decltype",
"default",
"defs",
"delete",
"do",
"double",
"dynamic",
"dynamic_cast",
"else",
"encode",
"end",
"enum",
"explicit",
"export",
"extern",
"false",
"finally",
"float",
"for",
"friend",
"getter",
"goto",
"if",
"implementation",
"import ",
"in",
"inline",
"inout",
"int",
"interface",
"long",
"module ",
"mutable",
"namespace",
"new",
"noexcept",
"nonatomic",
"nonnull",
"null_resettable",
"null_unspecified",
"nullable",
"nullptr",
"offsetof",
"oneway",
"operator",
"optional",
"out",
"package",
"private",
"property",
"protected",
"protocol",
"public",
"readonly",
"readwrite",
"register",
"reinterpret_cast",
"required",
"requires",
"restrict",
"retain",
"return",
"selector",
"setter",
"short",
"signed",
"sizeof",
"static",
"static_assert",
"static_cast",
"struct",
"switch",
"synchronized",
"synthesize",
"template",
"this",
"thread_local",
"throw",
"true",
"try",
"typedef",
"typeid",
"typename",
"typeof",
"typeof_unqual",
"union",
"unsigned",
"using",
"virtual",
"void",
"volatile",
"wchar_t",
"while",
};

//------------------------------------------------------------------------------
// MUST BE SORTED CASE-SENSITIVE

constexpr std::array builtin_type_base = {
  //"FILE", // used in fprintf.c torture test
  "_Bool",
  "_Complex", // yes this is both a prefix and a type :P
  "_Decimal128",
  "_Decimal32",
  "_Decimal64",
  "__INT16_TYPE__",
  "__INT32_TYPE__",
  "__INT64_TYPE__",
  "__INT8_TYPE__",
  "__INTMAX_TYPE__",
  "__INTPTR_TYPE__",
  "__INT_FAST16_TYPE__",
  "__INT_FAST32_TYPE__",
  "__INT_FAST64_TYPE__",
  "__INT_FAST8_TYPE__",
  "__INT_LEAST16_TYPE__",
  "__INT_LEAST32_TYPE__",
  "__INT_LEAST64_TYPE__",
  "__INT_LEAST8_TYPE__",
  "__PTRDIFF_TYPE__",
  "__SIG_ATOMIC_TYPE__",
  "__SIZE_TYPE__",
  "__UINT16_TYPE__",
  "__UINT32_TYPE__",
  "__UINT64_TYPE__",
  "__UINT8_TYPE__",
  "__UINTMAX_TYPE__",
  "__UINTPTR_TYPE__",
  "__UINT_FAST16_TYPE__",
  "__UINT_FAST32_TYPE__",
  "__UINT_FAST64_TYPE__",
  "__UINT_FAST8_TYPE__",
  "__UINT_LEAST16_TYPE__",
  "__UINT_LEAST32_TYPE__",
  "__UINT_LEAST64_TYPE__",
  "__UINT_LEAST8_TYPE__",
  "__WCHAR_TYPE__",
  "__WINT_TYPE__",
  "__builtin_va_list",
  "__imag__",
  "__int128",
  "__label__",
  "__real__",
  "bool",
  "char",
  "double",
  "float",
  "int",
  "int16_t",
  "int32_t",
  "int64_t",
  "int8_t",
  "long",
  "ptrdiff_t",
  "short",
  "signed",
  "size_t", // used in fputs-lib.c torture test
  "uint16_t",
  "uint32_t",
  "uint64_t",
  "uint8_t",
  "unsigned",
  //"va_list", // technically part of the c library, but it shows up in stdarg test files
  "void",
  "wchar_t",
};

// MUST BE SORTED CASE-SENSITIVE
constexpr std::array builtin_type_prefix = {
  "_Complex",
  "__complex",
  "__complex__",
  "__imag",
  "__imag__",
  "__real",
  "__real__",
  "__signed__",
  "__unsigned__",
  "long",
  "short",
  "signed",
  "unsigned"
};

// MUST BE SORTED CASE-SENSITIVE
constexpr std::array builtin_type_suffix = {
  // Why, GCC, why?
  "_Complex",
  "__complex__",
};

// MUST BE SORTED CASE-SENSITIVE
constexpr std::array qualifiers = {
  "_Noreturn",
  "_Thread_local",
  "__const",
  "__extension__",
  "__inline",
  "__inline__",
  "__restrict",
  "__restrict__",
  "__stdcall",
  "__thread",
  "__volatile",
  "__volatile__",
  "auto",
  "const",
  "consteval",
  "constexpr",
  "constinit",
  "explicit",
  "extern",
  "inline",
  "mutable",
  "register",
  "restrict",
  "static",
  "thread_local",
  "virtual",
  "volatile",
};

// MUST BE SORTED CASE-SENSITIVE
constexpr std::array binary_operators = {
  "!=",
  "%",
  "%=",
  "&",
  "&&",
  "&=",
  "*",
  "*=",
  "+",
  "+=",
  "-",
  "-=",
  "->",
  "->*",
  ".",
  ".*",
  "/",
  "/=",
  "::",
  "<",
  "<<",
  "<<=",
  "<=",
  "<=>",
  "=",
  "==",
  ">",
  ">=",
  ">>",
  ">>=",
  "^",
  "^=",
  "|",
  "|=",
  "||",
};


constexpr int prefix_precedence(const char* op) {
  if (strcmp(op, "++")  == 0) return 3;
  if (strcmp(op, "--")  == 0) return 3;
  if (strcmp(op, "+")   == 0) return 3;
  if (strcmp(op, "-")   == 0) return 3;
  if (strcmp(op, "!")   == 0) return 3;
  if (strcmp(op, "~")   == 0) return 3;
  if (strcmp(op, "*")   == 0) return 3;
  if (strcmp(op, "&")   == 0) return 3;

  // 2 type(a) type{a}
  // 3 (type)a sizeof a sizeof(a) co_await a
  // 3 new a new a[] delete a delete a[]
  // 16 throw a co_yield a
  return 0;
}

constexpr int prefix_assoc(const char* op) {
  if (strcmp(op, "++")  == 0) return -1;
  if (strcmp(op, "--")  == 0) return -1;
  if (strcmp(op, "+")   == 0) return -1;
  if (strcmp(op, "-")   == 0) return -1;
  if (strcmp(op, "!")   == 0) return -1;
  if (strcmp(op, "~")   == 0) return -1;
  if (strcmp(op, "*")   == 0) return -1;
  if (strcmp(op, "&")   == 0) return -1;

  // 2 type(a) type{a}
  // 3 (type)a sizeof a sizeof(a) co_await a
  // 3 new a new a[] delete a delete a[]
  // 16 throw a co_yield a
  return 0;
}

constexpr int binary_precedence(const char* op) {
  if (strcmp(op, "::")  == 0) return 1;
  if (strcmp(op, ".")   == 0) return 2;
  if (strcmp(op, "->")  == 0) return 2;
  if (strcmp(op, ".*")  == 0) return 4;
  if (strcmp(op, "->*") == 0) return 4;
  if (strcmp(op, "*")   == 0) return 5;
  if (strcmp(op, "/")   == 0) return 5;
  if (strcmp(op, "%")   == 0) return 5;
  if (strcmp(op, "+")   == 0) return 6;
  if (strcmp(op, "-")   == 0) return 6;
  if (strcmp(op, "<<")  == 0) return 7;
  if (strcmp(op, ">>")  == 0) return 7;
  if (strcmp(op, "<=>") == 0) return 8;
  if (strcmp(op, "<")   == 0) return 9;
  if (strcmp(op, "<=")  == 0) return 9;
  if (strcmp(op, ">")   == 0) return 9;
  if (strcmp(op, ">=")  == 0) return 9;
  if (strcmp(op, "==")  == 0) return 10;
  if (strcmp(op, "!=")  == 0) return 10;
  if (strcmp(op, "&")   == 0) return 11;
  if (strcmp(op, "^")   == 0) return 12;
  if (strcmp(op, "|")   == 0) return 13;
  if (strcmp(op, "&&")  == 0) return 14;
  if (strcmp(op, "||")  == 0) return 15;
  if (strcmp(op, "?")   == 0) return 16;
  if (strcmp(op, ":")   == 0) return 16;
  if (strcmp(op, "=")   == 0) return 16;
  if (strcmp(op, "+=")  == 0) return 16;
  if (strcmp(op, "-=")  == 0) return 16;
  if (strcmp(op, "*=")  == 0) return 16;
  if (strcmp(op, "/=")  == 0) return 16;
  if (strcmp(op, "%=")  == 0) return 16;
  if (strcmp(op, "<<=") == 0) return 16;
  if (strcmp(op, ">>=") == 0) return 16;
  if (strcmp(op, "&=")  == 0) return 16;
  if (strcmp(op, "^=")  == 0) return 16;
  if (strcmp(op, "|=")  == 0) return 16;
  if (strcmp(op, ",")   == 0) return 17;
  return 0;
}

constexpr int binary_assoc(const char* op) {
  if (strcmp(op, "::")  == 0) return 1;
  if (strcmp(op, ".")   == 0) return 1;
  if (strcmp(op, "->")  == 0) return 1;
  if (strcmp(op, ".*")  == 0) return 1;
  if (strcmp(op, "->*") == 0) return 1;
  if (strcmp(op, "*")   == 0) return 1;
  if (strcmp(op, "/")   == 0) return 1;
  if (strcmp(op, "%")   == 0) return 1;
  if (strcmp(op, "+")   == 0) return 1;
  if (strcmp(op, "-")   == 0) return 1;
  if (strcmp(op, "<<")  == 0) return 1;
  if (strcmp(op, ">>")  == 0) return 1;
  if (strcmp(op, "<=>") == 0) return 1;
  if (strcmp(op, "<")   == 0) return 1;
  if (strcmp(op, "<=")  == 0) return 1;
  if (strcmp(op, ">")   == 0) return 1;
  if (strcmp(op, ">=")  == 0) return 1;
  if (strcmp(op, "==")  == 0) return 1;
  if (strcmp(op, "!=")  == 0) return 1;
  if (strcmp(op, "&")   == 0) return 1;
  if (strcmp(op, "^")   == 0) return 1;
  if (strcmp(op, "|")   == 0) return 1;
  if (strcmp(op, "&&")  == 0) return 1;
  if (strcmp(op, "||")  == 0) return 1;
  if (strcmp(op, "?")   == 0) return -1;
  if (strcmp(op, ":")   == 0) return -1;
  if (strcmp(op, "=")   == 0) return -1;
  if (strcmp(op, "+=")  == 0) return -1;
  if (strcmp(op, "-=")  == 0) return -1;
  if (strcmp(op, "*=")  == 0) return -1;
  if (strcmp(op, "/=")  == 0) return -1;
  if (strcmp(op, "%=")  == 0) return -1;
  if (strcmp(op, "<<=") == 0) return -1;
  if (strcmp(op, ">>=") == 0) return -1;
  if (strcmp(op, "&=")  == 0) return -1;
  if (strcmp(op, "^=")  == 0) return -1;
  if (strcmp(op, "|=")  == 0) return -1;
  if (strcmp(op, ",")   == 0) return 1;
  return 0;
}

constexpr int suffix_precedence(const char* op) {
  if (strcmp(op, "++")  == 0) return 2;
  if (strcmp(op, "--")  == 0) return 2;
  return 0;
}

constexpr int suffix_assoc(const char* op) {
  if (strcmp(op, "++")  == 0) return 1;
  if (strcmp(op, "--")  == 0) return 1;
  return 0;
}
