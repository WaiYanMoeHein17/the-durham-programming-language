#include "tokenizer.h"
#include <algorithm>
#include <limits>

// Constructor
Tokenizer::Tokenizer(const std::string& source) : src(source), index(0), corrections_made(false) {}

// Destructor
Tokenizer::~Tokenizer() {}

// Calculate Levenshtein distance between two strings
int Tokenizer::levenshteinDistance(const std::string& s1, const std::string& s2) {
    const size_t len1 = s1.size(), len2 = s2.size();
    std::vector<std::vector<int>> d(len1 + 1, std::vector<int>(len2 + 1));

    for (size_t i = 0; i <= len1; ++i) d[i][0] = static_cast<int>(i);
    for (size_t j = 0; j <= len2; ++j) d[0][j] = static_cast<int>(j);

    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            d[i][j] = std::min({d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + cost});
        }
    }
    return d[len1][len2];
}

// Suggest a correction for an unknown word
std::optional<std::string> Tokenizer::suggestCorrection(const std::string& word) {
    static const std::vector<std::string> keywords = {
        "tlc", "mcs", "for", "if", "while", "new", "college", "at",
        "durham", "newcastle", "york", "edinburgh",
        "and", "or", "not", "greater", "lesser", "equals",
        "is", "begin", "end", "front", "back"
    };
    
    // Special case for 2-letter words - only suggest if very close to 3-letter keyword
    if (word.length() == 2) {
        for (const auto& keyword : keywords) {
            if (keyword.length() == 3 && levenshteinDistance(word, keyword) == 1) {
                return keyword;
            }
        }
        return std::nullopt;
    }
    
    // Don't suggest corrections for single letter words (definitely variable names)
    if (word.length() <= 1) {
        return std::nullopt;
    }
    
    int minDist = std::numeric_limits<int>::max();
    std::string bestMatch;
    
    for (const auto& keyword : keywords) {
        int dist = levenshteinDistance(word, keyword);
        // Only suggest if distance is 1 or 2 (typo or missing/extra char)
        // AND the word length is similar (within 2 chars)
        if (dist > 0 && dist <= 2 && dist < minDist && 
            abs((int)word.length() - (int)keyword.length()) <= 2) {
            minDist = dist;
            bestMatch = keyword;
        }
    }
    
    if (!bestMatch.empty()) {
        return bestMatch;
    }
    return std::nullopt;
}

// Prompt user for correction and apply to source
bool Tokenizer::promptUserForCorrection(const std::string& original, const std::string& suggestion, int position) {
    std::cout << "\nUnknown token: '" << original << "'" << std::endl;
    std::cout << "Did you mean: '" << suggestion << "'? (y/n): ";
    
    std::string response;
    std::getline(std::cin, response);
    
    if (!response.empty() && (response[0] == 'y' || response[0] == 'Y')) {
        // Replace in source string
        int start_pos = position - original.length();
        if (start_pos >= 0 && start_pos + original.length() <= src.length()) {
            src.replace(start_pos, original.length(), suggestion);
            // Adjust index for the length difference
            index = index - original.length() + suggestion.length();
            corrections_made = true;
        }
        return true;
    }
    return false;
}

std::optional<char> Tokenizer::peek(int offset) {
    if (index + offset >= src.length()) {
        return {}; 
    }
    return src.at(index + offset);
}

char Tokenizer::consume() {
    return src.at(index++); 
}

// Returns the decimal value as a string for colleges representing 0-16
std::optional<std::string> college_to_decimal(const std::string& college) {
    static const std::map<std::string, std::string> lookup = {
        {"butler", "0"},
        {"chads", "1"},
        {"marys", "2"},
        {"collingwood", "3"},
        {"johns", "4"},
        {"castle", "5"},
        {"cuths", "6"},
        {"trevs", "7"},
        {"aidans", "8"},
        {"snow", "9"},
        {"grey", "10"},
        {"stephenson", "11"},
        {"hatfield", "12"},
        {"hildbede", "13"},
        {"south", "14"},
        {"vanmildert", "15"},
        {"ustinov", "16"}
    };
    
    auto it = lookup.find(college);
    if (it != lookup.end()) return it->second;
    return std::nullopt;
}

// Legacy function for backwards compatibility - converts to single char for 0-9
std::optional<char> college_to_digit(const std::string& college) {
    auto decimal = college_to_decimal(college);
    if (decimal.has_value() && decimal.value().length() == 1) {
        return decimal.value()[0];
    }
    return std::nullopt;
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    
    while (peek().has_value()) {
        // Skip whitespace
        if (std::isspace(peek().value())) {
            consume();
            continue;
        }
        // Single character tokens
        if (peek().value() == '.') {
            consume();
            tokens.push_back({TokenType::semi});  // Changed: '.' is semicolon equivalent
            continue;
        }
        
        // String literals and comments with quotation marks
        if (peek().value() == '"') {
            // Check if the previous token was 'begin' (for strings: begin "..." end)
            bool is_string_context = !tokens.empty() && tokens.back().type == TokenType::open_paren;
            
            consume(); // consume opening quote
            std::string str_value;
            
            while (peek().has_value() && peek().value() != '"') {
                str_value += consume();
            }
            
            if (peek().has_value() && peek().value() == '"') {
                consume(); // consume closing quote
                
                // String literal: begin "text" end (for print or assignment)
                if (is_string_context) {
                    tokens.push_back({TokenType::quotations, str_value});
                }
                // Otherwise, it's a comment - skip it entirely
            } else {
                std::cerr << "Error: Unterminated string/comment" << std::endl;
            }
            continue;
        }
        
        // Alphabetic tokens (keywords, identifiers, college names)
        if (std::isalpha(peek().value())) {
            std::string buffer;
            
            // Consume all alphabetic characters
            while (peek().has_value() && std::isalpha(peek().value())) {
                buffer += consume();
            }
            
            // Check if it's a college name (digit)
            auto decimal_value = college_to_decimal(buffer);
            if (decimal_value.has_value()) {
                std::string number_string = decimal_value.value();
                
                // Handle multi-digit numbers with ','
                while (peek().has_value() && peek().value() == ',') {
                    consume(); // consume comma
                    
                    buffer.clear();
                    while (peek().has_value() && std::isalpha(peek().value())) {
                        buffer += consume();
                    }
                    
                    decimal_value = college_to_decimal(buffer);
                    if (decimal_value.has_value()) {
                        number_string += decimal_value.value();
                    } else {
                        std::cerr << "Error: Unknown college name '" << buffer << "'" << std::endl;
                        break;
                    }
                }
                
                tokens.push_back({TokenType::int_lit, number_string});
            } 
            // Check for keywords
            else if (buffer == "tlc") {
                tokens.push_back({TokenType::_tlc});
            } else if (buffer == "mcs") {
                tokens.push_back({TokenType::_mcs});
            } else if (buffer == "for") {
                tokens.push_back({TokenType::_for});
            } else if (buffer == "if") {
                tokens.push_back({TokenType::_if});
            } else if (buffer == "else") {
                tokens.push_back({TokenType::_else});
            } else if (buffer == "while") {
                tokens.push_back({TokenType::_while});
            } else if (buffer == "function") {
                tokens.push_back({TokenType::_function});
            // Type keywords
            } else if (buffer == "text") {
                tokens.push_back({TokenType::_text});
            } else if (buffer == "number") {
                tokens.push_back({TokenType::_number});
            // Vector/Array keywords
            } else if (buffer == "new") {
                tokens.push_back({TokenType::_new});
            } else if (buffer == "college") {
                tokens.push_back({TokenType::_college});
            } else if (buffer == "at") {
                tokens.push_back({TokenType::_at});
            // Arithmetic operators
            } else if (buffer == "durham") {
                tokens.push_back({TokenType::_durham});
            } else if (buffer == "newcastle") {
                tokens.push_back({TokenType::_newcastle});
            } else if (buffer == "york") {
                tokens.push_back({TokenType::_york});
            } else if (buffer == "edinburgh") {
                tokens.push_back({TokenType::_edinburgh});
            // Logical operators
            } else if (buffer == "and") {
                tokens.push_back({TokenType::_and});
            } else if (buffer == "or") {
                tokens.push_back({TokenType::_or});
            } else if (buffer == "not") {
                tokens.push_back({TokenType::_not});
            // Comparison operators
            } else if (buffer == "greater") {
                tokens.push_back({TokenType::_greater});
            } else if (buffer == "lesser") {
                tokens.push_back({TokenType::_lesser});
            } else if (buffer == "equals") {
                tokens.push_back({TokenType::_equals});
            } else if (buffer == "not") {
                tokens.push_back({TokenType::_not});
            } else if (buffer == "is") {
                tokens.push_back({TokenType::equals});  // "is" keyword for assignment
            } else if (buffer == "begin") {
                tokens.push_back({TokenType::open_paren}); 
            } else if (buffer == "end") {
                tokens.push_back({TokenType::close_paren}); 
            } else if (buffer == "front") {
                tokens.push_back({TokenType::open_brace}); 
            } else if (buffer == "back") {
                tokens.push_back({TokenType::close_brace}); 
            } else {
                // Check if this might be a typo
                auto suggestion = suggestCorrection(buffer);
                
                if (suggestion.has_value()) {
                    if (promptUserForCorrection(buffer, suggestion.value(), index)) {
                        // User accepted correction - re-tokenize the corrected word
                        std::string corrected = suggestion.value();
                        
                        // Map corrected keywords to tokens
                        if (corrected == "tlc") {
                            tokens.push_back({TokenType::_tlc});
                        } else if (corrected == "mcs") {
                            tokens.push_back({TokenType::_mcs});
                        } else if (corrected == "for") {
                            tokens.push_back({TokenType::_for});
                        } else if (corrected == "if") {
                            tokens.push_back({TokenType::_if});
                        } else if (corrected == "while") {
                            tokens.push_back({TokenType::_while});
                        } else if (corrected == "new") {
                            tokens.push_back({TokenType::_new});
                        } else if (corrected == "college") {
                            tokens.push_back({TokenType::_college});
                        } else if (corrected == "at") {
                            tokens.push_back({TokenType::_at});
                        } else if (corrected == "durham") {
                            tokens.push_back({TokenType::_durham});
                        } else if (corrected == "newcastle") {
                            tokens.push_back({TokenType::_newcastle});
                        } else if (corrected == "york") {
                            tokens.push_back({TokenType::_york});
                        } else if (corrected == "edinburgh") {
                            tokens.push_back({TokenType::_edinburgh});
                        } else if (corrected == "and") {
                            tokens.push_back({TokenType::_and});
                        } else if (corrected == "or") {
                            tokens.push_back({TokenType::_or});
                        } else if (corrected == "not") {
                            tokens.push_back({TokenType::_not});
                        } else if (corrected == "greater") {
                            tokens.push_back({TokenType::_greater});
                        } else if (corrected == "lesser") {
                            tokens.push_back({TokenType::_lesser});
                        } else if (corrected == "equals") {
                            tokens.push_back({TokenType::_equals});
                        } else if (corrected == "is") {
                            tokens.push_back({TokenType::equals});
                        } else if (corrected == "begin") {
                            tokens.push_back({TokenType::open_paren});
                        } else if (corrected == "end") {
                            tokens.push_back({TokenType::close_paren});
                        } else if (corrected == "front") {
                            tokens.push_back({TokenType::open_brace});
                        } else if (corrected == "back") {
                            tokens.push_back({TokenType::close_brace});
                        }
                        
                        std::cout << "Correction applied in memory. Continuing compilation..." << std::endl;
                    } else {
                        // User rejected - treat as identifier
                        tokens.push_back({TokenType::identifier, buffer});
                    }
                } else {
                    // No suggestion found - treat as identifier (variable name)
                    tokens.push_back({TokenType::identifier, buffer});
                }
            }
            
            continue;
        }
        
        // Unknown character - skip it
        std::cerr << "Warning: Unknown character '" << peek().value() << "'" << std::endl;
        consume();
    }
    
    // Post-processing: combine consecutive tokens
    std::vector<Token> processed_tokens;
    for (size_t i = 0; i < tokens.size(); i++) {
        if (i + 1 < tokens.size() && 
            tokens[i].type == TokenType::_not && 
            tokens[i + 1].type == TokenType::_equals) {
            // Combine into not_equals
            processed_tokens.push_back({TokenType::_not_equals});
            i++; // Skip the next token since we combined them
        } else {
            processed_tokens.push_back(tokens[i]);
        }
    }
    
    return processed_tokens;
}

