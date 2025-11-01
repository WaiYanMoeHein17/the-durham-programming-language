#ifndef MAIN_HPP
#define MAIN_HPP

#include <iostream> 
#include <string> 
#include <optional> 
#include <vector> 
#include <map> 
#include <sstream> 
#include <fstream>
#include <cctype>
#include "tokenizer.h"
#include "parser.h"
#include "gen_asm.h"

struct User {
    std::optional<std::string> name; 
    std::optional<std::string> college; 
    std::optional<std::string> course;
    std::optional<char> gender; 
    std::optional<int> age; 
}; 

struct Settings {
    bool jokes = true; 
    bool auto_correct = true;
    std::string college_one;
}; 

void ask_user_info(); 
void make_joke(User user); // make jokes depending on the user info 
void autocorrect(int argc, int **argv); // automatically corrects code in .dur file

#endif //MAIN_HPP