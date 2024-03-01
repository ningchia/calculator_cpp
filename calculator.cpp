#include <iostream>
#include <stack>
#include <sstream>  // for std::stringstream
//#include <format>   // for std::format, but it's c++ std 20. Only gcc 13.1 or above supports it !!
#include <iomanip>  // for std::fixed, std::setprecision, ... to control cout's output for int/float/double
#include <cmath>    // for floor, ceil, pow


// notice we use the following statement to tell compiler to try search symbols with leading "std::" as well.
// ie. stack<xx>  means   std::stack<xx>
//     cin        means   std::cin
//     cout       means   std::cout
//     stringstream means std::stringstream
//     hex        means   std::hex
//     ...
using namespace std;

// Whether to enable the part testing cout (std::cout) controls.
//
//#define Also_Run_Console_Out_Test

// Whether to enable the part testing float/double precision and how to round to best position.
//
//#define Also_Run_Precision_Test

// T can be double or float.
template <typename T>
class Calculator {
private:
  stack<T> operands_;
  stack<char> operators_;
  T result_ = 0;
  int result_precision_ = 0;

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

  T calc_round(T value, int decimal_digits) {
    return floor(value * pow(10, decimal_digits) + 0.5) / pow(10, decimal_digits);
  }

  void modify_result_to_best_precision(void){
    int idxDigit = 0;
    T result_after_round;
    T difference;
    
    while(1) {
      result_after_round = calc_round(result_, idxDigit);
      difference = result_ - result_after_round;
      // NOTICE : 
      // We cannot use the expression (x-y == 0) to tell whether 2 floating point values are equivalent.
      //   if (difference == 0) break;
      //
      // this is because arithmatic operations of floating point values will cause precision loss.
      // ex. x = 0.3;
      //     y = 0.3;
      //     return (x == y);       // true
      //     return ((x - y) == 0); // true
      //     x = 0.1 + 0.2;         // the "+" operation cause precision loss. (if it is not optimised by pre-processor)
      //     y = 0.3
      //     return (x == y);       // false
      //     return ((x - y) == 0); // false
      //
      // The correct way to tell the equivalence is by :
      //     if (std::abs(x - y) * 2 / (x + y)) < threshold)  // say, threshold = 1e-6.
      //        return equal.
      if (difference < 0) difference = 0 - difference;
      if ((difference * 2 / (result_ + result_after_round)) < 1e-6) break;
      idxDigit++;
    }
    result_ = result_after_round;
    result_precision_ = idxDigit;
  }

  void show_result(void){
    cout << fixed << setprecision(result_precision_);
    cout << result_;
    // set cout control back to default
    cout << setprecision(6);  
    cout.unsetf(ios::fixed);
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
      if (isdigit(ch) || (ch == '.')) {
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
          T tmp_operand = 0;
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
            T operand2 = operands_.top();
            operands_.pop();
            T operand1 = operands_.top();
            operands_.pop();
            char op = operators_.top();
            operators_.pop();
            T result;
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
            T operand2 = operands_.top();
            operands_.pop();
            T operand1 = operands_.top();
            operands_.pop();
            char op = operators_.top();
            operators_.pop();
            T result;
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
      T tmp_operand = 0;
      ss >> tmp_operand;
      operands_.push(tmp_operand);
      ss.str(""); // make ss.str().length() = 0.
      // ss.clear() here is MUST !! to clear the eof and fail state after "ss >> xx;" and "ss.str("");"
      // See "(DEBUG-1)" above.
      ss.clear(); 
    }

    while (!operators_.empty()) {
      T operand2 = operands_.top();
      operands_.pop();
      T operand1 = operands_.top();
      operands_.pop();
      char op = operators_.top();
      operators_.pop();
      T result;
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
    // print fraction part of result with proper decimal digits.
    // TBD : to determine the best number of decimal digits to display.
    result_ = operands_.top();
    cout << "result=";
    modify_result_to_best_precision();
    show_result();
    cout << endl;
  }

  // In most cases, the reason we have a friend function is to make it able to access private members
  // of a class, so we will have a friend function with a parameter of "reference to the targeted class",
  //
  // notice the function tp o be friend of a class should be defined before the class definition, 
  // or have a prototype there before the class definition.
  // ex.
  //    void ext_func(X &x, ...){   // definition before the class, or place a prototype declaration here.
  //      x.priv_data=...;
  //    }
  //    class X {
  //      private:
  //        priv_data;
  //      public:
  //        friend void ext_func(X &x, ...);
  //    }
  //
  // To have a template function as a friend, the containing class must have the same template parameter.
  // ex.
  //    template <typename T>             // have the same template param
  //    void ext_func(X<T> &x, ...){      // use class of template param in func parameter
  //      x.priv_data=...;
  //    }
  //    template <typename T>             // have the same template param
  //    class X {
  //      private:
  //        priv_data;
  //      public:
  //        friend void ext_func<T> (X<T> &x, ...);   // use template param in both func name and parameter
  //    }
  // 
  // The following doesn't work.
  //    class Calculator {   // the containing class doesn't have the same template parameters.
  //      ...
  //      friend template <class T>
  //      int round_to_best_precision(T orig, T &result);
  //    }
  //
  // Correct way is :
  //    template <class T>
  //    int round_to_best_precision(T orig, T &result){
  //      ...
  //    }
  //    template <typename T>
  //    class Calculator {
  //      ...
  //      friend int round_to_best_precision<T>(T orig, T &result);
  //    }
};

#ifdef Also_Run_Console_Out_Test
void cout_control_test(void);   // this test requires #include <iomanip>.
#endif

#ifdef Also_Run_Precision_Test
void precision_test(void);   // this test requires #include <cmath>.
#endif

int main() {
  cout << "Calculator Test. Please enter teh expression to evaluate :" << endl; 
  string expression;
  // use the following string for testing, and debug step-by-step to understand how this program runs : 
  // 
  // 1+2*(3+4*(5+6*7+1))*(8+9) = 6631
  // 12345+67890           = 80235
  // 12+34*(56+78*2)*(1+2) = 21636
  // 12.+13.45*(23.56+47.8*2) = 1614.702
  //
  cin >> expression;

  // Calculator<float> calculator;   // calculator with float type variables.
  Calculator<double> calculator;    // calculator with double type variables.
  calculator.Evaluate(expression);

#ifdef Also_Run_Console_Out_Test
  cout << endl << "--- Console out control test ---" << endl; 
  cout_control_test();
#endif
#ifdef Also_Run_Precision_Test
  cout << endl << "--- Float/Double presision and round to best position test ---" << endl; 
  precision_test();
#endif

  return 0;
}

#ifdef Also_Run_Console_Out_Test

void cout_control_test(void)
{
  // How to declare float/double number in program, and control precision when cout them.
  //   ref link : https://www.programiz.com/cpp-programming/float-double
  //
  // notice for float (32bit) 23 bits are used for the mantissa (about 7 decimal digits).
  // while for double (64bit) 52 bits are used for the mantissa (about 16 decimal digits)
  double a = 39.12348239;   // var with dicimal points but without trailing "f" will be interpreted as of "double" type.
  float  b = 39.12348239f;  // var of float type should be assigned by a number with trailing "f".

  // Modifiers only affect fpt numbers : "std::setprecision()", "std::fixed", "std::scientific".
  // Have persistent effect : ALL. 
  //
  // ----- Console output of the following code ----
  //  Test 1 : c++ Modifiers affacting fpt numbers 3.912348239
  //   <Default             > a(double)=39.1235, b(float)=39.1235
  //   <fixed, precision=8  > a(double)=39.12348239, b(float)=39.12348175
  //   <Retest w/o modifiers> a(double)=39.12348239, b(float)=39.12348175
  //   <scientific, precision=2> a(double)=3.91e+01, b(float)=3.91e+01
  //   <Retest w/o modifiers   > a(double)=3.91e+01, b(float)=3.91e+01
  //   <reset back to default  > a(double)=39.1235, b(float)=39.1235
  //
  cout << " Test 1 : c++ Modifiers affacting fpt numbers 3.912348239" << endl;
  cout << "  <Default             > ";
  cout << "a(double)=" << a << ", b(float)=" << b << endl;
  cout << "  <fixed, precision=8  > " << fixed << setprecision(8);
  cout << "a(double)=" << a << ", b(float)=" << b << endl;
  cout << "  <Retest w/o modifiers> ";
  cout << "a(double)=" << a << ", b(float)=" << b << endl;
  cout << "  <scientific, precision=2> "  << scientific << setprecision(2);
  cout << "a(double)=" << a << ", b(float)=" << b << endl;
  cout << "  <Retest w/o modifiers   > ";
  cout << "a(double)=" << a << ", b(float)=" << b << endl;
  
  // to reset back to default, Gemini mentioned "ios::defaultfmt", "ios:generic", but they are not 
  // defined in fact. let's use cout.unsetf() and set presision back to default (6) instead.
  cout << "  <reset back to default  > ";
  cout.unsetf(ios::fixed | ios::scientific);  
  cout << setprecision(6);  // default is 6.
  cout << "a(double)=" << a << ", b(float)=" << b << endl;

  // notice "std::setprecision", "std::fixed", and "std::scientific" only affect floating-point numbers.
  //
  // ----- Console output of the following code ----
  //  Test 2 : c++ Output integer c -128
  //   <No modifiers. No effect on integers.> c(int)=-128, c(uint)=4294967168
  //
  cout << " Test 2 : c++ Output integer c -128" << endl;
  int c = -128;
  cout << "  <No modifiers. No effect on integers.> c(int)=" << c << ", c(uint)=" << (unsigned int)c << endl;

  // Modifiers only affect integers : "std::hex","std::dec","std::oct","std::showbase","std::right","std::setw()".
  // Have persistent effect : "std::hex","std::dec","std::oct","std::showbase","std::right" 
  // Only effective for once : "std::setw()"
  //
  // ----- Console output of the following code ----
  //  Test 3 : c++ Modifiers affecting integers -128
  //   <hex, showbase, width=8, right-aligned> c(int)='0xffffff80'
  //   <Retest w/i width=10                  > c(int)='0xffffff80'
  //   <Retest w/o modifiers                 > c(int)='0xffffff80'
  //   <Retest on fpt number. No effect.     > a(double)='39.1235'
  //
  cout << " Test 3 : c++ Modifiers affecting integers -128" << endl;
  cout << "  <hex, showbase, width=8, right-aligned> c(int)='" << hex << showbase << setw(8) << right << c << "'" << endl;
  cout << "  <Retest w/i width=10                  > c(int)='" << setw(10) << c << "'" << endl;
  cout << "  <Retest w/o modifiers                 > c(int)='" << c << "'" << endl;
  cout << "  <Retest on fpt number. No effect.     > a(double)='" << a << "'" << endl;
  
  // cout is too lousy to control. That's why we still use c-style printf in c++ for debugging.
  // by the way, notice to use c-lib in c++ programs, the best practice is to include c-equivalent header
  // instead of c headers, they imports symbols into namespace "std".
  // More precisely, The difference is that stdio.h and other C-like libraries when imported in a C++ file 
  // may pollute the global namespace, while the corresponding C++ headers (cstdio, cstdlib, cassert) place 
  // the corresponding functions, variables, etc., in the std namespace.
  // ref link : https://cplusplus.com/reference/cstdio/
  //
  // for example, to include stdio.h, we should,
  //
  //   #include <cstdio>    // instead of #include <stdio.h>
  //
  // However, in this test we didn't include <cstdio>, but printf still works !! 
  // Need to check whether <iostream> had ever included <cstdio> already.
  //
  // ----- Console output of the following code ----
  //  Test 4 : use c printf still.
  //   fpt/double with precision 8, width 12 : a(double)=' 39.12348239', b(float)=' 39.12348175'
  //   int with width 12 : c(int)='        -128', c(uint)='  4294967168', c(in 8 hex digits)='0xffffff80'
  //
  printf(" Test 4 : use c printf still.\n");
  printf("  fpt/double with precision 8, width 12 : a(double)='%12.8f', b(float)='%12.8f'\n", a, b);
  printf("  int with width 12 : c(int)='%12d', c(uint)='%12u', c(in 8 hex digits)='0x%08x'\n", c, c, c);
}
#endif

// round routine.
// round to 1st decimal = multiply by 10, add 0.5 to it, then drop the faction part, and then divide by 10. 
// the template class T can be float or double.
//
// also notice it's recommended to use "template <typename T>" instead of "template <class T>".
template <class T>
T round(T value, int decimal_digits) {
  return floor(value * pow(10, decimal_digits) + 0.5) / pow(10, decimal_digits);
}

template <class T>
int round_to_best_precision(T orig, T &result){
  int idxDigit = 0;
  T difference;
  
  while(1) {
    result = round(orig, idxDigit);
    difference = orig - result;
    // Use the correct way to tell the equivalence :
    //   if (std::abs(x - y) * 2 / (x + y)) < threshold)  // say, threshold = 1e-6.
    //     return equal.
    if (difference < 0) difference = 0 - difference;
    if ((difference * 2 / (orig + result)) < 1e-6) break;
    idxDigit++;
  }

  return idxDigit;
}

#ifdef Also_Run_Precision_Test

void precision_test(void){
  float x = 3.192837465f;
  double y = 321.192837465;
  double z = 123.00;

  cout << "originally            x(float)='3.192837465', y(double)='321.192837465', z(double)='123.00'" << endl;
  cout << fixed << setprecision(9);
  cout << "after precision loss, x(float)='" << x << "', y(double)='" << y << "', z(double)='" << z << "'" << endl;
  cout << fixed;
  float x_result;
  int idx = round_to_best_precision<float>(x, x_result);
  cout << setprecision(idx);
  cout << "After rounding to " << idx << "th decimal digit, result x='" << x_result << "'" << endl;
  double y_result;
  idx = round_to_best_precision<double>(y, y_result);
  cout << setprecision(idx);
  cout << "After rounding to " << idx << "th decimal digit, result y='" << y_result << "'" << endl;
  double z_result;
  idx = round_to_best_precision<double>(z, z_result);
  cout << setprecision(idx);
  cout << "After rounding to " << idx << "th decimal digit, result y='" << z_result << "'" << endl;
}

#endif