#include <fstream>
#include <iostream>
#include <set>
#include <vector>

// 词法单元的类型
enum TokenType {
  KEYWORD,    // 关键字
  IDENTIFIER, // 标识符
  OPERATOR,   // 操作符
  CONSTANT,   // 常数
  STRING,     // 字符串
  DELIMITER,  // 分隔符
};
// 按照枚举值存储类型名称的数组
std::string tokenArr[]{"keyword",  "identifier", "operator",
                       "constant", "string",     "delimiter"};

// 词法单元的表示
struct Token {
  TokenType type;
  std::string value;
};

// 关键字集合
std::set<std::string> keywords{
    "void",  "char", "int",    "float",    "double",
    "short", "long", "signed", "unsigned",
		"struct", "union", "enum", "typedef", "sizeof",
		"auto", "static", "register", "extern", "const", "volatile",
		"return","continue","break","goto",
		"if", "else", "switch", "case", "default",
		"for", "do", "while"
};

bool match(char expected, std::ifstream &str);
bool isDigit(char ch);
bool isAlpha(char ch);
bool isAlphaNumeric(char ch);
std::string getNum(std::ifstream &str);                  // 获取常数
std::string getString(std::ifstream &str);               // 获取字符串常量
std::string getIdentifier(std::ifstream &str);           // 获取标字符或关键字
std::vector<Token> analyze(const std::string &filename); // 词法分析

void printToken(const std::vector<Token> &tokens) {
  for (auto token : tokens)
    std::cout << "(" << tokenArr[token.type] << ", '" << token.value << "')\n";
}

void printCode(const std::vector<Token> &tokens) {
  for (auto token : tokens)
    std::cout << token.value;
  std::cout << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    auto tokens = analyze(argv[1]);
    printToken(tokens);
    printCode(tokens);
  } else {
    std::cerr << "error: no input files" << std::endl;
  }
  return 0;
}

bool match(char expected, std::ifstream &str) {
  if (str.eof())
    return false;
  if (str.peek() == expected) {
    str.get(); // 下一个字符匹配成功后跳过该字符
    return true;
  }
  return false;
}

std::string getString(std::ifstream &str) {
  std::string res{};
  while (str.peek() != '"' && !str.eof()) {
    res += str.get();
  }
  // 跳过未读取到的"符号
  str.get();
  return res;
}

std::string getNum(std::ifstream &str) {
  std::string num{};
  str.unget();
  while (isDigit(str.peek()))
    num += str.get();
  // 浮点数处理
  if (str.peek() == '.') {
    char dot = str.get();
    if (isDigit(str.peek())) {
      num += dot;
      while (isDigit(str.peek()))
        num += str.get();
    }
  }
  return num;
}

// 获取标识符或关键字
std::string getIdentifier(std::ifstream &str) {
  std::string identifier{};
  str.unget();
  while (isAlphaNumeric(str.peek())) {
    identifier += str.get();
  }
  return identifier;
}

bool isDigit(char ch) { return ch >= '0' && ch <= '9'; }
// 标识符正则表达式[a-zA-Z_][a-zA-Z_0-9]*
bool isAlpha(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}
bool isAlphaNumeric(char ch) { return isAlpha(ch) || isDigit(ch); }

std::vector<Token> analyze(const std::string &filename) {
  std::ifstream str(filename);
  char ch;
  std::vector<Token> tokens;
  while (str >> ch) {
    switch (ch) {
    case '{':
      tokens.push_back({DELIMITER, "{"});
      break;
    case '}':
      tokens.push_back({DELIMITER, "}"});
      break;
    case ',':
      tokens.push_back({DELIMITER, ","});
      break;
    case ';':
      tokens.push_back({DELIMITER, ";"});
      break;
    case '(':
      tokens.push_back({DELIMITER, "("});
      break;
    case ')':
      tokens.push_back({DELIMITER, ")"});
      break;
    case '+':
      tokens.push_back({OPERATOR, "+"});
      break;
    case '-':
      tokens.push_back({OPERATOR, "-"});
      break;
    case '*':
      tokens.push_back({OPERATOR, "*"});
      break;
    case '=':
      if (match('=', str))
        tokens.push_back({OPERATOR, "=="});
      else
        tokens.push_back({OPERATOR, "="});
      break;
    case '!':
      if (match('=', str))
        tokens.push_back({OPERATOR, "!="});
      else
        tokens.push_back({OPERATOR, "!"});
      break;
    case '>':
      if (match('=', str))
        tokens.push_back({OPERATOR, ">="});
      else
        tokens.push_back({OPERATOR, ">"});
      break;
    case '<':
      if (match('=', str))
        tokens.push_back({OPERATOR, "<="});
      else
        tokens.push_back({OPERATOR, "<"});
      break;
    case '/':
      if (match('/', str)) {
        while (str.peek() != '\n' && !str.eof()) // 跳过注释
          str.get();
      } else {
        tokens.push_back({OPERATOR, "/"});
      }
    // 跳过空白
    case ' ':
    case '\t':
    case '\r':
    case '\n':
      break;
    case '"':
      tokens.push_back({STRING, getString(str)});

    default:
      if (isDigit(ch)) { // 识别到数字
        tokens.push_back({CONSTANT, getNum(str)});
      } else if (isAlpha(ch)) { // 识别到字母
        std::string identifier = getIdentifier(str);
        if (keywords.find(identifier) != keywords.end())
          tokens.push_back({KEYWORD, identifier});
        else
          tokens.push_back({IDENTIFIER, identifier});
      }
      break;
    }
  }
  return std::move(tokens);
}