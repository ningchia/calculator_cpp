#include <iostream>
#include <stack>

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
    for (int i = 0; i < expression.length(); i++) {
      if (isdigit(expression[i])) {
        operands_.push(expression[i] - '0');
      } else if (expression[i] == '(') {
        operators_.push(expression[i]);
      } else if (expression[i] == ')') {
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
               GetPriority(operators_.top()) >= GetPriority(expression[i])) {
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
        operators_.push(expression[i]);
      }
    }

    // point-3 to start calculation - last part, after all the "()" and "operations with high-then-low priority" 
    // done. All the left parts are of operations with low-then-high priority.
    // stop condition - no more operator available.
    // 
    // 1+2*(3+4*(5+6*7+1)*(8+9)
    // s<390.............<17...^   // Time7 : got the final result.
    // 
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
  // 1+2*(3+4*(5+6*7+1))*(8+9)
  //
  cin >> expression;

  Calculator calculator;
  calculator.Evaluate(expression);

  return 0;
}
