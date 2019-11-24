// Author: Jordan Randleman -- defmac.cpp -- lisp-esque macros in C++
#include <set>
#include <unordered_set>
// vector of std::pair enables client-side hierarchic #defmac syntax structs:
// overlapping syntaxes can coexist IFF more restrictive instances defined 1st
#include <vector>
#include <unordered_map>
#include <string>
#include <regex>
#include <iostream>
#include <fstream>
#include <cstdlib>

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
 *                             all $vars instances,& type MUST != a macro arg *
 * *****************************************************************************
 *                              3 CMD-LINE FLAGS                              *
 * *****************************************************************************
 * (0) -l     // Show info flag                                               *
 * (1) -local // Change local file - ie do NOT generate a new file            *
 * (2) -o     // Next string = filename for generated file                    *
 * *****************************************************************************
 *                            ABOUT #defmac MACROS                            *
 * *****************************************************************************
 * (0) HOW-TO:                                                                *
 *     => #defmac `custom syntax`C++ replacement`                             *
 *        - Note back-tick '`' seperation                                     *
 *        - Denote macro's arg tokens w/ '$' prefix                           *
 *     => IE: #defmac `loop $var to $end`for(int $var=0; $var!=$end; ++$var)` *
 *                                                                            *
 * (1) TYPE-DISTRIBUTED CASTING:                                              *
 *     => Prefix variable name instances w/in C++ arg w/ type                 *
 *     => IE: #defmac `$name = ($args) -> $result;`                           *
 *                      auto $name = []($<auto>args){return $result;};`       *
 *        - $args's C++ var-token instances = prefixed by "auto"              *
 *     => IE: "sum = (x, y) -> x + y;" BECOMES:                               *
 *            "auto sum = [](auto x, auto y){return x + y;};"                 *
 *                                                                            *
 * (2) HELPS CREATE:                                                          *
 *     => Domain-specific languages, redefine C++ syntax & retain C++ power   *
 *     => Non-ascii macro labels, ie #defmac `∑`SumUp` == #define ∑ SumUp     *
 *     => New operators/methods,  ie #defmac `$base ** $exp`pow($base, $exp)` *
 * *****************************************************************************
 *                            ABOUT #defmac_include                           *
 * *****************************************************************************
 * (0) #defmac_include `someFile.someExtension`                               *
 *     => Parses & stores #defmac instances within "someFile.someExtension"   *
 * (1) To consolidate #defmac's for multiple files in only 1 place            *
 * *****************************************************************************
 *                BEYOND #defmac: DIRECTLY ACCESSING THE PARSER               *
 * *****************************************************************************
 * (0) Parser Invoked By "DEF_MAC::Script" Object                             *
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
 *     => Look at DEF_MAC::Script's public members for more!                  *
 * ****************************************************************************/

/******************************************************************************
* CUSTOM SCRIPT SYNTAX MAP: USES VECTOR OF PAIRS W/ C++ KEYS & SCRIPT VALS
******************************************************************************/

namespace DEF_MAC {
  class Script {
  public:
    // Aliases:
    using string = std::string;
    using regex  = std::regex;

    using SyntaxMap      = std::vector<std::pair<string,regex>>; 
    using CppAlias       = std::pair<string,regex>;
    using PrefixVector   = std::vector<string>;

    using SyntaxIterator = std::vector<std::pair<string,regex>>::iterator;
    using PrefixIterator = std::vector<string>::iterator;



  private:
    // Private Members
    SyntaxMap    script_syntax; // a "syntax map" of "C++ to Script" aliases
    PrefixVector file_prefixes;


    // Struct For Type-Prefixed #defmac Variables
    struct type_prefix_data {
      string type, token;
      type_prefix_data(string ty, string to) : type(ty), token(to) {}
    };
    std::unordered_multimap<std::string,type_prefix_data> 
    formatted_prefixed_token_data; // Filled with instances of prefixed data as \v{n}


    // Private Methods
    // Merges all strings in container "string_container" and return their accumulation
    template <typename T>
    string merge_container_strings(const T string_container) {
      string accumulated_string;
      for(string str : string_container)
        accumulated_string += str;
      return accumulated_string;
    }

    // Confirm the given file object opened properly
    template<typename T>
    void confirmFileIsOpen(const T &fp, const string &filename) {
      if(!fp.is_open()) {
        std::cerr << "\033[1m" << __FILE__ << ":" << __func__ << ":" << __LINE__ 
                  << ":\033[31m ERROR:\033[0m\033[1m couldn't open file \"" 
                  << filename << "\"!\033[0m\n-:- Terminating Program -:-\n";
        std::exit(EXIT_FAILURE);
      }
    }



  public:
    Script() = default;
    // CTOR given a premade syntax map &/or a file prefix vector
    Script(const SyntaxMap custom_script_syntax_map, const PrefixVector file_prefix_vect = {}) {
      script_syntax = custom_script_syntax_map;
      file_prefixes = file_prefix_vect;
    }
    // CTOR given only a file prefix vector
    Script(const PrefixVector file_prefix_vect) {file_prefixes = file_prefix_vect;}
    bool show_info = false;


    // Push members
    // NOTE: 2 push aliases, 1 if given a script syntax regex & the other a script syntax string
    void push_alias(const string cpp_key, const regex script_syntax_val)  {script_syntax.push_back(CppAlias(cpp_key, script_syntax_val));}
    void push_alias(const string cpp_key, const string script_syntax_val) {regex reg(script_syntax_val); push_alias(cpp_key, reg);}
    void push_prefix(const string prefix) {file_prefixes.push_back(prefix);}


    // Pop members
    bool pop_cpp_alias(const string cpp_key) {
      for(auto e = script_syntax.begin(); e != script_syntax.end(); ++e)
        if(e->first == cpp_key) {
          script_syntax.erase(e);
          return true;
        }
      return false;
    }
    bool pop_prefix(const string prefix) {
      for(auto it = file_prefixes.begin(); it != file_prefixes.end(); ++it)
        if(*it == prefix) {
          file_prefixes.erase(it);
          return true;
        }
      return false;
    }


    // Clear members
    void clear_alias()  {script_syntax.clear();}
    void clear_prefix() {file_prefixes.clear();}
    void clear()        {script_syntax.clear(), file_prefixes.clear();}
    

    // Size members
    int alias_size()  const {return script_syntax.size();}
    int prefix_size() const {return file_prefixes.size();}


    // Begin & end members
    SyntaxIterator alias_begin()  {return script_syntax.begin();}
    SyntaxIterator alias_end()    {return script_syntax.end();}
    PrefixIterator prefix_begin() {return file_prefixes.begin();}
    PrefixIterator prefix_end()   {return file_prefixes.end();}


    // Return COPIES of data container members
    SyntaxMap    syntax() const {return script_syntax;}
    PrefixVector prefix() const {return file_prefixes;}


    // !!! USE THE CREATED DATA MEMBERS TO ETHER MANIPULATE A BUFFER OR FILE !!!

    // Scrape, parse, & write file "read_filename" to "write_filename" according to 
    // "script_syntax" member, prefixing file w/ code-block strings in the "file_prefixes" 
    // vector (File prefixes useful to pass fcns your script's syntax may implicitely invoke),
    // & clustering file's headers near top if "clusterHeaders" optional arg is true
    void map_file(const string read_filename, string write_filename = "", bool clusterHeaders = false) {
      using namespace std;
      if(write_filename.empty()) write_filename = read_filename;
      ifstream read_file(read_filename.c_str());
      confirmFileIsOpen(read_file, read_filename);
      string scriptBuffer = {istreambuf_iterator<char>(read_file), istreambuf_iterator<char>{}}; // slurp file
      string convertedBuffer = map_buffer(scriptBuffer); // convert every syntax-map val to their respective C++ key
      read_file.close();

      convertedBuffer = merge_container_strings(file_prefixes) + convertedBuffer;
      if(clusterHeaders)
        convertedBuffer = cluster_buffer_headers(convertedBuffer);

      // trim down sequences > 2 '\n' down to just 2 '\n'
      const regex multi_newlines(R"(\n{3,})");
      convertedBuffer = regex_replace(convertedBuffer, multi_newlines, "\n\n"); 
      
      ofstream write_file(write_filename.c_str());
      confirmFileIsOpen(write_file, write_filename);
      write_file << convertedBuffer;
      write_file.close();
    }


    // Parse #defmac macros from ".dmac" file
    // Enables keeping language syntax macros consolidated in a single file
    void parse_dmac_file(const std::string filename) {
      using namespace std;
      if(filename.empty()) return;
      ifstream read_file(filename.c_str());
      confirmFileIsOpen(read_file, filename);
      string scriptBuffer = {istreambuf_iterator<char>(read_file), istreambuf_iterator<char>{}}; // slurp file
      map_buffer(scriptBuffer); // convert every syntax-map val to their respective C++ key
      read_file.close();
    }


    // Given std::string buffer: parses out, regex-ifies, and inserts #defmac
    // "macros" to the "script_syntax" std::vector of std::pair member. 
    // Returns std::string of accumulated #defmac "macros" rmvd from buffer.
    string parse_defmac_macros(string &buffer) {
      using namespace std;
      using DefMacMap   = vector<pair<string,string>>; 
      using DefMacAlias = pair<string,string>;

      const regex variable_token(R"((\$\w+))");              // '$'-prefixed token regex for defmac var names
      const regex escaped_var_token(R"((\\\$\w+))");
      const string regex_control_chars(R"(\^$.*+?()[]{}|)"); // escape these in client landef's
      string defmac_buffer;                                  // accumulates parsed defmac "macros"
      string defmac_as_regex;                                // #defmac converted to regex notation 

      DefMacMap defmac_macro_map;       // read #defmac's into this map
      DefMacMap defmac_macro_regex_map; // write regex-fied #defmac syntax definitions here

      // Parse "#defmac_include" statements to parse premade "#defmac" macros defined
      // in the "#defmac_include"d external file => #defmac_include `someFile.anyExtension`
      const regex defmac_include(R"(#defmac_include\s*`(.+)`)");
      smatch defmac_include_matches;
      while(regex_search(buffer, defmac_include_matches, defmac_include)) {
        buffer = defmac_include_matches.prefix().str() + defmac_include_matches.suffix().str();
        defmac_buffer += defmac_include_matches.str(0) + "\n";
        parse_dmac_file(defmac_include_matches.str(1));
      }

      // put each instance of #defmac into map of lang-C++ pairs && remove them from the buffer 
      const regex language_definition(R"(#defmac(\s*?)`((.|\n)+?)`((.|\n)+?)`)"); // regex for defmac "macro"s
      smatch defmac_match; 
      while(regex_search(buffer, defmac_match, language_definition)) {
        defmac_macro_map.push_back(DefMacAlias(defmac_match.str(2),defmac_match.str(4))); // save #defmac "script : C++" pair
        defmac_buffer += defmac_match.str(0) + "\n";                        // accumulate defmac "macro"
        buffer = defmac_match.prefix().str() + defmac_match.suffix().str(); // remove #defmac from buffer        
      }

      // store type-prefixed instances of variable tokens to distribute types to later
      const regex variable_prefix_token(R"((\$<(\w+)>(\w+)))");
      unordered_multimap<string,type_prefix_data> prefixed_token_data; // Filled with instances of prefixed data
      smatch token_prefix_matches;
      int count = 0;
      for(auto &e : defmac_macro_map)
        while(regex_search(e.second, token_prefix_matches, variable_prefix_token)) {
          prefixed_token_data.insert(make_pair(e.first,type_prefix_data(token_prefix_matches.str(2),"$"+token_prefix_matches.str(3))));
          e.second = token_prefix_matches.prefix().str() + 
                     "/*" + to_string(count++) + "*/" + // comment w/ unique # prevents C++ defn overlaps once <type> distributions rmvd
                     "$" + token_prefix_matches.str(3) + token_prefix_matches.suffix().str();
        }

      // convert defmac "macros" to parsable regex notation
      int variable_syntax_index = 1;
      for(auto elt : defmac_macro_map) { 
        auto prefixed_token_range = prefixed_token_data.equal_range(elt.first); // iterator withing prefix data map
        // replace var tokens for C++ syntax equivalent with \v1, \v2, etc
        variable_syntax_index = 1;
        for(sregex_iterator it(elt.first.begin(),elt.first.end(),variable_token); it!=sregex_iterator{}; ++it,variable_syntax_index+=2) {
          // replace the variable name with its "\v{n}" equivalent
          for(auto prefix_iter = prefixed_token_range.first; prefix_iter != prefixed_token_range.second; ++prefix_iter)
            if(prefix_iter->second.token == it->str()) {
              prefix_iter->second.token = R"(\v)" + to_string(variable_syntax_index);
              break;
            }
          regex specific_var_token_instance(R"((\)" + it->str() + R"(\b))");
          elt.second = regex_replace(elt.second, specific_var_token_instance, R"(\v)" + to_string(variable_syntax_index)); 
        }

        // escape all regex control characters in the map's string keys (ie the syntax defn's)
        defmac_as_regex = elt.first;
        for(auto letter : regex_control_chars)
          defmac_as_regex = regex_replace(defmac_as_regex, regex(string(R"(\)") + letter), string(R"(\)") + letter);

        // replace repeating $token names w/ backtracing regex tokens
        set<string> seen_tokens;
        string backtraced_buffer(defmac_as_regex);
        auto token_iter = sregex_iterator(defmac_as_regex.begin(), defmac_as_regex.end(), escaped_var_token);
        int idx = 1;
        for(; token_iter != sregex_iterator{}; ++token_iter, idx+=2) {
          if(seen_tokens.find(token_iter->str()) != seen_tokens.end()) continue;
          seen_tokens.insert(token_iter->str());
          int token_idx_end = defmac_as_regex.find(token_iter->str()) + token_iter->str().size();
          string pre_first_token = defmac_as_regex.substr(0,token_idx_end), 
                 post_first_token = defmac_as_regex.substr(token_idx_end);
          backtraced_buffer = pre_first_token + regex_replace(post_first_token, regex(token_iter->str()), to_string(idx));
        }
        defmac_as_regex = backtraced_buffer;

        // replace all "$..." with a catch-all regex, EXCEPT for 2 unique cases:
        //   1) if var token ends the syntax defn, then its regex equivalent only scrapes to last NON-SPACE-CHAR
        //   2) if var token starts the syntax defn, then its regex equivalent only starts scraping AT a NON-SPACE-CHAR
        defmac_as_regex = regex_replace(defmac_as_regex, escaped_var_token, R"(((.)+?))");
        if(defmac_as_regex.rfind(R"(((.)+?))") == defmac_as_regex.size()-7) // var token ends sytax defn
          defmac_as_regex = defmac_as_regex.replace(defmac_as_regex.size()-7, string::npos, R"(((\S)+))");
        if(defmac_as_regex.find(R"(((.)+?))") == 0)                         // var token starts sytax defn
          defmac_as_regex = defmac_as_regex.replace(0, 7, R"(((\S)+))");    // nested parens for idx pattern from \v1, \v2, etc assignment
        
        // save regex-variant of syntax defn
        defmac_macro_regex_map.push_back(DefMacAlias(defmac_as_regex, elt.second));
        // save prefixed (ie type-distributed) data
        for(auto prefix_iter = prefixed_token_range.first; prefix_iter != prefixed_token_range.second; ++prefix_iter)
          formatted_prefixed_token_data.insert(make_pair(elt.second, prefix_iter->second));
      }

      // add generated "C++ : regex" pairs from #defmac "macros" to this' "script_syntax" unordered map member
      for(auto elt : defmac_macro_regex_map) { 
        script_syntax.push_back(CppAlias(elt.second,regex(elt.first)));
        if(show_info)
          cout << endl << elt.first << endl << elt.second << endl;
      }

      return defmac_buffer;
    }


    // Given std::string buffer using the defined custom script syntax, returns ANOTHER buffer
    // (std::string) with the "script_syntax" std::vector of std::pair member's regex 
    // "script-syntax vals" mapped to its "C++ keys"
    string map_buffer(string buffer) {
      using namespace std;

      set<int> var_token_match_idx_set;
      string replace_key, mapped_cppified_key;
      const string defmac_buffer = parse_defmac_macros(buffer);
      const regex var_token_match_reg(R"((\\v[\d]+))");
      const regex token_bound(R"([_[:alpha:]]\w*)");
      regex current_var_token_reg, replace_val;
      smatch matches;

      // for each C++: Script-CppAlias pair in the regex map
      for(auto phrase = script_syntax.begin(); phrase != script_syntax.end(); ++phrase) { 
        replace_key = phrase->first;  // C++ to splice in
        replace_val = phrase->second; // custom script-syntax to splice out

        // mk set of the numbers following any "variable tokens" (ie \\v{\\d}) to know which smatch idx's to retrieve
        var_token_match_idx_set.clear();
        sregex_iterator current_var_token_match(replace_key.begin(), replace_key.end(), var_token_match_reg);
        while(current_var_token_match != sregex_iterator{}) { 
          var_token_match_idx_set.insert(stoi(current_var_token_match->str().substr(2)));
          ++current_var_token_match;
        }

        // map each value (script-syntax skeleton's regex variables) to each 
        // key (C++ syntax skeleton's respective "variable token" placeholders)
        while(regex_search(buffer, matches, replace_val)) {
          mapped_cppified_key = replace_key;                        // get C++ key's syntax map skeleton
          for(auto var_token_match_idx : var_token_match_idx_set) { // map val vars to key placeholders
            current_var_token_reg = R"(\\v)" + to_string(var_token_match_idx);
            // "matches.str(0)" returns entire string that matched, any 
            // idx > 0 returns var_token_match_idx matching the regex expr

            // check whether ought to distribute types across "matches.str(var_token_match_idx)"
            const string current_val = R"(\v)" + to_string(var_token_match_idx);
            string script_syntax_instance = matches.str(var_token_match_idx);
            string prefixedToken_Data, suffixBuffer;

            auto prefix_range = formatted_prefixed_token_data.equal_range(replace_key);
            for(auto it = prefix_range.first; it != prefix_range.second; ++it) {

              if(it->second.token == current_val) {
                smatch token_matches;
                while(regex_search(script_syntax_instance, token_matches, token_bound)) {
                  prefixedToken_Data += token_matches.prefix().str() + it->second.type + " " + token_matches.str(0);
                  script_syntax_instance = suffixBuffer = token_matches.suffix().str();
                }
                prefixedToken_Data += suffixBuffer;
                break;
              }
            }
            if(!prefixedToken_Data.empty()) script_syntax_instance = prefixedToken_Data;

            mapped_cppified_key = regex_replace(mapped_cppified_key, current_var_token_reg, script_syntax_instance);
          }
          buffer = matches.prefix().str() + mapped_cppified_key + matches.suffix().str();
        }
      }
      return "/*\n" + defmac_buffer + "*/\n" + buffer; // prefix buffer with commented defmac "macros"
    }


    // Consolidate headers ("#include"s) to the front of the buffer
    string cluster_buffer_headers(string buffer) {
      using namespace std;
      const regex system_header_pattern(R"(([:blank:]*)#([:blank:]*)include(\s*)<([_[:alpha:]]\w*(\.[_[:alpha:]]\w*)?)>([:blank:]*))");
      const regex local_header_pattern(R"(([:blank:]*)#([:blank:]*)include(\s*)\"([_[:alpha:]]\w*(\.[_[:alpha:]]\w*)?)\"([:blank:]*))");
      vector<string> header_vector;
      
      for(sregex_iterator it(buffer.begin(), buffer.end(), system_header_pattern); it != sregex_iterator{}; ++it)
        header_vector.push_back(it->str());
      for(sregex_iterator it(buffer.begin(), buffer.end(), local_header_pattern);  it != sregex_iterator{}; ++it)
        header_vector.push_back(it->str());
      
      buffer = regex_replace(buffer, system_header_pattern, ""); // rmv system headers
      buffer = regex_replace(buffer, local_header_pattern,  ""); // rmv local directory headers
      for(auto &e : header_vector) e += '\n';                    // 1 header per line

      unordered_set<string> header_set;                          // rmv duplicate headers w/o changing #include order
      for(auto e : header_vector) header_set.insert(e);
      return merge_container_strings(header_set) + buffer;
    }
  }; // end of class Script
};   // end of namespace DEF_MAC

/******************************************************************************
* FILE ERROR HANDLING FUNCTION
******************************************************************************/

// Parses Cmd-Line argv Input:
// CMD LINE FLAGS:
//   1) -l == show info (print defmac.cpp's interpretation of #defmac macros)
//   2) -local == local edit, no new file generated 
//      (typically generates new name via oldFileName+"_DMAC.cpp")
//   3) -o == the following string is the name of the generated file (like GCC)
void confirm_valid_cmd_line_input(int argc, char **argv, bool &show_info, 
std::string &parse_filename, std::string &write_filename) {
  constexpr auto generate_filename = [](const std::string &readFile, const std::string &extension){
    return readFile.substr(0, readFile.rfind(".")) + extension;
  };
  bool no_default_writeFile = false;

  if(argc > 4 || argc < 2) {
    std::cerr << "\n\033[1m" << __FILE__ << ":" << __func__ << ":" << __LINE__ 
     << ":\n\033[31m ERROR:\033[0m\033[1m > 3 OR < 1 cmd-line args recieved!\033[0m\n"
     << "Cmd-Line Args MAY Include:\n"
     << "    \"-l\":     show parser's regex interpretation of \"#defmac\" macros\n"
     << "    \"-local\": edit native file, do \033[1mNOT\033[0m generate a new file\n"
     << "    \"-o\":     following string becomes the generated file's name\n"
     << "Cmd-Line Args MUST Include:\n"
     << "    \"yourFilename.cpp\": file to parse/apply \"#defmac\"-macros/syntax-mapping\n"
     << "-:- Terminating Program -:-\n\n";
    std::exit(EXIT_FAILURE);
  }

  for(int i = 1; i < argc; ++i) {
    if(std::string(argv[i]) == "-l")
      show_info = true;            // show info
    else if(std::string(argv[i]) == "-local")
      no_default_writeFile = true; // edit local file
    else if(std::string(argv[i]) == "-o") {
      no_default_writeFile = true; // custom write file name
      if(i == argc-1) {
        std::cerr << "\033[1m" << __FILE__ << ":" << __func__ << ":" << __LINE__ 
         << ":\033[31m ERROR:\033[0m\033[1m No Custom \"Write\" Filename Passed After '-o'!\033[0m\n"
         << "-:- Terminating Program -:-\n";
        std::exit(EXIT_FAILURE);
      }
      write_filename = std::string(argv[i+1]), ++i;
    } else
      parse_filename = argv[i];
  }

  if(parse_filename.empty()) {
    std::cerr << "\033[1m" << __FILE__ << ":" << __func__ << ":" << __LINE__ 
     << ":\033[31m ERROR:\033[0m\033[1m Cmd-Line Args Missing A Parseable C++ File!\033[0m\n"
     << "-:- Terminating Program -:-\n";
    std::exit(EXIT_FAILURE);
  }

  if(!no_default_writeFile)
    write_filename = generate_filename(parse_filename, "_DMAC.cpp");
}

/******************************************************************************
* MAIN EXECUTION
******************************************************************************/

int main(int argc, char **argv) {
  bool show_info = false;
  std::string parse_filename, write_filename;
  confirm_valid_cmd_line_input(argc,argv,show_info,parse_filename,write_filename);

  DEF_MAC::Script yourScript;       // DEF_MAC::Script object to map a file
  yourScript.show_info = show_info; // show_info flag for mapping in "yourScript"

  // Register Custom Regex (more direct control then #defmac macro) 
  //     To "yourScript"s Parser, (C++ : Custom_syntax)
  // yourScript.push_alias(R"(\v1std::vector<\v2> \v3(\v4);)", R"((\s)([_[:alpha:]]\w*) ([_[:alpha:]]\w*) = \[(\d+)\];)"); 
  
  // Register String To Prefix The File W/ Via "yourScript"s Parser
  // yourScript.push_prefix("#include <string>\n");

  // Cluster A Buffer's Headers (suppose some file-content std::string "buffer")
  // std::string newBuffer = yourScript.cluster_buffer_headers(buffer);

  // Map A Buffer With Registered Aliases & Any Contained #defmac Macros
  //     (suppose some file-content std::string "buffer")
  // std::string newBuffer yourScript.map_buffer(buffer)

  // Map File & Cluster Its Headers (triggered by last "true" flag passed)
  // yourScript.map_file(parse_filename, write_filename, true);

  yourScript.map_file(parse_filename, write_filename);

  std::cout << std::endl << "\033[1m"
            << R"(        __      __   ______  __     ____    _____   ______  _____  ____         )" << std::endl
            << R"(       |  \    /  \ (  __  ))  )__ (    \  /     ) /      )/  _  )/ _  )        )" << std::endl
            << R"(       |   \  / /\ )| |_ (//  /   )/ _   )/ /(  / /  /(  /(  /_)// /_)/         )" << std::endl
            << R"(       |    \( ( (/ |  _) / /  / // (_) // /  \/ /  /  \/  ) __/ ) __/          )" << std::endl
            << R"(       |  () )) )   | |  / (__/ // __  // (  __ (  (  __  / (   / (             )" << std::endl
            << R"(       |    /( (_/)_/ /_/ / _/ // / / ((   \_) )_\  \_) )/   ) /   )            )" << std::endl
            << R"(       (___/  \__/(__/(___)(___)\_)(___)\_____/(_)\____/(___/ (___/             )" << "\033[0m\n\n";

  if(!write_filename.empty())
    std::cout << "\033[1m -:- " << parse_filename << " ==PARSED=MAPPED=> " << write_filename << " -:-\033[0m\n\n";
  else
    std::cout << "\033[1m -:- " << parse_filename << " LOCALLY EDITED! -:-\033[0m\n\n";

  return 0;
}
