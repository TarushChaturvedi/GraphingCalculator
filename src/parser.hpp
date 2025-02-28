#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <cmath>
#include <sstream>
#include <iostream> // For debugging

#define DEBUG

class ExpressionParser {
    public:
        inline double parseExpression(const std::string& expression, char variable, double value) {
            std::vector<Token> tokens = tokenize(expression, variable);
            std::cout << "Tokens: " << std::endl;
            for (const auto& token : tokens) {
                std::cout << "Type: " << static_cast<int>(token.type) << ", Value: " << token.value << std::endl;
            }
            return evaluate(tokens, variable, value);
        }

    private:
        enum class TokenType {
            NUMBER,
            VARIABLE,
            OPERATOR,
            LEFT_PAREN,
            RIGHT_PAREN,
            FUNCTION,
            END
        };

        struct Token {
            TokenType type;
            std::string value;
        };

        inline std::vector<Token> tokenize(const std::string& expression, char variable) {
            std::vector<Token> tokens;
            std::stringstream ss(expression);
            char c;

            while (ss.get(c)) {
                if (isspace(c)) continue;

                if (isdigit(c) || c == '.') {
                    // Handle numbers
                    ss.putback(c);
                    double num;
                    ss >> num;
                    tokens.push_back({ TokenType::NUMBER, std::to_string(num) });
                }
                else if (c == variable) {
                    // Handle variables
                    tokens.push_back({ TokenType::VARIABLE, std::string(1, c) });
                }
                else if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^') {
                    // Handle operators
                    tokens.push_back({ TokenType::OPERATOR, std::string(1, c) });
                }
                else if (c == '(') {
                    tokens.push_back({ TokenType::LEFT_PAREN, "(" });
                }
                else if (c == ')') {
                    tokens.push_back({ TokenType::RIGHT_PAREN, ")" });
                }
                else if (isalpha(c)) {
                    // Handle functions (sin, cos, tan, log, etc.)
                    ss.putback(c);
                    std::string funcName;
                    while (isalpha(ss.peek())) {
                        ss.get(c);
                        funcName += c;
                    }
                    tokens.push_back({ TokenType::FUNCTION, funcName });
                }
                else {
                    throw std::runtime_error("Invalid character in expression: " + std::string(1, c));
                }
            }

            // Add implicit multiplication where necessary
            std::vector<Token> processedTokens;
            for (size_t i = 0; i < tokens.size(); ++i) {
                processedTokens.push_back(tokens[i]);

                // Check for implicit multiplication cases:
                // 1. Number followed by a variable or left parenthesis
                // 2. Variable followed by a left parenthesis
                // 3. Right parenthesis followed by a number, variable, or left parenthesis
                // 4. Function followed by a variable or left parenthesis (e.g., sin x or sin(x))
                if (tokens[i].type == TokenType::NUMBER && i + 1 < tokens.size() &&
                    (tokens[i + 1].type == TokenType::VARIABLE || tokens[i + 1].type == TokenType::LEFT_PAREN || tokens[i + 1].type == TokenType::FUNCTION)) {
                    processedTokens.push_back({ TokenType::OPERATOR, "*" });
                }
                else if (tokens[i].type == TokenType::VARIABLE && i + 1 < tokens.size() &&
                    (tokens[i + 1].type == TokenType::LEFT_PAREN || tokens[i + 1].type == TokenType::FUNCTION)) {
                    processedTokens.push_back({ TokenType::OPERATOR, "*" });
                }
                else if (tokens[i].type == TokenType::RIGHT_PAREN && i + 1 < tokens.size() &&
                    (tokens[i + 1].type == TokenType::NUMBER || tokens[i + 1].type == TokenType::VARIABLE || tokens[i + 1].type == TokenType::LEFT_PAREN || tokens[i + 1].type == TokenType::FUNCTION)) {
                    processedTokens.push_back({ TokenType::OPERATOR, "*" });
                }
            }

            processedTokens.push_back({ TokenType::END, "" }); // Add end token
            return processedTokens;
        }

        inline double evaluate(const std::vector<Token>& tokens, char variable, double value) {
            std::vector<double> values;
            std::vector<Token> ops;

            auto precedence = [&](const Token& op) {
                if (op.type == TokenType::FUNCTION) return 4; // Higher precedence for functions
                if (op.type != TokenType::OPERATOR) return 0;
                if (op.value == "+" || op.value == "-") return 1;
                if (op.value == "*" || op.value == "/") return 2;
                if (op.value == "^") return 3; // Higher precedence for exponentiation
                return 0;
            };

            auto applyOp = [&](double b, double a, const Token& op) {
                if (op.value == "+") return a + b;
                if (op.value == "-") return a - b;
                if (op.value == "*") return a * b;
                if (op.value == "/") {
                    if (b == 0) throw std::runtime_error("Division by zero");
                    return a / b;
                }
                if (op.value == "^") return pow(a, b); // Exponentiation
                throw std::runtime_error("Unknown operator: " + op.value);
            };

            auto applyFunc = [&](double a, const Token& func) -> double {
                const std::string& funcName = func.value;
                if (funcName == "sin") return sin(a);
                if (funcName == "cos") return cos(a);
                if (funcName == "tan") return tan(a);
                if (funcName == "asin") return asin(a);
                if (funcName == "acos") return acos(a);
                if (funcName == "atan") return atan(a);
                if (funcName == "log") return log(a);
                if (funcName == "log10") return log10(a);
                if (funcName == "exp") return exp(a);
                if (funcName == "sqrt") return sqrt(a);
                if (funcName == "abs") return std::abs(a); // Use std::abs for double

                throw std::runtime_error("Unknown function: " + funcName);
            };

            for (size_t i = 0; i < tokens.size(); ++i) {
                const Token& token = tokens[i];

                if (token.type == TokenType::NUMBER) {
                    values.push_back(std::stod(token.value));
                }
                else if (token.type == TokenType::VARIABLE) {
                    values.push_back(value);
                }
                else if (token.type == TokenType::LEFT_PAREN) {
                    ops.push_back(token);
                }
                else if (token.type == TokenType::RIGHT_PAREN) {
                    while (!ops.empty() && ops.back().type != TokenType::LEFT_PAREN) {
                        if (ops.back().type == TokenType::FUNCTION) {
                            double a = values.back(); values.pop_back();
                            Token func = ops.back(); ops.pop_back();
                            values.push_back(applyFunc(a, func));
                        }
                        else {
                            double b = values.back(); values.pop_back();
                            double a = values.back(); values.pop_back();
                            Token op = ops.back(); ops.pop_back();
                            values.push_back(applyOp(b, a, op));
                        }
                    }

                    // Remove the left parenthesis
                    if (!ops.empty()) ops.pop_back();
                    else throw std::runtime_error("Unmatched parenthesis");

                }
                else if (token.type == TokenType::OPERATOR) {
                    while (!ops.empty() && ops.back().type != TokenType::LEFT_PAREN && precedence(ops.back()) >= precedence(token)) {
                        double b = values.back(); values.pop_back();
                        double a = values.back(); values.pop_back();
                        Token op = ops.back(); ops.pop_back();
                        values.push_back(applyOp(b, a, op));
                    }
                    ops.push_back(token);
                }
                else if (token.type == TokenType::FUNCTION) {
                    ops.push_back(token);
                }
            }

            // Finish any remaining operations
            while (!ops.empty()) {
                if (ops.back().type == TokenType::FUNCTION) {
                    if (values.empty()) throw std::runtime_error("Missing argument for function");
                    double a = values.back(); values.pop_back();
                    Token func = ops.back(); ops.pop_back();
                    values.push_back(applyFunc(a, func));
                }
                else {
                    if (values.size() < 2) throw std::runtime_error("Missing operands for operator");
                    double b = values.back(); values.pop_back();
                    double a = values.back(); values.pop_back();
                    Token op = ops.back(); ops.pop_back();
                    values.push_back(applyOp(b, a, op));
                }
            }

            // Final check for valid result
            if (values.size() != 1) {
                throw std::runtime_error("Invalid Expression");
            }

            return values.back();
        }
};

