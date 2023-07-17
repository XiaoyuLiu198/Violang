struct LexRule {
  std::regex regex;
  Symbols symbol;
};

enum class Symbols {
    TS_NUM,         //number
    TS_STRING,      //string
    TS_SYMBOL,      //symbol
    TS_EOS,         //end of string
    TS_L_PAREN,     //"("
    TS_R_PAREN,     //")"
    TS_SPACE,

    NTS_S,
    NTS_E,
    NTS_E',
    NTS_T,
} 

struct Piece {
    Symbols symbol;
    int length; 
}

std::array<LexRule, 8> lexRules = {{
  {std::regex(R"(^\()"), Symbols::TS_L_PAREN},
  {std::regex(R"(^\))"), Symbols::TS_R_PAREN},
  {std::regex(R"(^\s+)"), Symbols::TS_SPACE},
  {std::regex(R"(^"[^\"]*")"), Symbols::TS_STRING},
  {std::regex(R"(^\d+)"), Symbols::TS_NUM},
  {std::regex(R"(^[\w\-+*=!<>/]+)"), Symbols::TS_SYMBOL}
}};

std::vector<std::string> tokens;

Piece lexer(const string c) {
   for (const auto& lex : lexRules) {
      std::smatch sm;

      if (std::regex_search(c, sm, rule.regex)) {
          yytext = sm[0]
          tokens.push_back(yytext.value());
          return Piece{ 
              .symbol = rule.symbol, 
              .length = yytext.length()
              };
        }
}

std::vector<int> calculateRules (const std::string str) {
    // expecting to return a list of rule numbers
    std::vector<int> result;

    std::vector<Symbols> ss;	// symbol stack

    int cursor = 0;

    // initialize the symbols stack
    ss.push_back(TS_EOS);	// terminal, $
    ss.push_back(NTS_S);		// non-terminal, S

    while (ss.size() > 0) {
        auto strSlice = str.substr(cursor);
        if (lexer(strSlice).symbol == ss[ss.size() - 1]) {
            std::cout << "Matched symbols: " << lexer(strSlice).symbol;
			cursor = cursor + lexer(strSlice).length;
			ss.pop_back();
        }
        else {
            std::cout << "Rule found";
            int rule;
            Symbols symbol = lexer(strSlice).symbol;
            auto last = ss[ss.size() - 1]
            if (symbol !== Symbols::TS_R_PAREN && last == Symbols::NTS_S) {
                rule = 0;
            } else if (last == Symbols::NTS_E) {
                rule = 1;
            } else if ((symbol == Symbols::TS_EOS | symbol == Symbols::TS_R_PAREN) && last == Symbols::NTS_E) {
                rule = 3;
            } else if (symbol !== Symbols::TS_EOS && symbol !== Symbols::TS_R_PAREN && last == Symbols::NTS_E) {
                rule = 2;
            } else if (symbol == Symbols::TS_L_PAREN && last == Symbols::NTS_T) {
                rule = 4;
            } else if (symbol == Symbols::TS_NUMBER && last == Symbols::NTS_T) {
                rule = 5;
            } else if (symbol == Symbols::TS_STRING && last == Symbols::NTS_T) {
                rule = 6;
            } else if (symbol == Symbols::TS_SYMBOL && last == Symbols::NTS_T) {
                rule = 7;
            }

            switch (rule)
            {
                case 0: {
                    ss.pop_back();
                    ss.push_back(Symbols::NTS_E);
                    break;
                }
                    
                case 1: {
                    ss.pop_back();
                    ss.push_back(Symbols::NTS_E');
                    break;
                }

                case 2: {
                    ss.pop_back();
                    ss.push_back(Symbols::NTS_E');
                    ss.push_back(Symbols::NTS_T);
                    break;
                }

                case 3: {
                    ss.pop_back();
                    ss.push_back(Symbols::TS_EOS);
                    break;
                }
                
                case 4: {
                    ss.pop_back();
                    ss.push_back(Symbols::TS_R_PAREN);
                    ss.push_back(Symbols::NTS_E);
                    ss.push_back(Symbols::TS_L_PAREN);
                    break;
                }

                case 5: {
                    ss.pop_back();
                    ss.push_back(Symbols::TS_NUMBER);
                    break;
                }

                case 6: {
                    ss.pop_back();
                    ss.push_back(Symbols::TS_STRING);
                    break;
                }

                case 7: {
                    ss.pop_back();
                    ss.push_back(Symbols::TS_SYMBOL);
                    break;
                }

                default:
                    std::cout << "Not in the parsing table"
                    break;
            }

            result.push_back(rule);
        }
    }
    return result
}

/**
 * Expression type.
 */
enum class ExpType {
  NUMBER,
  STRING,
  SYMBOL,
  LIST,
};

/**
 * Expression.
 */
struct Exp {
  ExpType type;

  int number;
  std::string string;
  std::vector<Exp> list;

  // Numbers:
  Exp(int number) : type(ExpType::NUMBER), number(number) {}

  // Strings, Symbols:
  Exp(std::string& strVal) {
    if (strVal[0] == '"') {
      type = ExpType::STRING;
      string = strVal.substr(1, strVal.size() - 2);
    } else {
      type = ExpType::SYMBOL;
      string = strVal;
    }
  }

  // Lists:
  Exp(std::vector<Exp> list) : type(ExpType::LIST), list(list) {}

};

Exp parse (const std::string str) {
    auto rules = calculateRules(str);
    std::vector<std::string> result = tree(str, rules);
    for (const auto item: result) {
        if (result == "(") {
            break;
        } else if () {
            break;
        }
    }
}

std::vector<std::string> tree (const std::vector<std::string>& str, const std::vector<int>& rules) {
    rule = rules.pop_back();
    std::vector<std::string> t;
    for (const std::vector<std::string> item: rule_dict[rule]) {
        if len(item > 1) {
            t.push_back(tree(str, rules));
        }
        else {
            t.push_back(str.pop_back());
        }
    }
    return t;
}
