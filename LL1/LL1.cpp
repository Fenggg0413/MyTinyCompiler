#include <cstddef>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

char beginSymbol; // 文法开始符

// 初始化文法
void init(std::unordered_map<char, std::vector<std::string>> &grammar) {
  grammar.insert(std::make_pair('E', std::vector<std::string>{"E+T", "T"}));
  grammar.insert(std::make_pair('T', std::vector<std::string>{"T*F", "F"}));
  grammar.insert(std::make_pair('F', std::vector<std::string>{"(E)", "i"}));
  beginSymbol = 'E';
}

// 获取非终结符和终结符
std::pair<std::set<char>, std::set<char>>
getSymbols(const std::unordered_map<char, std::vector<std::string>> &grammar) {
  std::set<char> nonterminals;
  std::set<char> terminals;
  // 获取非终结符集合
  for (const auto &[non, exp] : grammar) {
    nonterminals.insert(non);
  }
  // 获取终结符集合
  for (const auto &[Non, exp] : grammar) {
    for (const auto &str : exp) {
      for (const auto &ch : str) {
        // 如果在非终结符集合中没有找到
        if (nonterminals.find(ch) == nonterminals.end()) {
          if (ch != ' ')
            terminals.insert(ch);
        }
      }
    }
  }
  return std::make_pair(nonterminals, terminals);
}

// 消除左递归
void eliLeftRecursion(
    std::unordered_map<char, std::vector<std::string>> &grammar) {
  std::unordered_map<char, std::vector<std::string>> newGrammar;
  for (auto &[non, exp] : grammar) {
    std::vector<std::string> alpha; // 直接左递归部分
    std::vector<std::string> beta;  // 其他部分
    for (const auto &prod : exp) {
      if (prod[0] == non) { // 直接左递归
        alpha.push_back(prod.substr(1));
      } else {
        beta.push_back(prod);
      }
    }
    if (!alpha.empty()) {     // 存在左递归
      char newNon = non + 26; // 新的非终结符
      for (auto &b : beta) {
        b += newNon;
      }
      std::vector<std::string> newExp;
      for (auto &a : alpha) {
        newExp.push_back(a + newNon);
      }
      newExp.push_back(" "); // 空串
      // 构建新的语法表
      newGrammar[non] = beta;
      newGrammar[newNon] = newExp;
    } else {
      newGrammar[non] = exp;
    }
  }
  grammar = newGrammar;
}

// 打印语法消息
void printInfo(
    const std::unordered_map<char, std::vector<std::string>> &grammar) {
  for (const auto &[non, exp] : grammar) {
    std::cout << non << " -> ";
    for (size_t i = 0; i < exp.size(); ++i) {
      std::cout << exp[i];
      if (i < exp.size() - 1) {
        std::cout << " | ";
      }
    }
    std::cout << std::endl;
  }
}

// 打印集合
void printFirstSetInfo(std::map<std::string, std::vector<char>> &set) {
  for (const auto &[symbol, vec] : set) {
    std::cout << "first(" << symbol << "): ";
    for (size_t i = 0; i < vec.size(); i++) {
      std::cout << vec[i];
      if (i < vec.size() - 1)
        std::cout << ", ";
    }
    std::cout << "\n";
  }
}

void printFollowSetInfo(std::map<char, std::vector<char>> &set) {
  for (const auto &[symbol, vec] : set) {
    std::cout << "follow(" << symbol << "): ";
    for (size_t i = 0; i < vec.size(); i++) {
      std::cout << vec[i];
      if (i < vec.size() - 1)
        std::cout << ", ";
    }
    std::cout << "\n";
  }
}

bool isNotRepeated(char ch, std::string str,
                   std::map<std::string, std::vector<char>> &firstSet) {
  return std::find(firstSet[str].begin(), firstSet[str].end(), ch) ==
         firstSet[str].end();
}

// 获取first集
std::map<std::string, std::vector<char>>
getFirstSet(const std::set<char> &nonterminals, const std::set<char> &terminals,
            const std::unordered_map<char, std::vector<std::string>> &grammar) {
  std::map<std::string, std::vector<char>> firstSet;
  // 求每个产生部右侧的first集
  for (auto [non, exps] : grammar) {
    for (size_t i = 0; i < exps.size(); i++) {
      auto exp = exps[i];
      std::vector<std::string> tempExps{exp};
      for (size_t j = 0; j < tempExps.size(); j++) {
        auto tmp = tempExps[j];
        // 如果第一个字符是空字符且长度不等于1则删除该字符
        if (tmp[0] == ' ' && tmp.size() != 1)
          tmp.erase(0, 1);
        if (tmp[0] == ' ' && tmp.size() == 1) {
          if (isNotRepeated(' ', exp, firstSet))
            firstSet[exp].push_back(' ');
        }
        // 如果表达式的首个符号是终结符
        else if (terminals.find(tmp[0]) != terminals.end()) {
          if (isNotRepeated(tmp[0], exp, firstSet))
            firstSet[exp].push_back(tmp[0]); // 将符号加入first集
        } else {
          auto alpha = tmp.substr(1); // 除去第一个符号的表达式
          auto nextExps = grammar.at(tmp[0]); // 由第一个符号产生的表达式
          for (auto prod : nextExps) {
            tempExps.push_back(prod + alpha);
          }
        }
      }
    }
  }
  // 求每个非终结符的first集
  for (auto non : nonterminals) {
    std::string nonstr{non};
    auto exps = grammar.at(non);
    for (auto exp : exps) {
      // 将每个产生式右部的first集加入对应的非终结符的first集
      for (auto ch : firstSet[exp]) {
        if (isNotRepeated(ch, nonstr, firstSet))
          firstSet[nonstr].push_back(ch);
      }
    }
  }
  return firstSet;
}

// 合并向量去重
void mergeAndDeduplicate(std::vector<char> &dest,
                         const std::vector<char> &src) {
  for (char c : src) {
    if (std::find(dest.begin(), dest.end(), c) == dest.end()) {
      dest.push_back(c);
    }
  }
}

bool isNotRepeated(char ch, char non,
                   std::map<char, std::vector<char>> &followSet) {
  return std::find(followSet[non].begin(), followSet[non].end(), ch) ==
         followSet[non].end();
}

// 获取follow集
std::map<char, std::vector<char>>
getFollowSet(const std::set<char> &nonterminals,
             const std::set<char> &terminals,
             const std::unordered_map<char, std::vector<std::string>> &grammar,
             const std::map<std::string, std::vector<char>> &firstSet) {
  std::map<char, std::vector<char>> followSet;
  for (auto non : nonterminals) {
    if (non == beginSymbol) // 文法开始符的follow集中有'#'
      followSet[non].push_back('#');
  }
  bool changed = true;
  while (changed) {
    changed = false;
    for (const auto &[left, rights] : grammar) {
      for (const auto &prod : rights) {
        for (size_t i = 0; i < prod.size(); ++i) {
          char B = prod[i];
          // 如果B是非终结符
          if (nonterminals.find(B) != nonterminals.end()) {
            size_t k = i + 1;
            // k指向B的后继字符
            while (k < prod.size()) {
              char C = prod[k];
              // 如果后继字符是终结符则直接加入
              if (terminals.find(C) != terminals.end()) {
                // 检查是否重复
                if (isNotRepeated(C, B, followSet)) {
                  followSet[B].push_back(C);
                  changed = true;
                }
                break;
              } else {
                // 如果后继字符是非终结符则加入产生部左式的first集
                std::string str{C};
                auto fs = firstSet.at(str);
                bool containsEpsilon =
                    std::find(fs.begin(), fs.end(), ' ') != fs.end();
                fs.erase(std::remove(fs.begin(), fs.end(), ' '), fs.end());
                size_t oldSize = followSet[B].size();
                mergeAndDeduplicate(followSet[B], fs);
                if (followSet[B].size() != oldSize) {
                  changed = true;
                }
                if (!containsEpsilon) {
                  break;
                }
                ++k;
              }
            }
            if (k == prod.size()) {
              size_t oldSize = followSet[B].size();
              mergeAndDeduplicate(followSet[B], followSet[left]);
              if (followSet[B].size() != oldSize) {
                changed = true;
              }
            }
          }
        }
      }
    }
  }
  return followSet;
}

// 构造LL(1)分析表
std::map<char, std::map<char, std::string>>
LL1(const std::map<std::string, std::vector<char>> &firstSet,
    const std::map<char, std::vector<char>> &followSet,
    const std::set<char> &terminals,
    const std::unordered_map<char, std::vector<std::string>> &grammar) {
  std::map<char, std::map<char, std::string>> list;
  // 对于每一个left ::= right(exp1 | exp2 | exp3 |...)
  for (const auto &[A, exps] : grammar) {
    for (const auto &alpha : exps) {
      for (const auto &a : firstSet.at(alpha)) {
        // 对first(alpha)中每一个终结符a,置list[A][a]=alpha
        if (terminals.find(a) != terminals.end()) {
          list[A][a] = alpha;
        } else if (a == ' ') {
          // 若first(alpha)中含有ε
          // 则对follow(A)中的每一个符号b,置list[A][b]=alpha
          for (const auto &b : followSet.at(A)) {
            list[A][b] = alpha;
          }
        }
      }
    }
    for (const auto &terminal : terminals) {
      auto line = list[A];
      // 对于list中的其他情况置为出错
      if (line.find(terminal) == line.end())
        list[A][terminal] = "NULL";
      if (line.find('#') == line.end())
        list[A]['#'] = "NULL";
    }
  }
  return list;
}

// 计算每一列的最大宽度
std::vector<size_t>
calculateColumnWidths(const std::map<char, std::map<char, std::string>> &list) {
  std::vector<size_t> widths;
  // 初始化列宽向量
  auto header = list.begin()->second;
  for (const auto &[key, val] : header) {
    widths.push_back(std::max(val.length(), 1ul));
  }
  // 更新每列的最大宽度
  for (const auto &[row, line] : list) {
    int i = 0;
    for (const auto &[key, val] : line) {
      widths[i] = std::max(widths[i], val.length());
      i++;
    }
  }
  return widths;
}

void printInfo(const std::map<char, std::map<char, std::string>> &list) {
  // 计算列宽
  std::vector<size_t> columnWidths = calculateColumnWidths(list);
  // 打印表头
  auto header = list.begin()->second;
  std::cout << std::setw(3) << " ";
  int col = 0;
  for (const auto &[key, val] : header) {
    std::cout << std::setw(columnWidths[col] + 2) << std::left << key;
    col++;
  }
  std::cout << std::endl;
  // 打印表头下的分隔线
  std::cout << std::setw(3) << " ";
  for (const auto &width : columnWidths) {
    std::cout << std::setw(width + 2) << std::left << std::setfill('-') << "-";
  }
  std::cout << std::setfill(' ') << std::endl;
  // 打印表内元素
  for (const auto &[row, line] : list) {
    std::cout << row << " |";
    int col = 0;
    for (const auto &[key, val] : line) {
      std::string displayVal = (val == "NULL") ? "" : (val == " " ? "ε" : val);
      std::cout << std::setw(columnWidths[col] + 2) << std::left << displayVal;
      col++;
    }
    std::cout << std::endl;
  }
}

bool isOperation(char ch) {
  return ch == '(' || ch == ')' || ch == '+' || ch == '*';
}

// 对给定符号串进行词法分析
std::string lexAnalyze(const std::string &str) {
  std::string newStr{};
  for (int i = 0; i < str.size(); i++) {
    switch (str[i]) {
    case '(':
      newStr += "(";
      break;
    case ')':
      newStr += ")";
      break;
    case '+':
      newStr += "+";
      break;
    case '*':
      newStr += "*";
      break;
    default:
      while (true) {
        // 一直读取字符直到字符为操作符
        if (!isOperation(str[i]))
          ++i;
        else {
          newStr += "i";
          break;
        }
      }
      --i;
    }
  }
  return newStr;
}

// 分析符号串是否为文法所定义
bool LL1Analyze(std::map<char, std::map<char, std::string>> &list,
                std::set<char> &nonterminals, std::string &str) {
  // 初始化分析栈
  std::vector<char> analyzeStack{};
  analyzeStack.push_back('#');
  analyzeStack.push_back(beginSymbol);
  // 初始化符号串栈
  std::vector<char> strStack{};
  strStack.push_back('#');
  for (int i = str.size() - 1; i >= 0; --i) {
    strStack.push_back(str[i]);
  }

  // 打印分析过程
  std::cout << "步骤\t分析栈\t输入串\t所用表达式\n";
  int id = 1;
  while (!analyzeStack.empty() && !strStack.empty()) {
    std::cout << id++ << "\t";
    for (int i = 0; i < analyzeStack.size(); i++) {
      std::cout << analyzeStack[i];
    }
    std::cout << "\t";
    for (int i = strStack.size() - 1; i >= 0; --i) {
      std::cout << strStack[i];
    }
    std::cout << "\t";
    
    char topAnalyze = analyzeStack.back();
    char topStr = strStack.back();
    if (topAnalyze == topStr) {
      // 终结符匹配
      analyzeStack.pop_back();
      strStack.pop_back();
      std::cout << "\n";
    } else if (nonterminals.find(topAnalyze) != nonterminals.end()) {
      // 非终结符处理
      if (list[topAnalyze][topStr] != "NULL") {
        analyzeStack.pop_back();
        std::string production = list[topAnalyze][topStr];
        std::cout << topAnalyze << "->" << production << "\n";
        if (production != " ") {
          for (int i = production.size() - 1; i >= 0; --i) {
            analyzeStack.push_back(production[i]);
          }
        }
      } else {
        // 无法找到对应的产生式
        std::cout << "\n";
        return false;
      }
    } else {
      // 终结符不匹配
      std::cout << "\n";
      return false;
    }
  }

  // 如果两个栈都空了，说明匹配成功
  return analyzeStack.empty() && strStack.empty();
}

int main() {
  std::unordered_map<char, std::vector<std::string>> grammar;
  init(grammar);
  std::cout << "初始文法: \n";
  printInfo(grammar);
  eliLeftRecursion(grammar); // 消除左递归
  std::cout << "\n消除左递归后的文法: \n";
  printInfo(grammar);
  auto [nonterminals, terminals] = getSymbols(grammar);
  std::cout << "非终结符: ";
  for (auto &non : nonterminals) {
    std::cout << non << " ";
  }
  std::cout << "\n终结符: ";
  for (auto &ter : terminals) {
    std::cout << ter << " ";
  }
  std::cout << std::endl;
  auto firstSet = getFirstSet(nonterminals, terminals, grammar);
  std::cout << "\nFirst集: \n";
  printFirstSetInfo(firstSet);
  auto followSet = getFollowSet(nonterminals, terminals, grammar, firstSet);
  std::cout << "\nFollow集: \n";
  printFollowSetInfo(followSet);

  auto list = LL1(firstSet, followSet, terminals, grammar);
  std::cout << "\n分析表:\n";
  printInfo(list);

  std::vector<std::string> vec{"abc+age*80", "(abc-80(*s5)"};
  std::cout << "\n分析过程:\n";
  for (auto &str : vec) {
    auto newStr = lexAnalyze(str);
    if (LL1Analyze(list, nonterminals, newStr))
      std::cout << str << "是文法所定义的句子\n";
    else
      std::cout << str << "不是文法所定义的句子\n";
    std::cout << std::endl;
	}
  return 0;
}