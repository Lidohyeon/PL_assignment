/**
 * File Name: You should name your source file as your student ID (e.g., 20204870.c)
 * Author: Ix Lab TA Dohwa Kim
 * Year: 2025
 *
 * Description:
 * This file is the skeleton code for the PL parser project.
 * Basic code is provided in the main function and some other functions.
 * Students should not modify the main function or the provided functions.
 * Students should add additional functions and variables to complete the parser.
 * It is IMPORTANT to use the provided functions when you need to print information.
 * Detailed descriptions are given below, so please read carefully.
 * Our automated scoring system may give you 0 points if you modify the provided code, and no exceptions will be accepted.
 *
 * Instructions:
 * - Complete the parser project according to the instructions provided in the word file from the class.
 * - You should NOT modify the pre-made code.
 * - You may add standard library headers like string.h or stdlib.h, but do not use custom headers from the internet.
 *   (Standard libraries should be sufficient for this project.)
 *
 * Note:
 * If you have any questions about this project, please email me at: kimdohwa2@cau.ac.kr
 */

#include <stdio.h>
#include <string.h>  // Consider adding string.h for strcmp

/************************************************************
 * ================== HEADER SECTION =====================
 *********Import only Standard header files in this section********/

#include <stdlib.h>
#include <ctype.h>



// ================= HEADER SECTION END =============
//=============================================================

/**
 * @brief Represents an identifier with a name and value.
 *
 * This structure is used to store information about identifiers
 * in the PL parser project. Each identifier has a name and an associated value.
 * The value can be a string or a numeric value depending on the use case.
 * You can add more fields if you want but DO NOT MODIFY predefined fields
 */
typedef struct {
	char name[50];
	char value[100];
}Ident;

/************************************************************
 * ================== CONSTANTS SECTION =====================
 *********Define only constant values in this section********/





// ================= CONSTANT SECTION END =============
//=============================================================




/************************************************************
 * =============== GLOBAL VARIABLES SECTION =================
 *********Declare only global variables here****************/

char token[100];           // 현재 토큰
int token_pos;             // 토큰 위치
int num_ident;             // identifier 개수
char line[1024];
FILE *file;
Ident idArray[256];


// ================= GLOBAL VARIABLES SECTION END =============
//=============================================================

/**
 * @brief Function that prints the line and the number of IDs, constants, and operands.
 *
 * YOU SHOULD NOT MODIFY THIS CODE,
 * or you might risk receiving 0 points.
 *
 * @param line The line that the program has read.
 * @param ID The number of IDs.
 * @param CON The number of constants.
 * @param OP The number of operands.
 * @param 0 to print (OK)
 */
void printResultByLine(char *line, int ID, int CON, int OP) {
    printf("%s\n", line);
    printf("ID: %d; CONST: %d; OP: %d;\n", ID, CON, OP);
}

/**
 * @brief Function that prints the warning message about Operands
 *
 * YOU SHOULD NOT MODIFY THIS CODE,
 * or you might risk receiving 0 points.
 *
 * @param code The warning code of operand
 * 1 : Multiple (+) Operation | 2 : Multiple (-) Operation
 * 3 : Multiple (*) Operation | 4 : Multiple (/) Operation
 * 5 : Wrong Assignment Operation (:=)
 */
void printOPWarning(int code){
	switch(code){
	
    	case 1:
    		printf("(Warning) \"Eliminating duplicate operator (+)\"\n");
    		break;
    	case 2:
    		printf("(Warning) \"Eliminating duplicate operator (-)\"\n");
    		break;
    	case 3:
    		printf("(Warning) \"Eliminating duplicate operator (*)\"\n");
    		break;
    	case 4:
    		printf("(Warning) \"Eliminating duplicate operator (/)\"\n");
    		break;
    	case 5:
    		printf("(Warning) \"Substituting assignment operator (:=)\"\n"); 
    		break;
    	}
}

/**
 * @brief Function that prints OK sign
 *
 */
void printOK(){
	printf("(OK)\n");
}

/**
 * @brief Function that prints the line and the number of IDs, constants, and operands.
 *
 * YOU SHOULD NOT MODIFY THIS CODE,
 * or you might risk receiving 0 points.
 *
 * @param name The name of Identifier that didn't referenced before
 */
void printIDError(char *name){
	printf("(Error) \"referring to undefined identifiers(%s)\"\n",name);
}

/**
 * @brief Function that prints the result of identifiers.
 *
 * YOU SHOULD NOT MODIFY THIS CODE,
 * or you might risk receiving 0 points.
 *
 * Save identifiers in predefined array
 * @param num_ident The number of identifiers.
 * 
 * Result ==> operand1: 2; total: 2;
 */
void printIdent(int num_ident) {
    int i;
    printf("Result ==>");
    for (i = 0; i < num_ident; i++) {
        printf(" %s: %s;", idArray[i].name, idArray[i].value);
    }
}

/**
 * @brief Funtion that prints token by line
 *
 * YOU SHOULD NOT MODIFY THIS CODE,
 * or you might risk receiving 0 points
 *
 * Print token by line
 * This function is used for verbose mode(-v)
 * @param token Token that you want to print
 *
 */
void printToken(char *token){
	printf("%s\n", token);
}

/* ================== FUNCTION ADDITION AREA START ==================
=====================================================================
=================Add your own function definitions below=============
=================Do NOT modify any other parts of the code=========== */

// Forward declarations
void getNextToken();
int parseStatement(int verbose);
int parseExpression(int verbose);
int parseTerm(int verbose);
int parseFactor(int verbose);

// Lexical Analyzer
char *input_ptr;
int id_count, const_count, op_count;
int has_error;
char error_var[100];
int warning_codes[10];  // Store multiple warning codes
int warning_count;
int expr_has_unknown;  // Track if current expression uses unknown values
char reconstructed_line[1024];  // Reconstructed statement line

// Check if string is a number
int isNumber(char *str) {
    if (str == NULL || str[0] == '\0') return 0;
    int i = 0;
    if (str[0] == '-' || str[0] == '+') i = 1;
    if (str[i] == '\0') return 0;
    while (str[i] != '\0') {
        if (!isdigit(str[i])) return 0;
        i++;
    }
    return 1;
}

// Check if string is a valid identifier
int isIdentifier(char *str) {
    if (str == NULL || str[0] == '\0') return 0;
    if (!isalpha(str[0]) && str[0] != '_') return 0;
    for (int i = 1; str[i] != '\0'; i++) {
        if (!isalnum(str[i]) && str[i] != '_') return 0;
    }
    return 1;
}

// Get next token from input
void getNextToken() {
    // Skip whitespace
    while (*input_ptr != '\0' && *input_ptr <= 32) {
        input_ptr++;
    }
    
    if (*input_ptr == '\0') {
        token[0] = '\0';
        return;
    }
    
    int i = 0;
    
    // Check for operators and special characters
    if (*input_ptr == ':' && *(input_ptr + 1) == '=') {
        token[0] = ':';
        token[1] = '=';
        token[2] = '\0';
        input_ptr += 2;
        return;
    }
    
    if (*input_ptr == '+' || *input_ptr == '-' || *input_ptr == '*' || 
        *input_ptr == '/' || *input_ptr == '=' || *input_ptr == ';' || 
        *input_ptr == '(' || *input_ptr == ')') {
        token[0] = *input_ptr;
        token[1] = '\0';
        input_ptr++;
        return;
    }
    
    // Read identifier or number
    while (*input_ptr != '\0' && *input_ptr > 32 && 
           *input_ptr != '+' && *input_ptr != '-' && *input_ptr != '*' && 
           *input_ptr != '/' && *input_ptr != '=' && *input_ptr != ';' && 
           *input_ptr != '(' && *input_ptr != ')' && *input_ptr != ':') {
        token[i++] = *input_ptr++;
    }
    token[i] = '\0';
}

// Find identifier in symbol table
int findIdent(char *name) {
    for (int i = 0; i < num_ident; i++) {
        if (strcmp(idArray[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Get identifier value
int getIdentValue(char *name, int *is_unknown) {
    int idx = findIdent(name);
    if (idx == -1) {
        // Add to symbol table as Unknown
        strcpy(idArray[num_ident].name, name);
        strcpy(idArray[num_ident].value, "Unknown");
        num_ident++;
        *is_unknown = 1;
        return 0;
    }
    if (strcmp(idArray[idx].value, "Unknown") == 0) {
        *is_unknown = 0;  // Already in table, not an error anymore
        return 0;
    }
    *is_unknown = 0;
    return atoi(idArray[idx].value);
}

// Set identifier value
void setIdentValue(char *name, int value, int is_unknown) {
    int idx = findIdent(name);
    if (idx == -1) {
        strcpy(idArray[num_ident].name, name);
        if (is_unknown) {
            strcpy(idArray[num_ident].value, "Unknown");
        } else {
            sprintf(idArray[num_ident].value, "%d", value);
        }
        num_ident++;
    } else {
        if (is_unknown) {
            strcpy(idArray[idx].value, "Unknown");
        } else {
            sprintf(idArray[idx].value, "%d", value);
        }
    }
}

// Parse factor: <ident> | <const> | ( <expression> )
int parseFactor(int verbose) {
    int result = 0;
    int is_unknown = 0;
    
    if (token[0] == '(') {
        if (verbose) printToken("left_paren");
        strcat(reconstructed_line, "(");
        getNextToken();
        result = parseExpression(verbose);
        if (token[0] == ')') {
            if (verbose) printToken("right_paren");
            strcat(reconstructed_line, ")");
            getNextToken();
        }
    } else if (isNumber(token)) {
        if (verbose) printToken("const");
        const_count++;
        strcat(reconstructed_line, token);
        result = atoi(token);
        getNextToken();
    } else if (isIdentifier(token)) {
        if (verbose) printToken("ident");
        id_count++;
        char name[100];
        strcpy(name, token);
        strcat(reconstructed_line, name);
        int idx = findIdent(name);
        if (idx == -1) {
            // First reference - add as Unknown and mark as error
            strcpy(idArray[num_ident].name, name);
            strcpy(idArray[num_ident].value, "Unknown");
            num_ident++;
            has_error = 1;
            strcpy(error_var, name);
            is_unknown = 1;
            expr_has_unknown = 1;
        } else if (strcmp(idArray[idx].value, "Unknown") == 0) {
            // Already in table as Unknown - use it but mark expression as unknown
            is_unknown = 1;
            expr_has_unknown = 1;
        } else {
            result = atoi(idArray[idx].value);
        }
        getNextToken();
    }
    
    if (is_unknown) return 0;
    return result;
}

// Parse term: <factor> { <mult_op> <factor> }
int parseTerm(int verbose) {
    int result = parseFactor(verbose);
    
    while (token[0] == '*' || token[0] == '/') {
        char op = token[0];
        if (verbose) printToken("mult_operator");
        op_count++;
        getNextToken();
        
        // Check for duplicate operators
        if (token[0] == op) {
            if (op == '*') warning_codes[warning_count++] = 3;
            else warning_codes[warning_count++] = 4;
            while (token[0] == op) {
                getNextToken();
            }
        }
        
        char op_str[3] = " ";
        op_str[1] = op;
        strcat(reconstructed_line, op_str);
        strcat(reconstructed_line, " ");
        
        int right = parseFactor(verbose);
        if (op == '*') result *= right;
        else if (op == '/' && right != 0) result /= right;
    }
    
    return result;
}

// Parse expression: <term> { <add_op> <term> }
int parseExpression(int verbose) {
    int result = parseTerm(verbose);
    
    while (token[0] == '+' || token[0] == '-') {
        char op = token[0];
        if (verbose) printToken("add_operator");
        op_count++;
        getNextToken();
        
        // Check for duplicate operators
        if (token[0] == op) {
            if (op == '+') warning_codes[warning_count++] = 1;
            else warning_codes[warning_count++] = 2;
            while (token[0] == op) {
                getNextToken();
            }
        }
        
        char op_str[3] = " ";
        op_str[1] = op;
        strcat(reconstructed_line, op_str);
        strcat(reconstructed_line, " ");
        
        int right = parseTerm(verbose);
        if (op == '+') result += right;
        else result -= right;
    }
    
    return result;
}

// Parse statement: <ident> <assignment_op> <expression>
int parseStatement(int verbose) {
    id_count = 0;
    const_count = 0;
    op_count = 0;
    has_error = 0;
    error_var[0] = '\0';
    warning_count = 0;
    expr_has_unknown = 0;  // Reset for each statement
    reconstructed_line[0] = '\0';  // Reset reconstructed line
    
    if (!isIdentifier(token)) return 0;
    
    char var_name[100];
    strcpy(var_name, token);
    strcpy(reconstructed_line, var_name);
    if (verbose) printToken("ident");
    id_count++;
    getNextToken();
    
    // Check assignment operator
    if (token[0] == '=') {
        warning_codes[warning_count++] = 5;
        strcat(reconstructed_line, " := ");
        if (verbose) printToken("assignment_op");
        getNextToken();
    } else if (strcmp(token, ":=") == 0) {
        strcat(reconstructed_line, " := ");
        if (verbose) printToken("assignment_op");
        getNextToken();
    }
    
    // Add left-hand side variable to symbol table FIRST (to preserve order)
    // This ensures the assigned variable appears before any undefined vars in expression
    int lhs_idx = findIdent(var_name);
    if (lhs_idx == -1) {
        // Reserve a spot in symbol table for this variable
        strcpy(idArray[num_ident].name, var_name);
        strcpy(idArray[num_ident].value, "0");  // Temporary placeholder
        lhs_idx = num_ident;
        num_ident++;
    }
    
    int result = parseExpression(verbose);
    
    if (has_error) {
        strcpy(idArray[lhs_idx].value, "Unknown");
    } else if (expr_has_unknown) {
        // If expression used unknown values, result is unknown
        strcpy(idArray[lhs_idx].value, "Unknown");
    } else {
        sprintf(idArray[lhs_idx].value, "%d", result);
    }
    
    return 1;
}

/**
 * @brief This function is the starting point of this project with no option -v (print option a).
 *
 * You can freely modify this code or add more functions.
 * However, you SHOULD use the print functions below when you need to print some lines on screen,
 * or you might risk receiving 0 points even if the program works perfectly.
 */
void parse() {
    num_ident = 0;
    
    while (line[0] != '\0') {
        // Remove newline
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') {
            if (fgets(line, sizeof(line), file) == NULL) break;
            continue;
        }
        
        // Process each statement in the line
        input_ptr = line;
        getNextToken();
        
        while (token[0] != '\0') {
            char statement_line[1024];
            
            // Parse the statement (this will build reconstructed_line)
            parseStatement(0);
            
            // Check if original line had semicolon
            int has_semicolon = 0;
            if (token[0] == ';') {
                has_semicolon = 1;
            } else {
                // Look for semicolon in remaining input
                char *semi_pos = strchr(input_ptr, ';');
                char *next_ident = input_ptr;
                while (*next_ident && *next_ident <= 32) next_ident++;
                if (semi_pos && (!isalpha(*next_ident) || semi_pos < next_ident)) {
                    has_semicolon = 1;
                }
            }
            
            // Build final statement line
            strcpy(statement_line, reconstructed_line);
            if (has_semicolon) {
                strcat(statement_line, ";");
            }
            
            // Print results
            printResultByLine(statement_line, id_count, const_count, op_count);
            for (int w = 0; w < warning_count; w++) {
                printOPWarning(warning_codes[w]);
            }
            if (has_error) {
                printIDError(error_var);
            }
            if (!has_error && warning_count == 0) {
                printOK();
            }
            
            // Skip semicolon
            if (token[0] == ';') {
                getNextToken();
            }
            
            // If no more tokens, break
            if (token[0] == '\0') break;
        }
        
        // Read next line
        if (fgets(line, sizeof(line), file) == NULL) break;
    }
    
    // Print final results
    printIdent(num_ident);
    printf("\n");
}

/**
 * @brief This function is the starting point of this project with option -v (print option b).
 *
 * You can freely modify this code or add more functions.
 * However, you SHOULD use the print functions below when you need to print some lines on screen,
 * or you might risk receiving 0 points even if the program works perfectly.
 */
void parse_V() {
    num_ident = 0;
    
    while (line[0] != '\0') {
        // Remove newline
        line[strcspn(line, "\n")] = '\0';
        if (line[0] == '\0') {
            if (fgets(line, sizeof(line), file) == NULL) break;
            continue;
        }
        
        // Process tokens
        input_ptr = line;
        getNextToken();
        
        while (token[0] != '\0') {
            parseStatement(1);
            
            // Skip semicolon
            if (token[0] == ';') {
                printToken("semi_colon");
                getNextToken();
            }
            
            if (token[0] == '\0') break;
        }
        
        // Read next line
        if (fgets(line, sizeof(line), file) == NULL) break;
    }
}


/* ================== FUNCTION ADDITION AREA END ==================== 
=====================================================================
=====================================================================*/


/**
 *
 * @brief Main function for processing a file.
 *
 */
int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-v] <filepath>\n", argv[0]);
        return 1;
    }

    int verbose = 0;
    char *filepath = NULL;

    if (strcmp("-v", argv[1]) == 0) {
        verbose = 1;
        if (argc < 3) {
            fprintf(stderr, "Error: No file specified.\n");
            return 1;
        }
        filepath = argv[2];
    } else {
        filepath = argv[1];
    }

    file = fopen(filepath, "r");

    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file %s\n", filepath);
        return 1;
    }

    // Gets input line for the first time
    fgets(line, sizeof(line), file);

    // Depending on the verbose flag, call the appropriate function
    if (verbose) {
        parse_V();
    } else {
        parse();
    }

    fclose(file);
    return 0;
}
