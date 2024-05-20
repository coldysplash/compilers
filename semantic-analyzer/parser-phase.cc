#include "cool-parse.h"
#include "cool-tree.h"
#include "utilities.h"
#include <cstdio>
#include <unistd.h>

#include <unordered_set>

std::FILE *token_file = stdin;
extern Classes parse_results;
extern Program ast_root;

extern int curr_lineno;
const char *curr_filename = "<stdin>";
extern int parse_errors;

// Debug flags
extern int yy_flex_debug;
extern int cool_yydebug;
int lex_verbose = 0;

extern int cool_yyparse();

namespace semantic {

void check_builtin_type(std::string type, Expression expr) {
  std::string expr_type = expr->get_expr_type();
  if (expr_type == "no_expr_class") {
    return;
  }
  if (type == "Int" && expr_type != "int_const_class") {
    std::cerr
        << "Semantic Error! initialization of Int with non-integer value\n";
  } else if (type == "Bool" && expr_type != "bool_const_class") {
    std::cerr
        << "Semantic Error! initialization of Bool with non-boolean value\n";
  } else if (type == "String" && expr_type != "string_const_class") {
    std::cerr
        << "Semantic Error! initialization of String with non-string value\n";
  }
}

} // namespace semantic

int main(int argc, char **argv) {
  yy_flex_debug = 0;
  cool_yydebug = 0;
  lex_verbose = 0;

  for (int i = 1; i < argc; i++) {
    token_file = std::fopen(argv[i], "r");
    if (token_file == NULL) {
      std::cerr << "Error: can not open file " << argv[i] << std::endl;
      std::exit(1);
    }
    curr_lineno = 1;

    cool_yyparse();
    if (parse_errors != 0) {
      std::cerr << "Error: parse errors\n";
      std::exit(1);
    }

    // ast_root->dump_with_types(std::cerr, 0);

    // idtable.print();
    // inttable.print();
    // stringtable.print();

    Symbol filename = stringtable.add_string("<builtin-classes>");
    Symbol Object = idtable.add_string("Object");
    Symbol Bool = idtable.add_string("Bool");
    Symbol Int = idtable.add_string("Int");
    Symbol String = idtable.add_string("String");
    Symbol SELF_TYPE = idtable.add_string("SELF_TYPE");

    std::unordered_set<Symbol> not_inherited{Bool, Int, String, SELF_TYPE};
    std::unordered_set<Symbol> classes_names{
        Object, Bool, Int, String, SELF_TYPE};
    std::unordered_map<std::string, std::string> classes_hierarchy{
        {"Object", "Object"}};

    std::vector<std::string> classes;
    std::vector<std::string> parents;
    for (int i = parse_results->first(); parse_results->more(i);
         i = parse_results->next(i)) {
      Symbol class_name = parse_results->nth(i)->get_name();
      // проверка корректности имён классов
      auto result = classes_names.insert(class_name);
      if (!result.second) {
        std::cerr << "Semantic Error! class '" << class_name->get_string()
                  << "' redeclared." << std::endl;
      }
      classes.push_back(class_name->get_string());

      Symbol parent_name = parse_results->nth(i)->get_parent();
      // проверка корректности имён родительских классов
      if (not_inherited.find(parent_name) != not_inherited.end()) {
        std::cerr << "Semantic Error! class '" << class_name->get_string()
                  << "': can't use parent class '" << parent_name->get_string()
                  << "' (builtin)\n";
      }
      // формируем таблицу class <-> parent
      classes_hierarchy[class_name->get_string()] = parent_name->get_string();
      if (!classes_hierarchy.contains(parent_name->get_string())) {
        std::cerr << "Semantic Error! Unknow parent of class '"
                  << class_name->get_string() << "' - '"
                  << parent_name->get_string() << "'\n";
      }

      // проверяем методы класса
      Features features = parse_results->nth(i)->get_features();
      std::unordered_set<std::string> features_names;
      for (int j = features->first(); features->more(j);
           j = features->next(j)) {
        Feature feature = features->nth(j);
        std::string feature_name = feature->get_name()->get_string();

        if (feature_name == "self") {
          std::cerr << "Semantic Error! can't use 'self' as feature name"
                    << std::endl;
        }

        auto res = features_names.insert(feature_name);
        if (!res.second) {
          std::cerr << "Semantic Error! feature '" << feature_name << "' in '"
                    << class_name << "' already exists!" << std::endl;
        }

        Symbol type = feature->get_type();
        if (classes_names.find(type) == classes_names.end()) {
          std::cerr << "Semantic Error! unknown type '" << type << "' in "
                    << feature_name << std::endl;
        }

        if (feature->get_feature_type() == "method_class") {
        } else { // attributes_class
          attr_class *attr = dynamic_cast<attr_class *>(feature);
          semantic::check_builtin_type(
              attr->get_type()->get_string(), attr->get_expr());
        }
      }
    }

    std::fclose(token_file);
  }
}
