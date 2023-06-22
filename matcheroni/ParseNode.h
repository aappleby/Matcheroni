
#if 0
TOKEN RING
top ptr
alt ptr
bulldozing forward clears top
like packrat
never delete a match because it's still a match'
#endif

#pragma once
#include <typeinfo>
#include <set>
#include <string>
#include <vector>
#include <string.h>

#include "Lexemes.h"

struct ParseNode;
struct MatchMemo;
void set_color(uint32_t c);

//------------------------------------------------------------------------------
// Tokens associate lexemes with parse nodes.
// Tokens store bookkeeping data during parsing.

struct Token {
  Token(const Lexeme* lex) {
    this->lex = lex;
    this->top = nullptr;
    this->memo = nullptr;
  }

  bool is_valid() const {
    return lex->type != LEX_INVALID;
  }

  bool is_punct() const {
    return lex->is_punct();
  }

  bool is_punct(char p) const {
    return lex->is_punct(p);
  }

  bool is_lit(const char* lit) const {
    return lex->is_lit(lit);
  }

  bool is_identifier(const char* lit = nullptr) const {
    return lex->is_identifier(lit);
  }

  const Lexeme* lex;
  ParseNode* top;
  MatchMemo* memo;
};

//------------------------------------------------------------------------------

using token_matcher = matcher_function<Token>;

struct MatchMemo {
  const std::type_info* type;
  token_matcher matcher;
  ParseNode*    node;
  MatchMemo*    next;

  static constexpr int slab_size = 1024;
  inline static int inst_count = 0;
  inline static std::vector<MatchMemo*> inst_pool;

  static MatchMemo* take() {
    int slab_index = inst_count / slab_size;
    int inst_index = inst_count % slab_size;

    if (slab_index >= inst_pool.size()) {
      inst_pool.push_back(new MatchMemo[1024]);
    }

    auto memo = &inst_pool[slab_index][inst_index];
    memset(memo, 0, sizeof(MatchMemo));

    inst_count++;
    return memo;
  }

  static void clear_pool() {
    for (auto slab : inst_pool) delete [] slab;
    inst_pool.clear();
    inst_count = 0;
  }

private:
  MatchMemo() {}
};

//------------------------------------------------------------------------------

struct ParseNode {

  void init(token_matcher matcher, Token* tok_a, Token* tok_b) {
    this->matcher = matcher;
    this->tok_a = tok_a;
    this->tok_b = tok_b;
    instance_count++;
    constructor_count++;
  }

  virtual ~ParseNode() {
    //printf("Deleting parsenode\n");

    instance_count--;
    destructor_count++;

    auto cursor = head;
    while(cursor) {
      auto next = cursor->next;
      cursor->parent = nullptr;
      cursor->prev = nullptr;
      cursor->next = nullptr;
      cursor = next;
    }
  }

  //----------------------------------------

  ParseNode* top() {
    if (parent) return parent->top();
    return this;
  }

  //----------------------------------------

  int node_count() const {
    int accum = 1;
    for (auto c = head; c; c = c->next) {
      accum += c->node_count();
    }
    return accum;
  }

  //----------------------------------------

  template<typename P>
  bool isa() const {
    if (this == nullptr) return false;
    return typeid(*this) == typeid(P);
  }

  template<typename P>
  P* child() {
    if (this == nullptr) return nullptr;
    for (auto cursor = head; cursor; cursor = cursor->next) {
      if (cursor->isa<P>()) {
        return dynamic_cast<P*>(cursor);
      }
    }
    return nullptr;
  }

  template<typename P>
  const P* child() const {
    if (this == nullptr) return nullptr;
    for (auto cursor = head; cursor; cursor = cursor->next) {
      if (cursor->isa<P>()) {
        return dynamic_cast<const P*>(cursor);
      }
    }
    return nullptr;
  }

  template<typename P>
  P* search() {
    if (this == nullptr) return nullptr;
    if (this->isa<P>()) return this->as<P>();
    for (auto cursor = head; cursor; cursor = cursor->next) {
      if (auto c = cursor->search<P>()) {
        return c;
      }
    }
    return nullptr;
  }

  template<typename P>
  P* as() {
    return dynamic_cast<P*>(this);
  };

  template<typename P>
  const P* as() const {
    return dynamic_cast<const P*>(this);
  };

  void print_class_name() {
    const char* name = typeid(*this).name();
    int name_len = 0;
    if (sscanf(name, "%d", &name_len)) {
      while((*name >= '0') && (*name <= '9')) name++;
      for (int i = 0; i < name_len; i++) {
        putc(name[i], stdout);
      }
    }
    else {
      printf("%s", name);
    }
  }

  //----------------------------------------

  void detach_children() {
    auto c = head;
    while(c) {
      auto next = c->next;
      c->next = nullptr;
      c->prev = nullptr;
      c->parent = nullptr;
      c = next;
    }
    head = nullptr;
    tail = nullptr;
  }

  //----------------------------------------
  // Attach all the tops under this node to it.

  void attach_children() {

    auto cursor = tok_a;
    while (cursor < tok_b) {
      if (cursor->top) {
        auto child = cursor->top;
        if (child->parent) child->parent->detach_children();

        child->parent = this;
        if (tail) {
          tail->next = child;
          child->prev = tail;
          tail = child;
        }
        else {
          head = child;
          tail = child;
        }

        cursor = child->tok_b;
      }
      else {
        cursor++;
      }
    }
  }

  //----------------------------------------

  void reattach_children() {
    // Run our matcher to move all our children to the top
    auto end = matcher(tok_a, tok_b);
    assert(end == tok_b);
    attach_children();
  }

  //----------------------------------------

  void sanity() {
    // All our children should be sane.
    for (auto cursor = head; cursor; cursor = cursor->next) {
      cursor->sanity();
    }

    // Our prev/next pointers should be hooked up correctly
    assert(!next || next->prev == this);
    assert(!prev || prev->next == this);

    ParseNode* cursor = nullptr;

    // Check node chain
    for (cursor = head; cursor && cursor->next; cursor = cursor->next);
    assert(cursor == tail);

    for (cursor = tail; cursor && cursor->prev; cursor = cursor->prev);
    assert(cursor == head);
  }

  //----------------------------------------

  inline static int constructor_count = 0;
  inline static int destructor_count = 0;
  inline static int instance_count = 0;

  token_matcher matcher = nullptr;
  Token* tok_a = nullptr;
  Token* tok_b = nullptr;

  ParseNode* parent = nullptr;
  ParseNode* prev   = nullptr;
  ParseNode* next   = nullptr;
  ParseNode* head   = nullptr;
  ParseNode* tail   = nullptr;

  //----------------------------------------

  typedef std::set<std::string> IdentifierSet;

  inline static IdentifierSet builtin_types = {
    "void",
    "bool",
    "char", "short", "int", "long",
    "float", "double",
    "signed", "unsigned",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "int8_t", "int16_t", "int32_t", "int64_t",
    "_Bool",
    "_Complex", // yes this is both a prefix and a type :P
    "__real__",
    "__imag__",
    "__builtin_va_list",
    "wchar_t",

    // technically part of the c library, but it shows up in stdarg test files
    "va_list",

    // used in fprintf.c torture test
    "FILE",

    // used in fputs-lib.c torture test
    "size_t",

    // pr60003.c fails if this is included, pr56982.c fails if it isn't
    //"jmp_buf",

    // gcc stuff
    "__int128",
    "__SIZE_TYPE__",
    "__PTRDIFF_TYPE__",
    "__WCHAR_TYPE__",
    "__WINT_TYPE__",
    "__INTMAX_TYPE__",
    "__UINTMAX_TYPE__",
    "__SIG_ATOMIC_TYPE__",
    "__INT8_TYPE__",
    "__INT16_TYPE__",
    "__INT32_TYPE__",
    "__INT64_TYPE__",
    "__UINT8_TYPE__",
    "__UINT16_TYPE__",
    "__UINT32_TYPE__",
    "__UINT64_TYPE__",
    "__INT_LEAST8_TYPE__",
    "__INT_LEAST16_TYPE__",
    "__INT_LEAST32_TYPE__",
    "__INT_LEAST64_TYPE__",
    "__UINT_LEAST8_TYPE__",
    "__UINT_LEAST16_TYPE__",
    "__UINT_LEAST32_TYPE__",
    "__UINT_LEAST64_TYPE__",
    "__INT_FAST8_TYPE__",
    "__INT_FAST16_TYPE__",
    "__INT_FAST32_TYPE__",
    "__INT_FAST64_TYPE__",
    "__UINT_FAST8_TYPE__",
    "__UINT_FAST16_TYPE__",
    "__UINT_FAST32_TYPE__",
    "__UINT_FAST64_TYPE__",
    "__INTPTR_TYPE__",
    "__UINTPTR_TYPE__",
  };

  inline static IdentifierSet builtin_type_prefixes = {
    "signed", "unsigned", "short", "long", "_Complex",
    "__signed__", "__unsigned__",
    "__complex__", "__real__", "__imag__",
  };

  inline static IdentifierSet builtin_type_suffixes = {
    // Why, GCC, why?
    "_Complex", "__complex__",
  };

  inline static IdentifierSet class_types;
  inline static IdentifierSet struct_types;
  inline static IdentifierSet union_types;
  inline static IdentifierSet enum_types;
  inline static IdentifierSet typedef_types;

  static void reset_types() {
    class_types.clear();
    struct_types.clear();
    union_types.clear();
    enum_types.clear();
    typedef_types.clear();
  }
};

//------------------------------------------------------------------------------

inline int atom_cmp(Token& a, const LexemeType& b) {
  a.top = nullptr;
  return int(a.lex->type) - int(b);
}

inline int atom_cmp(Token& a, const char& b) {
  a.top = nullptr;
  int len_cmp = a.lex->len() - 1;
  if (len_cmp != 0) return len_cmp;
  return int(a.lex->span_a[0]) - int(b);
}

template<int N>
inline bool atom_cmp(Token& a, const StringParam<N>& b) {
  a.top = nullptr;
  int len_cmp = int(a.lex->len()) - int(b.len);
  if (len_cmp != 0) return len_cmp;

  for (auto i = 0; i < b.len; i++) {
    int cmp = int(a.lex->span_a[i]) - int(b.value[i]);
    if (cmp) return cmp;
  }

  return 0;
}

//------------------------------------------------------------------------------

//#define USE_MEMO
//#define MEMOIZE_UNMATCHES

template<typename NT>
struct NodeMaker : public ParseNode {

  static Token* match(Token* a, Token* b) {

    // See if there's a node on the token that we can reuse
    NT* node = nullptr;

#ifdef USE_MEMO
    for (auto memo = a->memo; memo; memo = memo->next) {
      /*if (typeid(*c) == typeid(NT))*/
      if (memo->matcher == (token_matcher)NT::pattern::match) {
#ifdef MEMOIZE_UNMATCHES
        if (memo->node) {
          // Memo has a node, reuse it
          node = dynamic_cast<NT*>(memo->node);
          assert(node);
        }
        else {
          //static int fail_count = 0;
          //printf("faaaail! %d\n", fail_count++);
          // Memo had no node, match already going to fail
          return nullptr;
        }
#else
        // Memo has a node, reuse it
        node = dynamic_cast<NT*>(memo->node);
        assert(node);
#endif
        break;
      }
    }

    if (node) {
      // Yep, there is - reconnect its children if needed
      //printf("yay reuse\n");
      if (!node->head) {
        //printf("reused node was disconnected\n");
        auto end = NT::pattern::match(a, b);
        assert(end);
        node->attach_children();
      }
      // And now our new node becomes token A's top.
      a->top = node;
      return node->tok_b;
    }
#endif

    // No node. Create a new node if the pattern matches, bail if it doesn't.
    auto end = NT::pattern::match(a, b);

    if (end) {
      node = new NT();
      node->init(NT::pattern::match, a, end);
      node->attach_children();
      // And now our new node becomes token A's top.
      a->top = node;
    }

    // Storing failed matches does not currently appear to be worth it.
#if 1
#ifdef MEMOIZE_UNMATCHES
    {
#else
    if (node) {
#endif
      auto memo = MatchMemo::take();
      memo->type    = &typeid(NT);
      memo->matcher = NT::pattern::match;
      memo->node    = node;
      memo->next    = a->memo;
      a->memo       = memo;
    }
#endif

    return end;
  }
};

//------------------------------------------------------------------------------

struct ParseNodeIterator {
  ParseNodeIterator(const ParseNode* cursor) : n(cursor) {}
  ParseNodeIterator& operator++() { n = n->next; return *this; }
  bool operator!=(const ParseNodeIterator& b) const { return n != b.n; }
  const ParseNode* operator*() const { return n; }
  const ParseNode* n;
};

inline ParseNodeIterator begin(const ParseNode* parent) {
  return ParseNodeIterator(parent->head);
}

inline ParseNodeIterator end(const ParseNode* parent) {
  return ParseNodeIterator(nullptr);
}

//------------------------------------------------------------------------------

inline Token* find_matching_delim(char ldelim, char rdelim, Token* a, Token* b) {
  if (*a->lex->span_a != ldelim) return nullptr;
  a++;

  while(a && a < b) {
    if (a->is_punct(rdelim)) return a;

    // Note that we _don't_ recurse through <> because they're not guaranteed
    // to be delimiters. Annoying aspect of C. :/

    if (a && a->is_punct('(')) a = find_matching_delim('(', ')', a, b);
    if (a && a->is_punct('{')) a = find_matching_delim('{', '}', a, b);
    if (a && a->is_punct('[')) a = find_matching_delim('[', ']', a, b);

    if (!a) return nullptr;
    a++;
  }

  return nullptr;
}

//------------------------------------------------------------------------------
// The Delimited<> modifier constrains a matcher to fit exactly between a pair
// of matching delimiters.
// For example, Delimited<'(', ')', NodeConstant> will match "(1)" but not
// "(1 + 2)".

template<char ldelim, char rdelim, typename P>
struct Delimited {
  static Token* match(Token* a, Token* b) {
    if (!a || !a->is_punct(ldelim)) return nullptr;
    auto new_b = find_matching_delim(ldelim, rdelim, a, b);
    if (!new_b || !new_b->is_punct(rdelim)) return nullptr;

    if (!new_b) return nullptr;
    if (auto end = P::match(a + 1, new_b)) {
      if (end != new_b) return nullptr;
      return new_b + 1;
    }
    else {
      return nullptr;
    }
  }
};

//------------------------------------------------------------------------------

struct NodeDispenser {

  NodeDispenser(ParseNode** children, size_t child_count) {
    this->children = children;
    this->child_count = child_count;
  }

  template<typename P>
  operator P*() {
    if (child_count == 0) return nullptr;
    if (children[0] == nullptr) return nullptr;
    if (children[0]->isa<P>()) {
      P* result = children[0]->as<P>();
      children++;
      child_count--;
      return result;
    }
    else {
      return nullptr;
    }
  }

  ParseNode** children;
  size_t child_count;
};

//------------------------------------------------------------------------------
