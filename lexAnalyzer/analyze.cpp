#include <cstdlib>
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

bool isMatch(char expected, std::ifstream &str);         // 是否匹配提供的字符
bool isDigit(char ch);                                   // 是否匹配数字
bool isAlpha(char ch);                                   // 是否匹配字母下划线
bool isAlphaNumeric(char ch);                            // 是否匹配字母下划线数字
std::string getNum(std::ifstream &str);                  // 获取常数
std::string getString(std::ifstream &str);               // 获取字符串常量
std::string getIdentifier(std::ifstream &str);           // 获取标字符或关键字
std::vector<Token> analyze(const std::string &filename); // 词法分析

// 打印扫描出的所有词法单元
void printToken(const std::vector<Token> &tokens) {
  for (auto token : tokens)
    std::cout << "(" << token.type+1 << ", '" << token.value << "')\n";
}

// 打印出处理掉注释、空白、换行后的代码
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

bool isMatch(char expected, std::ifstream &str) {
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
  // 字符串字面值用""包围
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
  // 读取所有数字字符
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
// 标识符正则表达式[a-zA-Z_][a-zA-Z_0-9]*
std::string getIdentifier(std::ifstream &str) {
  std::string identifier{};
  str.unget();
  // 识别[a-zA-Z_0-9]*过程
  while (isAlphaNumeric(str.peek())) {
    identifier += str.get();
  }
  return identifier;
}

bool isDigit(char ch) { return ch >= '0' && ch <= '9'; }
bool isAlpha(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}
bool isAlphaNumeric(char ch) { return isAlpha(ch) || isDigit(ch); }

// 词法分析
std::vector<Token> analyze(const std::string &filename) {
  std::ifstream str(filename);
  if (!str.is_open())
    exit(-1);
  char ch;
  std::vector<Token> tokens;
  // 逐个处理输入的字符直到文件尾
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
      // 对于操作符 = ! > <, 需要考虑后一个字符是否是 = 的情况
      // 因为这代表 == != >= <= 操作符
    case '=':
      if (isMatch('=', str))
        tokens.push_back({OPERATOR, "=="});
      else
        tokens.push_back({OPERATOR, "="});
      break;
    case '!':
      if (isMatch('=', str))
        tokens.push_back({OPERATOR, "!="});
      else
        tokens.push_back({OPERATOR, "!"});
      break;
    case '>':
      if (isMatch('=', str))
        tokens.push_back({OPERATOR, ">="});
      else
        tokens.push_back({OPERATOR, ">"});
      break;
    case '<':
      if (isMatch('=', str))
        tokens.push_back({OPERATOR, "<="});
      else
        tokens.push_back({OPERATOR, "<"});
      break;
      // 除法操作符'/'需要考虑是否是注释操作符'//'的情况
    case '/':
      if (isMatch('/', str)) {
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
      } else if (isAlpha(ch)) { // 识别到字母下划线
        std::string identifier = getIdentifier(str);
        if (keywords.find(identifier) != keywords.end())
          // 如果返回的字符串匹配到关键字
          tokens.push_back({KEYWORD, identifier});
        else
          // 未匹配到关键字则说明是标识符
          tokens.push_back({IDENTIFIER, identifier});
      }
      break;
    }
  }
  str.close();
  return std::move(tokens);
}