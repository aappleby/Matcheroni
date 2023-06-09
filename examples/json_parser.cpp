// SPDX-FileCopyrightText:  2023 Austin Appleby <aappleby@gmail.com>
// SPDX-License-Identifier: MIT License

#include "matcheroni/Matcheroni.hpp"
#include "matcheroni/Parseroni.hpp"

using namespace matcheroni;

//------------------------------------------------------------------------------

struct JsonParser {
  // clang-format off
  using ws        = Any<Atom<' ','\n','\r','\t'>>;
  using hex       = Oneof<Range<'0','9'>, Range<'A','F'>, Range<'a','f'>>;
  using escape    = Oneof<Charset<"\"\\/bfnrt">, Seq<Atom<'u'>, Rep<4, hex>>>;
  using keyword   = Oneof<Lit<"true">, Lit<"false">, Lit<"null">>;
  using character = Oneof< NotAtom<'"','\\'>, Seq<Atom<'\\'>, escape> >;
  using string    = Seq<Atom<'"'>, Any<character>, Atom<'"'>>;

  using sign      = Atom<'+','-'>;
  using digit     = Range<'0','9'>;
  using onenine   = Range<'1','9'>;
  using digits    = Some<digit>;
  using fraction  = Seq<Atom<'.'>, digits>;
  using exponent  = Seq<Atom<'e','E'>, Opt<sign>, digits>;
  using integer   = Seq< Opt<Atom<'-'>>, Oneof<Seq<onenine,digits>,digit> >;
  using number    = Seq<integer, Opt<fraction>, Opt<exponent>>;

  template<typename P>
  using comma_separated = Seq<P, Any<Seq<ws, Atom<','>, ws, P>>>;

  static cspan value(void* ctx, cspan s) {
    return Oneof<
      Capture<"array",   array,   NodeBase>,
      Capture<"number",  number,  NodeBase>,
      Capture<"object",  object,  NodeBase>,
      Capture<"string",  string,  NodeBase>,
      Capture<"keyword", keyword, NodeBase>
    >::match(ctx, s);
  }

  using pair =
  Seq<
    Capture<"key", string, NodeBase>,
    ws,
    Atom<':'>,
    ws,
    Capture<"value", Ref<value>, NodeBase>
  >;

  using object =
  Seq<
    Atom<'{'>, ws,
    Opt<comma_separated<
      Capture<"member", pair, NodeBase>
    >>,
    ws,
    Atom<'}'>
  >;

  using array =
  Seq<
    Atom<'['>,
    ws,
    Opt<comma_separated<Ref<value>>>, ws,
    Atom<']'>
  >;

  // clang-format on
  //----------------------------------------

  static cspan match(void* ctx, cspan s) {
    return Seq<ws, Ref<value>, ws>::match(ctx, s);
  }
};


cspan parse_json(void* ctx, cspan s) {
  return JsonParser::match(ctx, s);
}

//------------------------------------------------------------------------------
