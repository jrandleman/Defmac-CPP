// Author: Jordan Randleman – defmac_sampleExec.cpp

#include <iostream>
#include <vector>
#include <cmath>
#include <map>

using namespace std;

// include a file to parse #defmac's from (has a GO-esque ":=" assignment operator)
#defmac_include `defmac_macros.dmac`

// alternate conditional assignment (a la python)
#defmac `$var = $res1 if $condition else $res2;`$var = ($condition) ? $res1 : $res2;`

// mk lambdas by using the type-distribution cast mechanism (on $args here) (a la JavaScript)
#defmac `$lambda = ($args) => $operation;`auto $lambda = []($<auto>args){return $operation;};`

// alternate assignments for std::vector & std::map
#defmac `$type $var = [];`std::vector<$type>$var;`
#defmac `$key_type $var = {$val_type};`std::map<$key_type,$val_type>$var;`

// initializing arrays with values
// => NOTE the more constrictive version with a "$step" is above the more lax version
//         below, so that the 2nd version won't improperly process instances of the 1st
#defmac `$type $var[$start:$end:$step];`$type $var[($end-$start)/$step]; for($type i = $start, j = 0; i < $end; i+=$step, ++j) $var[j] = i;`
#defmac `$type $var[$start:$end];`$type $var[$end-$start]; for($type i = 0; i < $end-$start; ++i) $var[i] = i+$start;`

// applying a "map" method to all containers supporting range-for loop!
#defmac `$container.map($elt1 => $operation);`for(auto &$elt1 : $container) $elt1 = $operation;`

// alternate looping syntax using mathematical range notation 
// => IE '[' is inclusive & '(' is exclusive
#defmac `loop $var : [$start,$end]`for(auto $var = $start; ($start < $end) ? $var <= $end : $var >= $end; ($start < $end) ? ++$var : --$var)`
#defmac `loop $var : ($start,$end]`for(auto $var = $start + (($start < $end)?1:-1); ($start < $end) ? $var <= $end : $var >= $end; ($start < $end) ? ++$var : --$var)`
#defmac `loop $var : [$start,$end)`for(auto $var = $start; ($start < $end) ? $var < $end : $var > $end; ($start < $end) ? ++$var : --$var)`
#defmac `loop $var : ($start,$end)`for(auto $var = $start + (($start < $end)?1:-1); ($start < $end) ? $var < $end : $var > $end; ($start < $end) ? ++$var : --$var)`

// redefine adjacent asterisks to form an "exponent" operator (a la python)
#defmac `$base ** $exp`std::pow($base, $exp)`



int main() { 

  // Using the "exponent" operator for 3^4
  cout << "Using the \"exponent\" operator for 3^4: 3 ** 4 = "
       << 3 ** 4 << "\n\n";

  // Initializing 2 arrays via colon notation
  cout << "Initializing 2 arrays via colon notation:\n";
  int arr[3:10];
  int arr2[3:11:2];

  for(auto e : arr) cout << e << " ";
  cout << endl;
  for(auto e : arr2) cout << e << " ";

  // Mapping each elt in the 1st array *= 2 via universal container ".map()" method
  cout << "\n\nMapping each elt in the 1st array *= 2 via universal container \".map()\" method:\n";
  arr.map(e => e * 2);
  for(auto e : arr) cout << e << " ";

  // Capitalizing a std::string via the universal container ".map()" method
  cout << "\n\nCapitalizing a std::string via the universal container \".map()\" method:\n";
  string str = "hello";
  cout << str << endl;
  str.map(letter => letter - 32); // capitalize entire string!
  cout << str;

  // Using generic lambdas made with the type-distributed cast mechanism
  increm = (x) => x + 1;
  gt = (x, y) => x > y;
  cout << "\n\nUsing generic lambdas made with the type-distributed cast mechanism:\n"
       << "increm(8) = " << increm(8)
       << "\ngt(3,1) = " << gt(3,1);

  // alternate means of std::vector & std::map assignment
  int vec_2 = [];
  double vec_3 = {int};

  // auto-assignment via GO-esque notation from #defmac_include file
  myVar := 5.5

  // python-esque conditional assignment alternative
  int x = 2 if 3 > 5 else 9;
  int y = 9 if 88 >= 12 else 100;

  // using alternative looping syntax to push range 40-0 to a vector
  cout << "\n\nUsing alternative looping syntax to push range 40-0 to a vector:\n";
  vector<int> v;
  loop i : [40,30] {
    v.push_back(i);
  }

  loop i : (30,20] {
    v.push_back(i);
  }

  loop i : [19,10) {
    v.push_back(i);
  }

  loop i : (11,-1) {
    v.push_back(i);
  }

  for(auto e : v) {
    cout << e << " ";
  }
  cout << "\n\nBye!\n";

  return 0;
}
