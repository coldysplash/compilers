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

class__class *find_class(std::string name, Classes classes) {
  for (int i = classes->first(); classes->more(i); i = classes->next(i)) {
    class__class *cur_class = dynamic_cast<class__class *>(classes->nth(i));
    if (name == cur_class->get_name()->get_string()) {
      return cur_class;
    }
  }
  return nullptr;
}

bool check_signatures(method_class *m1, method_class *m2) {
  if (m1->get_type() != m2->get_type()) {
    return false;
  }

  Formals m1_formals = m1->get_formals();
  Formals m2_formals = m2->get_formals();
  // Check formals count
  if (m1_formals->len() != m2_formals->len()) {
    return false;
  }

  // Loop through formals
  for (int i = m1_formals->first(); m1_formals->more(i);
       i = m1_formals->next(i)) {
    formal_class *m1_formal = dynamic_cast<formal_class *>(m1_formals->nth(i));
    formal_class *m2_formal = dynamic_cast<formal_class *>(m2_formals->nth(i));

    std::string name1 = m1_formal->get_name()->get_string();
    std::string name2 = m2_formal->get_name()->get_string();
    // Check formal names
    if (name1 != name2) {
      return false;
    }

    // Get formal types
    std::string type1 = m1_formal->get_type()->get_string();
    std::string type2 = m1_formal->get_type()->get_string();
    // Check formal types
    if (type1 != type2) {
      return false;
    }
  }
  return true;
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

    // Добавление в AST класса (узла) для встроенных типов
    Symbol filename = stringtable.add_string("<builtin-classes>");
    Symbol Object = idtable.add_string("Object");
    Symbol Bool = idtable.add_string("Bool");
    Symbol Int = idtable.add_string("Int");
    Symbol String = idtable.add_string("String");
    Symbol SELF_TYPE = idtable.add_string("SELF_TYPE");
    Symbol Main = idtable.add_string("Main");

    std::unordered_set<Symbol> not_inherited{Bool, Int, String, SELF_TYPE};
    std::unordered_set<Symbol> classes_names{
        Object, Bool, Int, String, SELF_TYPE};
    std::unordered_map<std::string, std::string> classes_hierarchy;

    std::vector<std::string> classes;
    std::vector<std::string> parents;
    for (int i = parse_results->first(); parse_results->more(i);
         i = parse_results->next(i)) {
      Symbol class_name = parse_results->nth(i)->get_name();
      // проверка корректности имён классов
      auto result = classes_names.insert(class_name);
      if (!result.second) {
        std::cerr << "Semantic Error! class '" << class_name->get_string()
                  << "' redeclared.\n";
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
      if (std::string(parent_name->get_string()) != "Object") {
        if (!classes_hierarchy.contains(parent_name->get_string())) {
          std::cerr << "Semantic Error! Unknown parent of class '"
                    << class_name->get_string() << "' - '"
                    << parent_name->get_string() << "'\n";
        }
      }

      // проверяем методы класса
      Features features = parse_results->nth(i)->get_features();
      std::unordered_set<std::string> features_names;
      for (int j = features->first(); features->more(j);
           j = features->next(j)) {
        Feature feature = features->nth(j);
        std::string feature_name = feature->get_name()->get_string();

        if (feature_name == "self") {
          std::cerr << "Semantic Error! can't use 'self' as feature name\n";
        }

        auto res = features_names.insert(feature_name);
        if (!res.second) {
          std::cerr << "Semantic Error! feature '" << feature_name << "' in '"
                    << class_name << "' already exists!" << '\n';
        }

        Symbol type = feature->get_type();
        if (classes_names.find(type) == classes_names.end()) {
          std::cerr << "Semantic Error! Unknown type '" << type << "' in "
                    << feature_name << '\n';
        }

        if (feature->get_feature_type() == "method_class") {
          // Check method overrides - must have same signature
          if (std::string(parent_name->get_string()) != "Object") {
            class__class *parent =
                semantic::find_class(parent_name->get_string(), parse_results);

            if (parent) {
              Features parent_features = parent->get_features();

              // Loop through parent features
              for (int a = parent_features->first(); parent_features->more(a);
                   a = parent_features->next(a)) {
                Feature parent_feature = parent_features->nth(a);

                // Get feature name
                std::string parent_feature_name =
                    parent_feature->get_name()->get_string();

                // If there is parent feature with same name
                if (parent_feature_name == feature_name) {

                  // Check if feature is same type
                  if (parent_feature->get_feature_type() !=
                      feature->get_feature_type()) {
                    std::cerr << "Semantic Error! wrong override of feature '"
                              << feature_name << "' from class '"
                              << parent_name->get_string() << "' in class '"
                              << class_name << "'\n";
                  }

                  // Check method signatures
                  method_class *cur_method =
                      dynamic_cast<method_class *>(feature);
                  method_class *parent_method =
                      dynamic_cast<method_class *>(parent_feature);
                  if (!semantic::check_signatures(cur_method, parent_method)) {
                    std::cerr
                        << "Semantic Error! '" << feature_name
                        << "' method from class '" << parent_name->get_string()
                        << "' doesn't match override version of it in class '"
                        << class_name << "'";
                  }
                }
              }
            } else {
              std::cerr << "Semantic Error! Unknown parent of class '"
                        << class_name->get_string() << "' - '"
                        << parent_name->get_string() << "'\n";
            }
          }
          Formals formals = feature->get_formals();
          // Local formals names
          std::unordered_set<std::string> formals_names;

          // Loop through formals
          for (int k = formals->first(); formals->more(k);
               k = formals->next(k)) {

            std::string formal_name = formals->nth(k)->get_name()->get_string();

            // 'self' name check
            if (formal_name == "self") {
              std::cerr << "Semantic Error! can't use 'self' as formal name\n";
            }

            // Unique name check
            auto f_result = formals_names.insert(formal_name);
            if (!f_result.second) {
              std::cerr << "Semantic Error! formal '" << formal_name
                        << "' in method '" << feature_name
                        << "' already exists!\n";
            }

            type = formals->nth(k)->get_type();

            // Check formal type
            if (classes_names.find(type) == classes_names.end()) {
              std::cerr << "Semantic Error! Unknown type '"
                        << type->get_string() << "' in " << formal_name << '\n';
            }

            // Get method expression
            Expression expr = features->nth(j)->get_expr();

            // block_class check
            if (expr->get_expr_type() == "block_class") {

              // Get expressions from block
              Expressions exprs = expr->get_expressions();

              // Block expressions check
              for (int l = exprs->first(); exprs->more(l); l = exprs->next(l)) {
                Expression current = exprs->nth(l);

                // let
                if (current->get_expr_type() == "let_class") {

                  // Get let-expr variable name
                  formal_name = current->get_name();

                  // 'self' name check
                  if (formal_name == "self") {
                    std::cerr
                        << "Semantic Error! can't use 'self' as formal name\n";
                  }

                  // Check unique of nested formal
                  f_result = formals_names.insert(formal_name);
                  if (!f_result.second) {
                    std::cerr << "Semantic Error! formal '" << formal_name
                              << "' in method '" << feature_name << "' from '"
                              << class_name->get_string()
                              << "' already exists!\n";
                  }

                  // Let-expr formal type check
                  type = current->get_type_decl();
                  if (classes_names.find(type) == classes_names.end()) {
                    std::cerr << "Semantic Error! Unknown type '"
                              << type->get_string() << "' in " << formal_name
                              << '\n';
                  }
                }
              }
            }
          }
        } else { // attributes_class
          attr_class *attr = dynamic_cast<attr_class *>(feature);
          semantic::check_builtin_type(
              attr->get_type()->get_string(), attr->get_expr());
        }
      }
      // Check existence of method main in class Main
      if (std::string(class_name->get_string()) == "Main" &&
          features_names.find("main") == features_names.end()) {
        std::cerr << "No method 'main' in class 'Main'\n";
      }
    }

    // Check existence of class Main
    if (classes_names.find(Main) == classes_names.end()) {
      std::cerr << "class Main doesn't exist\n";
    }

    std::fclose(token_file);
  }
  std::cout << std::endl;
}