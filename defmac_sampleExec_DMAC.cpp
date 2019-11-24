/*
#defmac_include `defmac_macros.dmac`
#defmac `$var = $res1 if $condition else $res2;`$var = ($condition) ? $res1 : $res2;`
#defmac `$lambda = ($args) => $operation;`auto $lambda = []($<auto>args){return $operation;};`
#defmac `$type $var = [];`std::vector<$type>$var;`
#defmac `$key_type $var = {$val_type};`std::map<$key_type,$val_type>$var;`
#defmac `$type $var[$start:$end:$step];`$type $var[($end-$start)/$step]; for($type i = $start, j = 0; i < $end; i+=$step, ++j) $var[j] = i;`
#defmac `$type $var[$start:$end];`$type $var[$end-$start]; for($type i = 0; i < $end-$start; ++i) $var[i] = i+$start;`
#defmac `$container.map($elt1 => $operation);`for(auto &$elt1 : $container) $elt1 = $operation;`
#defmac `loop $var : [$start,$end]`for(auto $var = $start; ($start < $end) ? $var <= $end : $var >= $end; ($start < $end) ? ++$var : --$var)`
#defmac `loop $var : ($start,$end]`for(auto $var = $start + (($start < $end)?1:-1); ($start < $end) ? $var <= $end : $var >= $end; ($start < $end) ? ++$var : --$var)`
#defmac `loop $var : [$start,$end)`for(auto $var = $start; ($start < $end) ? $var < $end : $var > $end; ($start < $end) ? ++$var : --$var)`
#defmac `loop $var : ($start,$end)`for(auto $var = $start + (($start < $end)?1:-1); ($start < $end) ? $var < $end : $var > $end; ($start < $end) ? ++$var : --$var)`
#defmac `$base ** $exp`std::pow($base, $exp)`
*/
// Author: Jordan Randleman – defmac_sampleExec.cpp

#include <iostream>
#include <vector>
#include <cmath>
#include <map>

using namespace std;

// include a file to parse #defmac's from (has a GO-esque ":=" assignment operator)

// alternate conditional assignment (a la python)

// mk lambdas by using the type-distribution cast mechanism (on $args here) (a la JavaScript)

// alternate assignments for std::vector & std::map

// initializing arrays with values
// => NOTE the more constrictive version with a "$step" is above the more lax version
//         below, so that the 2nd version won't improperly process instances of the 1st

// applying a "map" method to all containers supporting range-for loop!

// alternate looping syntax using mathematical range notation 
// => IE '[' is inclusive & '(' is exclusive

// redefine adjacent asterisks to form an "exponent" operator (a la python)

int main() { 

  // Using the "exponent" operator for 3^4
  cout << "Using the \"exponent\" operator for 3^4: std::pow(3, 4) = "
       << std::pow(3, 4) << "\n\n";

  // Initializing 2 arrays via colon notation
  cout << "Initializing 2 arrays via colon notation:\n";
  int arr[10-3]; for(int i = 0; i < 10-3; ++i) arr[i] = i+3;
  int arr2[(11-3)/2]; for(int i = 3, j = 0; i < 11; i+=2, ++j) arr2[j] = i;

  for(auto e : arr) cout << e << " ";
  cout << endl;
  for(auto e : arr2) cout << e << " ";

  // Mapping each elt in the 1st array *= 2 via universal container ".map()" method
  cout << "\n\nMapping each elt in the 1st array *= 2 via universal container \".map()\" method:\n";
  for(auto &e : arr) e = e * 2;
  for(auto e : arr) cout << e << " ";

  // Capitalizing a std::string via the universal container ".map()" method
  cout << "\n\nCapitalizing a std::string via the universal container \".map()\" method:\n";
  string str = "hello";
  cout << str << endl;
  for(auto &letter : str) letter = letter - 32; // capitalize entire string!
  cout << str;

  // Using generic lambdas made with the type-distributed cast mechanism
  auto increm = [](/*0*/auto x){return x + 1;};
  auto gt = [](/*0*/auto x, auto y){return x > y;};
  cout << "\n\nUsing generic lambdas made with the type-distributed cast mechanism:\n"
       << "increm(8) = " << increm(8)
       << "\ngt(3,1) = " << gt(3,1);

  // alternate means of std::vector & std::map assignment
  std::vector<int>vec_2;
  std::map<double,int>vec_3;

  // auto-assignment via GO-esque notation from #defmac_include file
  auto myVar = 5.5;

  // python-esque conditional assignment alternative
  int x = (3 > 5) ? 2 : 9;
  int y = (88 >= 12) ? 9 : 100;

  // using alternative looping syntax to push range 40-0 to a vector
  cout << "\n\nUsing alternative looping syntax to push range 40-0 to a vector:\n";
  vector<int> v;
  for(auto i = 40; (40 < 30) ? i <= 30 : i >= 30; (40 < 30) ? ++i : --i) {
    v.push_back(i);
  }

  for(auto i = 30 + ((30 < 20)?1:-1); (30 < 20) ? i <= 20 : i >= 20; (30 < 20) ? ++i : --i) {
    v.push_back(i);
  }

  for(auto i = 19; (19 < 10) ? i < 10 : i > 10; (19 < 10) ? ++i : --i) {
    v.push_back(i);
  }

  for(auto i = 11 + ((11 < -1)?1:-1); (11 < -1) ? i < -1 : i > -1; (11 < -1) ? ++i : --i) {
    v.push_back(i);
  }

  for(auto e : v) {
    cout << e << " ";
  }
  cout << "\n\nBye!\n";

  return 0;
}
