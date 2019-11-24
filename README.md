# Defmac-CPP
```
        __      __   ______  __     ____    _____   ______  _____  ____         
       |  \    /  \ (  __  ))  )__ (    \  /     ) /      )/  _  )/ _  )        
       |   \  / /\ )| |_ (//  /   )/ _   )/ /(  / /  /(  /(  /_)// /_)/         
       |    \( ( (/ |  _) / /  / // (_) // /  \/ /  /  \/  ) __/ ) __/          
       |  () )) )   | |  / (__/ // __  // (  __ (  (  __  / (   / (             
       |    /( (_/)_/ /_/ / _/ // / / ((   \_) )_\  \_) )/   ) /   )            
       (___/  \__/(__/(___)(___)\_)(___)\_____/(_)\____/(___/ (___/           
```
## LISP-esque C++ Macros: Syntax Aliasing &amp; Domain-Specific Langs!
------------------------------------------------------------------------
## Using The Parser:
### Compile >= C++11:
```c++
g++ -o defmac defmac.cpp
./defmac yourFile.cpp        // parse yourFile.cpp & output results to yourFile_DMAC.cpp
./defmac -local yourFile.cpp // parse yourFile.cpp, making changes to the original file
./defmac -l yourFile.cpp     // parse yourFile.cpp & print parser's #defmac conversions
./defmac -o yourFile2.cpp yourFile.cpp // parse yourFile.cpp & name output file yourFile2.cpp
```

### Implementation:
* parse `#defmac` macros, converting to regex-string pairs pushed to a vector, then map the file

### Use References:
* [Demo Sample Execution File](https://github.com/jrandleman/Defmac-CPP/blob/master/defmac_sampleExec.cpp) ([Parser's Conversion Shown Here](https://github.com/jrandleman/Defmac-CPP/blob/master/defmac_sampleExec_DMAC.cpp))</br>
* [6 Caveats for Using `defmac.cpp`](#6-caveats)

### Example:
```c++
#include <iostream>
// #defmac `custom syntax`c++ replacement` ($-prefixed tokens denote args)
#defmac `loop $var from $start to $end`for(auto $var = $start; $var < $end; ++$var)`

int main() {
  int x[] = {1, 2, 3, 4, 5};
  loop i from 0 to 5 {
    std::cout << x[i] << std::endl;
  }
  return 0;
}
```

------------------------------------------------------------------------
## 6 Caveats:
```c++
/* *****************************************************************************
 *                                  6 CAVEATS                                 *
 * *****************************************************************************
 * (0) RESERVED CHARS:         '`' & '$'                                      *
 * (1) BLIND TO BOUNDS:        Will replace string contents matching #defmac  *
 * (2) UNIVERSAL APPLICATION:  #defmac macros applied throughout entire file  *
 * (3) NO RECURSION:           Nested #defmac instances = undefn behavior     *
 * (4) SEQUENTIALLY PROCESSED: Order overlapping #defmac defns top-down by    *
 *                             most constraints                               *
 * (5) DISTRIBUTED TYPE CAST:  Type-distributed casting via $<type>vars casts *
 * ****************************************************************************/
```
------------------------------------------------------------------------
## Using `#defmac` Macros:
### How-To:
```c++
/* => #defmac `custom syntax`C++ replacement`                             *
 *    - Note back-tick '`' seperation                                     *
 *    - Denote macro's arg tokens w/ '$' prefix                           *
 * => IE: #defmac `loop $var to $end`for(int $var=0; $var!=$end; ++$var)` */
```

### Type-Distributed Casting:
```c++
/* => Prefix variable name instances w/in C++ arg w/ type         *
 * => IE: #defmac `$name = ($args) -> $result;`                   *
 *                 auto $name = []($<auto>args){return $result;};`*
 *    - $args's C++ var-token instances = prefixed by "auto"      *
 * => IE: "sum = (x, y) -> x + y;" BECOMES:                       *
 *        "auto sum = [](auto x, auto y){return x + y;};"         */
```

### Helps Create:
```c++
/* => Domain-specific languages, redefine C++ syntax & retain C++ power   *
 * => Non-ascii macro labels, ie #defmac `∑`SumUp` == #define ∑ SumUp     *
 * => New operators/methods,  ie #defmac `$base ** $exp`pow($base, $exp)` */
```
------------------------------------------------------------------------
## About `#defmac_include`:

```c++
/* (0) #defmac_include `someFile.someExtension`                               *
 *     => Parses & stores #defmac instances within "someFile.someExtension"   *
 * (1) To consolidate #defmac's for multiple files in only 1 place            */
```
------------------------------------------------------------------------
## Beyond `#defmac`: Directly Accessing The Parser:
```c++
/* (0) Parser Invoked By "DEF_MAC::Script" Object                             *
 *                                                                            *
 * (1) Generating Aliases:                                                    *
 *     => Directly pass syntax aliases in regex form for detailed control     *
 *        - !!! all regex subpatterns ought to be wrapped in parens '()' !!!  *
 *     => Label C++ replacement args as \v{n} where {n} = the # regex match   *
 *        from your syntax regex you wish to splice in \v{n}'s position       *
 *     => METHODS: obj.push_alias(C++ string, syntax regex)                   *
 *                 obj.push_alias(C++ string, syntax string)                  *
 *     => IE: obj.push_alias("\v1 += \v2;", "(\w+) addeq (\d+);");            *
 *                                                                            *
 * (2) File Prefixing:                                                        *
 *     => Prefix a file certain strings (ie a library #include, fcn, etc)     *
 *     => IE: obj.push_prefix("#include <string>");                           *
 *                                                                            *
 * (3) Header Consolidation:                                                  *
 *     => Cluster all #include's to the top of a string, systems above locals *
 *     => string newBuffer = obj.cluster_buffer_headers(oldBuffer);           *
 *                                                                            *
 * (4) Map A File With Prefixes, Aliases, & Any #defmac Macros W/in:          *
 *     => obj.map_file(old_filename, new_mapped_filename);                    *
 *     => Look at DEF_MAC::Script's public members for more!                  */
```
