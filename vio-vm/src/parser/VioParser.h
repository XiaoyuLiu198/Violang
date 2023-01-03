/**
 * LR parser for C++
 */
#ifndef __Syntax_LR_Parser_h
#define __Syntax_LR_Parser_h

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"

#include <assert.h>
#include <array>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>


#include <string>
#include <vector>

enum class ExpType {
    NUMBER,
    STRING,
    SYMBOL,
    LIST,
};

struct Exp {
    ExpType type;

    int number;
    std::string string;
    std::vector<Exp> list;

    Exp(int number) : type(ExpType::Number), number(number) {}

    Exp(std::string& strVal) {
        if (strVal[0] == '"'){
            type = ExpType::String;
            string = strVal.substr(1, strVal.size() - 2);
        } else {
            type = ExpType::Symbol;
            string = strVal;
        }
    }

    Exp(std::vector<Exp> list) : type(ExpType::List), list(list) {}
};

using Value = Exp;

namespace syntax {

/**
 * Tokenizer class.
 */
#ifndef __Syntax_Tokenizer_h
#define __Syntax_Tokenizer_h

class Tokenizer;

// ------------------------------------------------------------------
// TokenType.

enum class TokenType {
  __EMPTY = -1,
  NUMBER = 4,
  STRING = 5,
  SYMBOL = 6,
  TOKEN_TYPE_7 = 7,
  TOKEN_TYPE_8 = 8,
  __EOF = 9
};

// ------------------------------------------------------------------
// Token.

struct Token {
  TokenType type;
  std::string value;

  int startOffset;
  int endOffset;
  int startLine;
  int endLine;
  int startColumn;
  int endColumn;
};

using SharedToken = std::shared_ptr<Token>;

typedef TokenType (*LexRuleHandler)(const Tokenizer&, const std::string&);

// ------------------------------------------------------------------
// Lex rule: [regex, handler]

struct LexRule {
  std::regex regex;
  LexRuleHandler handler;
};

// ------------------------------------------------------------------
// Token.

enum TokenizerState {
  // clang-format off
  INITIAL
  // clang-format on
};

// ------------------------------------------------------------------
// Tokenizer.

class Tokenizer {
 public:
  /**
   * Initializes a parsing string.
   */
  void initString(const std::string& str) {
    str_ = str;

    // Initialize states.
    states_.clear();
    states_.push_back(TokenizerState::INITIAL);

    cursor_ = 0;
    currentLine_ = 1;
    currentColumn_ = 0;
    currentLineBeginOffset_ = 0;

    tokenStartOffset_ = 0;
    tokenEndOffset_ = 0;
    tokenStartLine_ = 0;
    tokenEndLine_ = 0;
    tokenStartColumn_ = 0;
    tokenEndColumn_ = 0;
  }

  /**
   * Whether there are still tokens in the stream.
   */
  inline bool hasMoreTokens() { return cursor_ <= str_.length(); }

  /**
   * Returns current tokenizing state.
   */
  TokenizerState getCurrentState() { return states_.back(); }

  /**
   * Enters a new state pushing it on the states stack.
   */
  void pushState(TokenizerState state) { states_.push_back(state); }

  /**
   * Alias for `push_state`.
   */
  void begin(TokenizerState state) { states_.push_back(state); }

  /**
   * Exits a current state popping it from the states stack.
   */
  TokenizerState popState() {
    auto state = states_.back();
    states_.pop_back();
    return state;
  }

  /**
   * Returns next token.
   */
  SharedToken getNextToken() {
    if (!hasMoreTokens()) {
      yytext = __EOF;
      return toToken(TokenType::__EOF);
    }

    auto strSlice = str_.substr(cursor_);

    auto lexRulesForState = lexRulesByStartConditions_.at(getCurrentState());

    for (const auto& ruleIndex : lexRulesForState) {
      auto rule = lexRules_[ruleIndex];
      std::smatch sm;

      if (std::regex_search(strSlice, sm, rule.regex)) {
        yytext = sm[0];

        captureLocations_(yytext);
        cursor_ += yytext.length();

        // Manual handling of EOF token (the end of string). Return it
        // as `EOF` symbol.
        if (yytext.length() == 0) {
          cursor_++;
        }

        auto tokenType = rule.handler(*this, yytext);

        if (tokenType == TokenType::__EMPTY) {
          return getNextToken();
        }

        return toToken(tokenType);
      }
    }

    if (isEOF()) {
      cursor_++;
      yytext = __EOF;
      return toToken(TokenType::__EOF);
    }

    throwUnexpectedToken(std::string(1, strSlice[0]), currentLine_,
                         currentColumn_);
  }

  /**
   * Whether the cursor is at the EOF.
   */
  inline bool isEOF() { return cursor_ == str_.length(); }

  SharedToken toToken(TokenType tokenType) {
    return std::shared_ptr<Token>(new Token{
        .type = tokenType,
        .value = yytext,
        .startOffset = tokenStartOffset_,
        .endOffset = tokenEndOffset_,
        .startLine = tokenStartLine_,
        .endLine = tokenEndLine_,
        .startColumn = tokenStartColumn_,
        .endColumn = tokenEndColumn_,
    });
  }

  /**
   * Throws default "Unexpected token" exception, showing the actual
   * line from the source, pointing with the ^ marker to the bad token.
   * In addition, shows `line:column` location.
   */
  [[noreturn]] void throwUnexpectedToken(const std::string& symbol, int line,
                                         int column) {
    std::stringstream ss{str_};
    std::string lineStr;
    int currentLine = 1;

    while (currentLine++ <= line) {
      std::getline(ss, lineStr, '\n');
    }

    auto pad = std::string(column, ' ');

    std::stringstream errMsg;

    errMsg << "Syntax Error:\n\n"
           << lineStr << "\n"
           << pad << "^\nUnexpected token \"" << symbol << "\" at " << line
           << ":" << column << "\n\n";

    std::cerr << errMsg.str();
    throw new std::runtime_error(errMsg.str().c_str());
  }

  /**
   * Matched text.
   */
  std::string yytext;

 private:
  /**
   * Captures token locations.
   */
  void captureLocations_(const std::string& matched) {
    auto len = matched.length();

    // Absolute offsets.
    tokenStartOffset_ = cursor_;

    // Line-based locations, start.
    tokenStartLine_ = currentLine_;
    tokenStartColumn_ = tokenStartOffset_ - currentLineBeginOffset_;

    // Extract `\n` in the matched token.
    std::stringstream ss{matched};
    std::string lineStr;
    std::getline(ss, lineStr, '\n');
    while (ss.tellg() > 0 && ss.tellg() <= len) {
      currentLine_++;
      currentLineBeginOffset_ = tokenStartOffset_ + ss.tellg();
      std::getline(ss, lineStr, '\n');
    }

    tokenEndOffset_ = cursor_ + len;

    // Line-based locations, end.
    tokenEndLine_ = currentLine_;
    tokenEndColumn_ = tokenEndOffset_ - currentLineBeginOffset_;
    currentColumn_ = tokenEndColumn_;
  }

  static constexpr size_t LEX_RULES_COUNT = 8;
  static std::array<LexRule, LEX_RULES_COUNT> lexRules_;
  static std::map<TokenizerState, std::vector<size_t>> lexRulesByStartConditions_;

  /**
   * Special EOF token.
   */
  static std::string __EOF;

  /**
   * Tokenizing string.
   */
  std::string str_;

  /**
   * Cursor for current symbol.
   */
  int cursor_;

  /**
   * States.
   */
  std::vector<TokenizerState> states_;

  /**
   * Line-based location tracking.
   */
  int currentLine_;
  int currentColumn_;
  int currentLineBeginOffset_;

  /**
   * Location data of a matched token.
   */
  int tokenStartOffset_;
  int tokenEndOffset_;
  int tokenStartLine_;
  int tokenEndLine_;
  int tokenStartColumn_;
  int tokenEndColumn_;
};

// ------------------------------------------------------------------
// Lexical rule handlers.

std::string Tokenizer::__EOF("$");

inline TokenType _lexRule1(const Tokenizer& tokenizer, const std::string& yytext) {
return TokenType::TOKEN_TYPE_7;
}

inline TokenType _lexRule2(const Tokenizer& tokenizer, const std::string& yytext) {
return TokenType::TOKEN_TYPE_8;
}

inline TokenType _lexRule3(const Tokenizer& tokenizer, const std::string& yytext) {
return TokenType::__EMPTY;
}

inline TokenType _lexRule4(const Tokenizer& tokenizer, const std::string& yytext) {
return TokenType::__EMPTY;
}

inline TokenType _lexRule5(const Tokenizer& tokenizer, const std::string& yytext) {
return TokenType::__EMPTY;
}

inline TokenType _lexRule6(const Tokenizer& tokenizer, const std::string& yytext) {
return TokenType::STRING;
}

inline TokenType _lexRule7(const Tokenizer& tokenizer, const std::string& yytext) {
return TokenType::NUMBER;
}

inline TokenType _lexRule8(const Tokenizer& tokenizer, const std::string& yytext) {
return TokenType::SYMBOL;
}

// ------------------------------------------------------------------
// Lexical rules.

std::array<LexRule, Tokenizer::LEX_RULES_COUNT> Tokenizer::lexRules_ = {{
  {std::regex(R"(^\()"), &_lexRule1},
  {std::regex(R"(^\))"), &_lexRule2},
  {std::regex(R"(^\/\/.*)"), &_lexRule3},
  {std::regex(R"(^\/\*[\s\S]*?\*\/)"), &_lexRule4},
  {std::regex(R"(^\s+)"), &_lexRule5},
  {std::regex(R"(^"[^\"]*")"), &_lexRule6},
  {std::regex(R"(^\d+)"), &_lexRule7},
  {std::regex(R"(^[\w\-+*=!<>/]+)"), &_lexRule8}
}};
std::map<TokenizerState, std::vector<size_t>> Tokenizer::lexRulesByStartConditions_ =  {{TokenizerState::INITIAL, {0, 1, 2, 3, 4, 5, 6, 7}}};

#endif

#define POP_V()              \
  parser.valuesStack.back(); \
  parser.valuesStack.pop_back()

#define POP_T()              \
  parser.tokensStack.back(); \
  parser.tokensStack.pop_back()

#define PUSH_VR() parser.valuesStack.push_back(__)
#define PUSH_TR() parser.tokensStack.push_back(__)

/**
 * Parsing table type.
 */
enum class TE {
  Accept,
  Shift,
  Reduce,
  Transit,
};

/**
 * Parsing table entry.
 */
struct TableEntry {
  TE type;
  int value;
};

class VioParser;

using yyparse = VioParser;

typedef void (*ProductionHandler)(yyparse&);

/**
 * Encoded production.
 *
 * opcode - encoded index
 * rhsLength - length of the RHS to pop.
 */
struct Production {
  int opcode;
  int rhsLength;
  ProductionHandler handler;
};

// Key: Encoded symbol (terminal or non-terminal) index
// Value: TableEntry
using Row = std::map<int, TableEntry>;

/**
 * Parser class.
 */
class VioParser {
 public:
  /**
   * Parsing values stack.
   */
  std::vector<Value> valuesStack;

  /**
   * Token values stack.
   */
  std::vector<std::string> tokensStack;

  /**
   * Parsing states stack.
   */
  std::vector<int> statesStack;

  /**
   * Tokenizer.
   */
  Tokenizer tokenizer;

  /**
   * Previous state to calculate the next one.
   */
  int previousState;

  /**
   * Parses a string.
   */
  Value parse(const std::string& str) {

    // Initialize the tokenizer and the string.
    tokenizer.initString(str);

    // Initialize the stacks.
    valuesStack.clear();
    tokensStack.clear();
    statesStack.clear();

    // Initial 0 state.
    statesStack.push_back(0);

    auto token = tokenizer.getNextToken();
    auto shiftedToken = token;

    // Main parsing loop.
    for (;;) {
      auto state = statesStack.back();
      auto column = (int)token->type;

      if (table_[state].count(column) == 0) {
        throwUnexpectedToken(token);
      }

      auto entry = table_[state].at(column);

      // Shift a token, go to state.
      if (entry.type == TE::Shift) {
        // Push token.
        tokensStack.push_back(token->value);

        // Push next state number: "s5" -> 5
        statesStack.push_back(entry.value);

        shiftedToken = token;
        token = tokenizer.getNextToken();
      }

      // Reduce by production.
      else if (entry.type == TE::Reduce) {
        auto productionNumber = entry.value;
        auto production = productions_[productionNumber];

        tokenizer.yytext = shiftedToken->value;

        auto rhsLength = production.rhsLength;
        while (rhsLength > 0) {
          statesStack.pop_back();
          rhsLength--;
        }

        // Call the handler.
        production.handler(*this);

        auto previousState = statesStack.back();

        auto symbolToReduceWith = production.opcode;
        auto nextStateEntry = table_[previousState].at(symbolToReduceWith);
        assert(nextStateEntry.type == TE::Transit);

        statesStack.push_back(nextStateEntry.value);
      }

      // Accept the string.
      else if (entry.type == TE::Accept) {
        // Pop state number.
        statesStack.pop_back();

        // Pop the parsed value.
        auto result = valuesStack.back(); valuesStack.pop_back();

        if (statesStack.size() != 1 || statesStack.back() != 0 ||
            tokenizer.hasMoreTokens()) {
          throwUnexpectedToken(token);
        }

        statesStack.pop_back();

        return result;
      }
    }
  }

 private:
  /**
   * Throws parser error on unexpected token.
   */
  [[noreturn]] void throwUnexpectedToken(SharedToken token) {
    if (token->type == TokenType::__EOF && !tokenizer.hasMoreTokens()) {
      std::string errMsg = "Unexpected end of input.\n";
      std::cerr << errMsg;
      throw std::runtime_error(errMsg.c_str());
    }
    tokenizer.throwUnexpectedToken(token->value, token->startLine,
                                   token->startColumn);
  }

  static constexpr size_t PRODUCTIONS_COUNT = 9;
  static std::array<Production, PRODUCTIONS_COUNT> productions_;

  static constexpr size_t ROWS_COUNT = 11;
  static std::array<Row, ROWS_COUNT> table_;
};

// ------------------------------------------------------------------

void _handler1(yyparse& parser) {
// Semantic action prologue.
auto _1 = POP_V();

auto __ = _1;

 // Semantic action epilogue.
PUSH_VR();

}

void _handler2(yyparse& parser) {
// Semantic action prologue.
auto _1 = POP_V();

auto __ = _1;

 // Semantic action epilogue.
PUSH_VR();

}

void _handler3(yyparse& parser) {
// Semantic action prologue.
auto _1 = POP_V();

auto __ = _1;

 // Semantic action epilogue.
PUSH_VR();

}

void _handler4(yyparse& parser) {
// Semantic action prologue.
auto _1 = POP_T();

auto __ = Exp(std::stoi(_1)) ;

 // Semantic action epilogue.
PUSH_VR();

}

void _handler5(yyparse& parser) {
// Semantic action prologue.
auto _1 = POP_T();

auto __ = Exp(_1) ;

 // Semantic action epilogue.
PUSH_VR();

}

void _handler6(yyparse& parser) {
// Semantic action prologue.
auto _1 = POP_T();

auto __ = Exp(_1) ;

 // Semantic action epilogue.
PUSH_VR();

}

void _handler7(yyparse& parser) {
// Semantic action prologue.
parser.tokensStack.pop_back();
auto _2 = POP_V();
parser.tokensStack.pop_back();

auto __ = _2 ;

 // Semantic action epilogue.
PUSH_VR();

}

void _handler8(yyparse& parser) {
// Semantic action prologue.


auto __ = Exp(std::vector<Exp>{}) ;

 // Semantic action epilogue.
PUSH_VR();

}

void _handler9(yyparse& parser) {
// Semantic action prologue.
auto _2 = POP_V();
auto _1 = POP_V();

_1.list.push_back(_2); auto __ = _1 ;

 // Semantic action epilogue.
PUSH_VR();

}
std::array<Production, yyparse::PRODUCTIONS_COUNT> yyparse::productions_ = {{{-1, 1, &_handler1},
{0, 1, &_handler2},
{0, 1, &_handler3},
{1, 1, &_handler4},
{1, 1, &_handler5},
{1, 1, &_handler6},
{2, 3, &_handler7},
{3, 0, &_handler8},
{3, 2, &_handler9}}};

// ------------------------------------------------------------------
// Parsing table.
std::array<Row, yyparse::ROWS_COUNT> yyparse::table_ = {
    Row {{0, {TE::Transit, 1}}, {1, {TE::Transit, 2}}, {2, {TE::Transit, 3}}, {4, {TE::Shift, 4}}, {5, {TE::Shift, 5}}, {6, {TE::Shift, 6}}, {7, {TE::Shift, 7}}},
    Row {{9, {TE::Accept, 0}}},
    Row {{4, {TE::Reduce, 1}}, {5, {TE::Reduce, 1}}, {6, {TE::Reduce, 1}}, {7, {TE::Reduce, 1}}, {8, {TE::Reduce, 1}}, {9, {TE::Reduce, 1}}},
    Row {{4, {TE::Reduce, 2}}, {5, {TE::Reduce, 2}}, {6, {TE::Reduce, 2}}, {7, {TE::Reduce, 2}}, {8, {TE::Reduce, 2}}, {9, {TE::Reduce, 2}}},
    Row {{4, {TE::Reduce, 3}}, {5, {TE::Reduce, 3}}, {6, {TE::Reduce, 3}}, {7, {TE::Reduce, 3}}, {8, {TE::Reduce, 3}}, {9, {TE::Reduce, 3}}},
    Row {{4, {TE::Reduce, 4}}, {5, {TE::Reduce, 4}}, {6, {TE::Reduce, 4}}, {7, {TE::Reduce, 4}}, {8, {TE::Reduce, 4}}, {9, {TE::Reduce, 4}}},
    Row {{4, {TE::Reduce, 5}}, {5, {TE::Reduce, 5}}, {6, {TE::Reduce, 5}}, {7, {TE::Reduce, 5}}, {8, {TE::Reduce, 5}}, {9, {TE::Reduce, 5}}},
    Row {{3, {TE::Transit, 8}}, {4, {TE::Reduce, 7}}, {5, {TE::Reduce, 7}}, {6, {TE::Reduce, 7}}, {7, {TE::Reduce, 7}}, {8, {TE::Reduce, 7}}},
    Row {{0, {TE::Transit, 10}}, {1, {TE::Transit, 2}}, {2, {TE::Transit, 3}}, {4, {TE::Shift, 4}}, {5, {TE::Shift, 5}}, {6, {TE::Shift, 6}}, {7, {TE::Shift, 7}}, {8, {TE::Shift, 9}}},
    Row {{4, {TE::Reduce, 6}}, {5, {TE::Reduce, 6}}, {6, {TE::Reduce, 6}}, {7, {TE::Reduce, 6}}, {8, {TE::Reduce, 6}}, {9, {TE::Reduce, 6}}},
    Row {{4, {TE::Reduce, 8}}, {5, {TE::Reduce, 8}}, {6, {TE::Reduce, 8}}, {7, {TE::Reduce, 8}}, {8, {TE::Reduce, 8}}}
};
} 

#endif