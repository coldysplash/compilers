#include <unistd.h>
#include <cstdio>
#include "cool-tree.h"
#include "utilities.h"
#include "cool-parse.h"

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

int main(int argc, char **argv)
{
    yy_flex_debug = 0;
    cool_yydebug = 0;
    lex_verbose = 0;

    for (int i = 1; i < argc; i++)
    {
        token_file = std::fopen(argv[i], "r");
        if (token_file == NULL)
        {
            std::cerr << "Error: can not open file " << argv[i] << std::endl;
            std::exit(1);
        }
        curr_lineno = 1;

        cool_yyparse();
        if (parse_errors != 0)
        {
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

        std::unordered_set<Symbol> not_inherited{Bool, Int, String,
                                                 SELF_TYPE};
        std::unordered_set<Symbol> classes_names{Object, Bool, Int,
                                                 String, SELF_TYPE};
        std::unordered_map<std::string, std::string> classes_hierarchy;

        std::vector<std::string> classes;
        std::vector<std::string> parents;
        for (int i = parse_results->first(); parse_results->more(i); i = parse_results->next(i))
        {
            Symbol class_name = parse_results->nth(i)->get_name();
            // проверка корректности имён классов
            auto result = classes_names.insert(class_name);
            if (!result.second)
            {
                std::cerr << "Error! class '" << class_name->get_string() << "' redeclared." << std::endl;
            }
            classes.push_back(class_name->get_string());

            Symbol parent_name = parse_results->nth(i)->get_parent();
            // проверка корректности имён родительских классов
            if (not_inherited.find(parent_name) != not_inherited.end())
            {
                std::cerr << "class '" << class_name->get_string() << "': can't use parent class '"
                          << parent_name->get_string() << "' (builtin)\n";
            }
            // формируем таблицу class <-> parent
            classes_hierarchy[class_name->get_string()] = parent_name->get_string();

            // проверяем методы класса
            Features features = parse_results->nth(i)->get_features();
            std::unordered_set<std::string> features_names;
            for (int j = features->first(); features->more(j); j = features->next(j))
            {
                Feature feature = features->nth(j);
                std::string feature_name = feature->get_name()->get_string();

                if (feature_name == "self")
                {
                    std::cerr << "can't use 'self' as feature name" << std::endl;
                }

                auto res = features_names.insert(feature_name);
                if (!res.second)
                {
                    std::cerr << "feature '" << feature_name << "' in '"
                              << class_name << "' already exists!" << std::endl;
                }

                Symbol type = feature->get_type();
                if (classes_names.find(type) == classes_names.end())
                {
                    std::cerr << "unknown type '" << type << "' in " << feature_name << std::endl;
                }
            }
        }

        std::fclose(token_file);
    }
}
