#include <iostream>
#include <stack>
#include <sstream>  // for std::stringstream
//#include <format>   // for std::format, but it's c++ std 20, so onlyu gcc 13.1 or above supports it !!

using namespace std;

class Calculator {
private:
  stack<int> operands_;
  stack<char> operators_;

  int GetPriority(char op) {
    switch (op) {
      case '+':
      case '-':
        return 1;
      case '*':
      case '/':
        return 2;
      default:
        return 0;
    }
  }
  
public:
  void Evaluate(const string& expression) {
    // use stringstream (a stream with source/destination as a string),
    // just like fstream is a stream using file as source/destination, cin/cout is a stream using 
    // console device as the source/destination.
    // Here we let "expression" act the source/destination of "ss". 
    stringstream ss;

    // "for (type x : container)" : 
    // a new looping syntax in c++, meaning to use "iterator of container" to get one element each time.
    for (char ch : expression) {  
      if (isdigit(ch)) {
        // operands_.push(ch - '0');
        // 將數字添加到stringstream中
        ss << ch;

        // (DEBUG-1) : Whether the followed "ss.clear()" is necessary ? YES !! it's MUST !!
        //
        //   the following lines are to debug the issue observed that the stringstream fails to 
        //   get "67890" when testing this program with input "12345+67890".
        //
        //   To reproduce the problem : comment out the line "ss.clear();" below and try with "12345+67890".
        //
        // refer to : https://cplusplus.com/reference/ios/ios/rdstate/
        // ios::eofbit	: End-of-File reached on input operation. can be detected by eof().
        // ios::badbit	: Read/writing error on i/o operation. can be checked by both fail() and bad().
        // ios::failbit: Logical error on i/o operation. can be checked by fail().
        //
        cout << "pushed " << ch << " to ss. The multi-digit integer str so far is : \"" 
          << ss.str() << "\"" << endl; // debug.
        if (!ss.good()){
          cout << "stringstream got eof, bad, or fail. (";
          
          // std::format() is c++ std 20 and is only supported in gcc 13.1 or above. don't use it.
          //cout << format("state=0x{%x}", ss.rdstate()) << ", ";

          // also notice the syntax CANNOT work :
          //   cout "a" << (test)? "B" : "C" << "D"...
          // the issue is ...................^^........ 
          // the "<<" was interpreted as "operator <<"" of ["C"], but it should be operator << of [cout ..."C"]
          // to make sure it is properly interpreted, we add the enclosing parentheses for it.
          //   cout "a" << ((test)? "B" : "C") << "D" ...
          if (ss.eof())
            cout << "eof=" << ((ss.rdstate() & ios::eofbit)? "Y":"N") << " ";  // will be "Y" (2)
          if (ss.bad())
            cout << "bad=" << (ss.rdstate() & ios::badbit)? "Y ":"N ";         // will be "N" (0)
          if (ss.fail())
            cout << "fail=" << (ss.rdstate() & ios::failbit)? "Y ":"N ";       // will be "Y" (4)
          cout << ")" << endl;
        }

      } else {
        if (ss.str().length() > 0) {
          // 如果不是運算數, 而且stringstream裡有東西的話, 
          // 將其透過stringstream 的 >> operator 的 implicit convertion, 轉換為整數(=stoi/atoi), 並推入堆疊
          // refer to : https://aprilyang.home.blog/2020/04/17/stringstream-to-read-int-from-a-string/
          //            https://hackmd.io/@Maxlight/rJwlvj8ad
          //            https://cplusplus.com/reference/sstream/stringstream/str/
          int tmp_operand = 0;
          ss >> tmp_operand;
          operands_.push(tmp_operand);
          ss.str(""); // make ss.str().length() = 0.
          // NOTICE "ss.clear()" is MUST here !!
          // after ss >> tmp_operand, ss will get its "eof" and "fail" state set, so any followed 
          // "ss << xxx" won't take effect any more.
          // ss.clear() is used to clear the "eof" and posisble "bad" or "fail" states.
          //
          // See (DEBUG-1) above also.
          ss.clear(); 
        }
        // 如果是運算子
        if (ch == '(') {
          operators_.push(ch);
        } else if (ch == ')') {
          // point-1 to start calculation(^) - when getting ')'
          // stop condition (s) - calculate until '(' met from stack top.
          // (continue: '<', skip: '.', value : new operand1/2)
          //
          // 1+2*(3+4*(5+6*7+1))*(8+9)
          //          s...47<<^         // Time2, got 48, pop one '('
          //     s<<<<48.......^        // Time3, got 195, pop one '('
          //                     s<<<^  // Time5, got 17, pop one '('
          //     
          while (operators_.top() != '(') {
            // pop 2 operands from stack top, and pop one operator each time.
            // push the result of "operand1 op operand2" back to stack as the new operand2.
            int operand2 = operands_.top();
            operands_.pop();
            int operand1 = operands_.top();
            operands_.pop();
            char op = operators_.top();
            operators_.pop();
            int result;
            switch (op) {
              case '+':
                result = operand1 + operand2;
                break;
              case '-':
                result = operand1 - operand2;
                break;
              case '*':
                result = operand1 * operand2;
                break;
              case '/':
                result = operand1 / operand2;
                break;
            }
            operands_.push(result);
          }
          operators_.pop();
        } else { // getting operator.
          // point-2 to start calculation (^)- when getting operator after operand2 with priority 
          // lower than or equivalent to the previous one (the one on stack top, should not be '(').
          // stop condition - calculate until 
          //    a) no more operator available, 
          //    b) '(' met from stack top.
          //    c) current operator has higher priority (* or /) than the one on stack top
          // 
          // 1+2*(3+4*(5+6*7+1)*(8+9)
          //          b<<<<<^           // Time1, got 47.
          //  c<<195...........^        // Time4, got 390.
          //
          while (!operators_.empty() && operators_.top() != '(' &&
                GetPriority(operators_.top()) >= GetPriority(ch)) {
            int operand2 = operands_.top();
            operands_.pop();
            int operand1 = operands_.top();
            operands_.pop();
            char op = operators_.top();
            operators_.pop();
            int result;
            switch (op) {
              case '+':
                result = operand1 + operand2;
                break;
              case '-':
                result = operand1 - operand2;
                break;
              case '*':
                result = operand1 * operand2;
                break;
              case '/':
                result = operand1 / operand2;
                break;
            }
            operands_.push(result);
          }
          operators_.push(ch);
        }
      }
    }

    cout << "final round : The multi-digit integer str so far is : \"" 
      << ss.str() << "\"" << endl; // debug.
    // point-3 to start calculation - last part, after all the "()" and "operations with high-then-low priority" 
    // done. All the left parts are of operations with low-then-high priority.
    // stop condition - no more operator available.
    // 
    // 1+2*(3+4*(5+6*7+1)*(8+9)
    // s<390.............<17...^   // Time7 : got the final result.
    // 

    // Gemini's proposal missed this par - the last operand might not be pushed to operands_ yet.
    if (ss.str().length() > 0) {
      // 如果不是運算數, 而且stringstream裡有東西的話, 
      // 將其透過stringstream 的 >> operator 的 implicit convertion, 轉換為整數(=stoi/atoi), 並推入堆疊
      // refer to : https://aprilyang.home.blog/2020/04/17/stringstream-to-read-int-from-a-string/
      //            https://hackmd.io/@Maxlight/rJwlvj8ad
      //            https://cplusplus.com/reference/sstream/stringstream/str/
      int tmp_operand = 0;
      ss >> tmp_operand;
      operands_.push(tmp_operand);
      ss.str(""); // make ss.str().length() = 0.
      // ss.clear() here is MUST !! to clear the eof and fail state after "ss >> xx;" and "ss.str("");"
      // See "(DEBUG-1)" above.
      ss.clear(); 
    }

    while (!operators_.empty()) {
      int operand2 = operands_.top();
      operands_.pop();
      int operand1 = operands_.top();
      operands_.pop();
      char op = operators_.top();
      operators_.pop();
      int result;
      switch (op) {
        case '+':
          result = operand1 + operand2;
          break;
        case '-':
          result = operand1 - operand2;
          break;
        case '*':
          result = operand1 * operand2;
          break;
        case '/':
          result = operand1 / operand2;
          break;
      }
      operands_.push(result);
    }

    cout << operands_.top() << endl;
  }

};

int main() {
  string expression;
  // use the following string for testing, and debug step-by-step to understand how this program runs : 
  // 
  // 1+2*(3+4*(5+6*7+1))*(8+9) = 6631
  // 12345+67890           = 80235
  // 12+34*(56+78*2)*(1+2) = 21636
  cin >> expression;

  Calculator calculator;
  calculator.Evaluate(expression);

  return 0;
}
