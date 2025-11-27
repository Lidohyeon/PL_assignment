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
#include <string.h> // Consider adding string.h for strcmp

/************************************************************
 * ================== HEADER SECTION =====================
 *********Import only Standard header files in this section********/
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

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
typedef struct
{
    char name[50];
    char value[100];
} Ident;

/************************************************************
 * ================== CONSTANTS SECTION =====================
 *********Define only constant values in this section********/
#define ASSIGNMENT1 99
#define ASSIGNMENT2 100
#define SEMI_COLON 101
#define SUB_OP 102
#define ADD_OP 103
#define MULTI_OP 104
#define DEVISION_OP 105
#define LEFT_PAREN 106
#define RIGHT_PAREN 107
#define IDENTIFIER 108
#define CONST 109

typedef struct
{
    int symbol_type;
    char id_name[100];
    int const_value;
} Symbol;

// ================= CONSTANT SECTION END =============
//=============================================================

/************************************************************
 * =============== GLOBAL VARIABLES SECTION =================
 *********Declare only global variables here****************/
Symbol symbolArray[256];
int symbol_pos[256];
int symbol_len[256];
int symbol_count;
int symbol_current;
char lexeme[100];
int lexLen;

int line_curr;

int idArray_count;

int statementIdCount;
int statementOpCount;
int statementConstCount;

bool error_occured;
int error_count;

int opWarningCode[10];
int opWarnigCode;
int opWarningCount;

char *errorIdName;

// FactorTail termTail;

// ================= GLOBAL VARIABLES SECTION END =============
//=============================================================
char line[1024];
FILE *file;
Ident idArray[256];
bool undefinedHandled[256];

char current_statement[1024];

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
void printResultByLine(char *line, int ID, int CON, int OP)
{
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
void printOPWarning(int code)
{
    switch (code)
    {

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
void printOK()
{

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
void printIDError(char *name)
{
    printf("(Error) \"referring to undefined identifiers(%s)\"\n", name);
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
void printIdent(int num_ident)
{
    int i;
    printf("Result ==>");
    for (i = 0; i < num_ident; i++)
    {
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
void printToken(char *token)
{
    printf("%s\n", token);
}

/* ================== FUNCTION ADDITION AREA START ==================
=====================================================================
=================Add your own function definitions below=============
=================Do NOT modify any other parts of the code=========== */

/*===========================Recursive Grammar Fuction Part ===========================
 */

/*=================================================================================
 */
void parseProgram();
void parseStatements();
void parseStatement();
int parseExpression();
int parseTerm();
// FactorTail parseFactorTail();
int parseFactor();

void removeDuplicateOperators(char *input_line);

void substituteAssignmentOperator(char *input_line);

Symbol *getCurrentToken()
{
    return &symbolArray[symbol_current];
}
void moveToNextToken()
{
    symbol_current++;
}

char getChar()
{
    return line[line_curr++];
}
char getNextChar()
{
    return line[(line_curr + 1)];
}

int lookup(char ch)
{

    switch (ch)
    {
    case ':':
        return ASSIGNMENT1;
        break;
    case '=':
        return ASSIGNMENT2;
        break;
    case ';':
        return SEMI_COLON;
        break;
    case '-':
        return SUB_OP;
        break;
    case '/':
        return DEVISION_OP;
        break;

    case '+':
        return ADD_OP;
        break;
    case '*':
        return MULTI_OP;
        break;
    case '(':
        return LEFT_PAREN;
        break;
    case ')':
        return RIGHT_PAREN;
        break;

    default:
        return 0;
        break;
    }
}

Ident *isExistId(Symbol symbol) // id 중에 있는지 확인 및 그 id반환
{
    if (symbol.symbol_type == IDENTIFIER)
    {
        for (int i = 0; i < idArray_count; i++)
        {
            if (strcmp(idArray[i].name, symbol.id_name) == 0)
            {
                return &idArray[i];
            }
        }
        return NULL;
    }
    return NULL;
}

int getIdIndex(const char *name)
{
    for (int i = 0; i < idArray_count; i++)
    {
        if (strcmp(idArray[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

void createIdArray()
{
    for (int i = 0; i < symbol_count; i++)
    {
        if (symbolArray[i].symbol_type == IDENTIFIER)
        {
            // 중복 체크: 이미 존재하지 않는 경우에만 추가
            if (isExistId(symbolArray[i]) == NULL)
            {
                strcpy(idArray[idArray_count].name, symbolArray[i].id_name);
                strcpy(idArray[idArray_count].value, "Unknown"); // 초기값 설정
                undefinedHandled[idArray_count] = false;
                idArray_count++;
            }
        }
    }
}

void lexical_analysis()
{
    /* Initialize indices */
    line_curr = 0;
    symbol_count = 0;
    symbol_current = 0;

    /* Read first character using getChar to set up proper consumption */
    char ch = getChar();

    while (ch != '\0' && ch != '\n' && ch != EOF)
    {
        /* Skip whitespace */
        if (isspace((unsigned char)ch))
        {
            ch = getChar();
            continue;
        }

        int token_start = line_curr - 1;

        /* Operators / single-character tokens */
        if (lookup(ch) != 0)
        {
            int sym = lookup(ch);
            int token_length = 1;
            /* If assignment operator ":=", lookup checked next char; consume it here */
            if (ch == ':' && getNextChar() == '=')
            {
                /* consume '=' so next getChar progresses past it */
                getChar();
                token_length = 2;
            }
            else if (ch == '=')
            {
                lookup(ch);
            }
            symbol_pos[symbol_count] = token_start;
            symbol_len[symbol_count] = token_length;
            symbolArray[symbol_count++].symbol_type = sym;
            ch = getChar();
            continue;
        }
        else if (isalpha((unsigned char)ch))
        {
            lexLen = 0;
            lexeme[lexLen++] = ch;

            char next_ch = getChar();

            while (isdigit(next_ch) || isalpha(next_ch))
            {
                lexeme[lexLen++] = next_ch;
                next_ch = getChar();

                if (lexLen >= (int)sizeof(lexeme) - 1)
                    break; /* avoid overflow */
            }
            lexeme[lexLen] = '\0'; /* 널 종료 문자 추가 */

            char lex_id_name[100];
            strcpy(lex_id_name, lexeme);
            memset(lexeme, 0, sizeof(lexeme));

            symbol_pos[symbol_count] = token_start;
            symbol_len[symbol_count] = lexLen;
            symbolArray[symbol_count].symbol_type = IDENTIFIER;
            strcpy(symbolArray[symbol_count++].id_name, lex_id_name);

            ch = next_ch;
            continue;
        }
        else if (isdigit((unsigned char)ch))
        {
            lexLen = 0;
            lexeme[lexLen++] = ch;

            char next_ch = getChar();

            while (isdigit(next_ch))
            {
                lexeme[lexLen++] = next_ch;
                next_ch = getChar();
                if (lexLen >= (int)sizeof(lexeme) - 1)
                    break;
            }
            lexeme[lexLen] = '\0';

            int value = (int)strtol(lexeme, NULL, 10);

            /* store symbol */
            symbol_pos[symbol_count] = token_start;
            symbol_len[symbol_count] = lexLen;
            symbolArray[symbol_count].symbol_type = CONST;
            symbolArray[symbol_count].const_value = value;
            symbol_count++;

            ch = next_ch;
            continue;
        }

        /* Any other character: skip it */
        ch = getChar();
    }
    createIdArray(); // createdIdArray 를 어디다 두어야할지 고민해야함.!!!!!!!!!!!!!!!!!!
}

void parseProgram()
{
    idArray_count = 0;
    for (int i = 0; i < 256; i++)
    {
        undefinedHandled[i] = false;
    }
    parseStatements();
    printIdent(idArray_count);
}

int statement_start_pos = 0;
int statement_end_pos = 0;

void findStatementPosition(char *original_line, int start_token, int end_token)
{
    // 원본 라인에서 현재 statement의 시작과 끝 위치를 찾기
    int token_count = 0;
    int pos = 0;
    int len = strlen(original_line);

    statement_start_pos = 0;
    statement_end_pos = len - 1;

    // 첫 번째 토큰의 시작 위치 찾기
    while (pos < len && isspace(original_line[pos]))
    {
        pos++;
    }

    if (start_token > 0)
    {
        // start_token까지 건너뛰기
        int current_token = 0;
        while (pos < len && current_token < start_token)
        {
            // 공백 건너뛰기
            while (pos < len && isspace(original_line[pos]))
            {
                pos++;
            }

            if (pos >= len)
                break;

            // 현재 토큰 건너뛰기
            if (isalpha(original_line[pos]))
            {
                while (pos < len && (isalnum(original_line[pos])))
                {
                    pos++;
                }
            }
            else if (isdigit(original_line[pos]))
            {
                while (pos < len && isdigit(original_line[pos]))
                {
                    pos++;
                }
            }
            else if (original_line[pos] == ':' && pos + 1 < len && original_line[pos + 1] == '=')
            {
                pos += 2;
            }
            else
            {
                pos++;
            }
            current_token++;
        }

        // 공백 건너뛰고 시작 위치 설정
        while (pos < len && isspace(original_line[pos]))
        {
            pos++;
        }
        statement_start_pos = pos;
    }

    // 세미콜론이나 다음 statement까지의 위치 찾기
    pos = statement_start_pos;
    int paren_count = 0;
    bool found_semicolon = false;

    while (pos < len)
    {
        if (original_line[pos] == '(')
        {
            paren_count++;
        }
        else if (original_line[pos] == ')')
        {
            paren_count--;
        }
        else if (original_line[pos] == ';' && paren_count == 0)
        {
            statement_end_pos = pos; // 세미콜론을 포함하도록 수정
            found_semicolon = true;
            break;
        }
        pos++;
    }

    if (!found_semicolon)
    {
        statement_end_pos = len - 1;
        while (statement_end_pos > statement_start_pos && isspace(original_line[statement_end_pos]))
        {
            statement_end_pos--;
        }
    }
}

void extractCurrentStatement(char *original_line, int start_token, int end_token)
{
    if (start_token < 0 || end_token < start_token || end_token >= symbol_count)
    {
        current_statement[0] = '\0';
        return;
    }

    int start_pos = symbol_pos[start_token];
    int end_pos = symbol_pos[end_token] + symbol_len[end_token] - 1;

    // 안전 장치
    if (start_pos < 0)
        start_pos = 0;
    int line_len = strlen(original_line);
    if (end_pos >= line_len)
        end_pos = line_len - 1;

    // 앞뒤 공백 제거
    while (start_pos <= end_pos && isspace((unsigned char)original_line[start_pos]))
    {
        start_pos++;
    }
    while (end_pos >= start_pos && isspace((unsigned char)original_line[end_pos]))
    {
        end_pos--;
    }

    int statement_len = end_pos - start_pos + 1;
    if (statement_len > 0 && statement_len < 1024)
    {
        strncpy(current_statement, original_line + start_pos, statement_len);
        current_statement[statement_len] = '\0';
    }
    else
    {
        current_statement[0] = '\0';
    }
}

// Remove whitespace that appears immediately before a semicolon to match
// the expected output format (e.g., "a := 3 ;" -> "a := 3;").
void trimSpaceBeforeSemicolon(char *stmt)
{
    int len = strlen(stmt);
    int write = 0;

    for (int read = 0; read < len; read++)
    {
        if (stmt[read] == ';')
        {
            // 뒤쪽 공백은 유지하지만 세미콜론 바로 앞 공백은 제거
            while (write > 0 && isspace((unsigned char)stmt[write - 1]))
            {
                write--;
            }
            stmt[write++] = ';';
        }
        else
        {
            stmt[write++] = stmt[read];
        }
    }

    stmt[write] = '\0';

    // 마지막에 남아있는 불필요한 공백 제거
    while (write > 0 && isspace((unsigned char)stmt[write - 1]))
    {
        stmt[--write] = '\0';
    }
}

void checkMoreStatements()
{
    if (symbol_current < symbol_count && getCurrentToken()->symbol_type == SEMI_COLON)
    {
        // 세미콜론을 건너뛰고 다음 토큰으로 이동
        moveToNextToken();

        // 아직 현재 줄에 처리할 토큰이 남아있는지 확인
        if (symbol_current < symbol_count)
        {
            int statement_start = symbol_current;

            // 같은 줄에서 다음 statement 처리
            parseStatement();

            // 다음 세미콜론이나 줄 끝까지 찾기
            int statement_end = symbol_current - 1; // 현재 파싱이 끝난 지점의 이전까지
            if (symbol_current < symbol_count && getCurrentToken()->symbol_type == SEMI_COLON)
            {
                statement_end = symbol_current; // 세미콜론 포함
            }
            else
            {
                statement_end = symbol_count - 1; // 마지막 토큰까지
            }

            // 토큰 위치를 기반으로 현재 statement를 추출
            extractCurrentStatement(line, statement_start, statement_end);

            // 출력 형식에 맞게 세미콜론 앞의 공백을 제거
            trimSpaceBeforeSemicolon(current_statement);

            // 중복 연산자가 있는 경우 current_statement에서 제거
            for (int i = 0; i < opWarningCount; i++)
            {
                if (opWarningCode[i] >= 1 && opWarningCode[i] <= 4)
                {
                    removeDuplicateOperators(current_statement);
                    break; // 한 번만 실행
                }
            }

            // assignment operator 치환이 필요한 경우
            for (int i = 0; i < opWarningCount; i++)
            {
                if (opWarningCode[i] == 5)
                {
                    substituteAssignmentOperator(current_statement);
                    break; // 한 번만 실행
                }
            }

            // 출력 형식에 맞게 세미콜론 앞의 공백을 제거
            trimSpaceBeforeSemicolon(current_statement);

            printResultByLine(current_statement, statementIdCount, statementConstCount, statementOpCount);

            if (error_occured == false && opWarningCount == 0)
            {
                printOK();
            }
            else if (error_occured)
            {
                printIDError(errorIdName);
            }
            else if (opWarningCount > 0)
            {
                // 순서대로 경고 메시지 출력
                for (int i = 0; i < opWarningCount; i++)
                {
                    printOPWarning(opWarningCode[i]);
                }
            }

            // 재귀적으로 더 많은 statement가 있는지 확인
            checkMoreStatements();
        }
        return;
    }
    else
    {
        return;
    }
}

void parseStatements()
{
    // 현재 줄이 비어있거나 파일이 끝났으면 종료
    if (strlen(line) == 0 || line[0] == '\0' || line[0] == '\n')
    {
        return;
    }

    // 줄 끝의 개행 문자 제거
    int len = strlen(line);
    if (len > 0 && line[len - 1] == '\n')
    {
        line[len - 1] = '\0';
    }

    lexical_analysis();

    // 심볼이 없으면 종료
    if (symbol_count == 0)
    {
        return;
    }

    int statement_start = 0;
    parseStatement();

    // 첫 번째 세미콜론이나 줄 끝까지 찾기
    int statement_end = symbol_current;
    if (symbol_current < symbol_count && getCurrentToken()->symbol_type == SEMI_COLON)
    {
        statement_end = symbol_current;
    }
    else
    {
        statement_end = symbol_count - 1;
    }

    // 현재 statement 추출
    extractCurrentStatement(line, statement_start, statement_end);

    // 출력 형식에 맞게 세미콜론 앞의 공백을 제거
    trimSpaceBeforeSemicolon(current_statement);

            // 중복 연산자가 있는 경우 current_statement에서 제거
            for (int i = 0; i < opWarningCount; i++)
            {
                if (opWarningCode[i] >= 1 && opWarningCode[i] <= 4)
                {
                    removeDuplicateOperators(current_statement);
                    break; // 한 번만 실행
                }
            }

            // assignment operator 치환이 필요한 경우
            for (int i = 0; i < opWarningCount; i++)
            {
                if (opWarningCode[i] == 5)
                {
                    substituteAssignmentOperator(current_statement);
                    break; // 한 번만 실행
                }
            }

            // 출력 형식에 맞게 세미콜론 앞의 공백을 제거
            trimSpaceBeforeSemicolon(current_statement);

    printResultByLine(current_statement, statementIdCount, statementConstCount, statementOpCount);

    if (error_occured == false && opWarningCount == 0)
    {
        printOK();
        checkMoreStatements();
        if (fgets(line, sizeof(line), file) != NULL)
        {
        parseStatements();
    }
}
    else if (error_occured)
    {
        printIDError(errorIdName);
        checkMoreStatements();
        if (fgets(line, sizeof(line), file) != NULL)
        {
            parseStatements();
        }
    }
    else if (opWarningCount > 0)
    {
        // 순서대로 경고 메시지 출력
        for (int i = 0; i < opWarningCount; i++)
        {
            printOPWarning(opWarningCode[i]);
        }
        checkMoreStatements();
        if (fgets(line, sizeof(line), file) != NULL)
        {
            parseStatements();
        }
    }
    else
    {
        checkMoreStatements();
        if (fgets(line, sizeof(line), file) != NULL)
        {
            parseStatements();
        }
    }
}

void parseStatement()
{
    statementIdCount = 0;
    statementConstCount = 0;
    statementOpCount = 0;
    error_occured = false;
    opWarnigCode = 0;
    opWarningCount = 0;
    error_count = 0;

    Symbol *sym = getCurrentToken();

    if (sym->symbol_type != IDENTIFIER)
    {
        printf("%d\n", sym->symbol_type);
        printf("Syntax ERROR NO IDENTFIER in Statements");
        return;
    }
    statementIdCount++;
    Ident *id = isExistId(*sym);

    // id가 null인지는 확인할 필요가 없다 이미 전부 createIdarray를 통해 모든 Id를 만들었기때문에.
    moveToNextToken();
    if (getCurrentToken()->symbol_type == ASSIGNMENT1)
    {
        moveToNextToken();
        if (getCurrentToken()->symbol_type == ASSIGNMENT2)
        {
            moveToNextToken();
            int result = parseExpression();
            if (error_occured == false)
            {                                     // 반환값을 변수에 저장
                sprintf(id->value, "%d", result); // 정수를 문자열로 변환하여 할당
            }
            else
            {
                sprintf(id->value, "%s", "Unknown");
            }
            return;
        }
        else
        {
            opWarningCode[opWarningCount++] = 5;
            int result = parseExpression(); // 반환값을 변수에 저장
            if (error_occured == false)
            {                                     // 반환값을 변수에 저장
                sprintf(id->value, "%d", result); // 정수를 문자열로 변환하여 할당
            }
            else
            {
                sprintf(id->value, "%s", "Unknown");
            }
            return;
        }
    }
    else if (getCurrentToken()->symbol_type == ASSIGNMENT2)
    {
        opWarningCode[opWarningCount++] = 5;
        moveToNextToken();
        int result = parseExpression(); // 반환값 = parseExpression();   // 반환값을 변수에 저장
        if (error_occured == false)
        {                                     // 반환값을 변수에 저장
            sprintf(id->value, "%d", result); // 정수를 문자열로 변환하여 할당
        }
        else
        {
            sprintf(id->value, "%s", "Unknown");
        } // 정수를 문자열로 변환하여 할당
        return;
    }
    else
    {
        printf("Error : We Need Assignment!!!!\n");
        return;
    }
}

int parseExpression()
{
    int term = parseTerm();

    // 왼쪽부터 오른쪽으로 연산하기 위해 반복문 사용
    int result = term;
    while (getCurrentToken()->symbol_type == ADD_OP || getCurrentToken()->symbol_type == SUB_OP)
    {
        int op_type = getCurrentToken()->symbol_type;
        moveToNextToken();

        // 중복 연산자 처리
        if (getCurrentToken()->symbol_type == op_type)
        {
            if (op_type == ADD_OP)
                opWarningCode[opWarningCount++] = 1;
            else
                opWarningCode[opWarningCount++] = 2;
            moveToNextToken();
        }

        statementOpCount++;
        int next_term = parseTerm();

        if (op_type == ADD_OP)
        {
            result = result + next_term;
        }
        else // SUB_OP
        {
            result = result - next_term;
        }
    }

    return result;
}

int parseTerm()
{
    int factor = parseFactor();

    // 왼쪽부터 오른쪽으로 연산하기 위해 반복문 사용
    int result = factor;
    while (getCurrentToken()->symbol_type == MULTI_OP || getCurrentToken()->symbol_type == DEVISION_OP)
    {
        int op_type = getCurrentToken()->symbol_type;
        moveToNextToken();

        // 중복 연산자 처리
        if (getCurrentToken()->symbol_type == op_type)
        {
            if (op_type == MULTI_OP)
                opWarningCode[opWarningCount++] = 3;
            else
                opWarningCode[opWarningCount++] = 4;
            moveToNextToken();
        }

        statementOpCount++;
        int next_factor = parseFactor();

        if (op_type == MULTI_OP)
        {
            result = result * next_factor;
        }
        else // DEVISION_OP
        {
            if (next_factor != 0)
                result = result / next_factor;
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

int parseFactor()
{
    if (getCurrentToken()->symbol_type == LEFT_PAREN)
    {
        moveToNextToken();
        int result = parseExpression();
        if (getCurrentToken()->symbol_type == RIGHT_PAREN)
        {
            moveToNextToken();
            return result;
        }
        else
        {
            printf("Error : The right Paren is Needed!!!!!");
        }
    }
    else if (getCurrentToken()->symbol_type == IDENTIFIER)
    {
        statementIdCount++;
        Ident *id = isExistId(*getCurrentToken());
        int idIndex = getIdIndex(id->name);
        if ((strlen(id->value) == 0 || strcmp(id->value, "Unknown") == 0) &&
            idIndex >= 0 && undefinedHandled[idIndex] == false)
        {
            errorIdName = id->name;
            error_occured = true;
            error_count++;
            undefinedHandled[idIndex] = true;
            moveToNextToken();
            return 0;
        }
        else
        {
            int result = atoi(id->value);
            moveToNextToken();
            return result;
        }
    }
    else if (getCurrentToken()->symbol_type == CONST)
    {
        statementConstCount++;
        int result = getCurrentToken()->const_value;
        moveToNextToken();
        return result;
    }
    else
    {
        return 0;
    }
    return 0;
}

void removeDuplicateOperators(char *input_line)
{
    char temp_line[1024];
    int len = strlen(input_line);
    int write_pos = 0;

    for (int i = 0; i < len; i++)
    {
        temp_line[write_pos] = input_line[i];

        // 현재 문자가 연산자인 경우
        if (input_line[i] == '+' || input_line[i] == '-' ||
            input_line[i] == '*' || input_line[i] == '/')
        {
            char curr_op = input_line[i];
            int j = i + 1;

            // 공백 건너뛰기
            while (j < len && isspace(input_line[j]))
            {
                j++;
            }

            // 같은 연산자가 있는지 확인
            if (j < len && input_line[j] == curr_op)
            {
                // 중복 연산자 발견, 두 번째 것을 건너뛰기
                i = j; // 두 번째 연산자 위치로 이동

                // 추가 중복 연산자가 있는지 확인 (연속된 3개 이상)
                j = i + 1;
                while (j < len)
                {
                    if (isspace(input_line[j]))
                    {
                        j++;
                        continue;
                    }
                    else if (input_line[j] == curr_op)
                    {
                        i = j; // 계속 건너뜀
                        j++;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
        write_pos++;
    }

    temp_line[write_pos] = '\0';
    strcpy(input_line, temp_line);
}

void substituteAssignmentOperator(char *input_line)
{
    char temp_line[1024];
    int len = strlen(input_line);
    int write_pos = 0;

    for (int i = 0; i < len; i++)
    {
        if (i < len - 1 && input_line[i] == ':' && input_line[i + 1] == '=')
        {
            // ":=" -> ":=" 그대로 유지
            temp_line[write_pos++] = ':';
            temp_line[write_pos++] = '=';
            i++; // '=' 건너뛰기
        }
        else if (input_line[i] == '=' && (i == 0 || input_line[i - 1] != ':'))
        {
            // 단독 '=' -> ":=" 로 변경
            temp_line[write_pos++] = ':';
            temp_line[write_pos++] = '=';
        }
        else
        {
            temp_line[write_pos++] = input_line[i];
        }
    }

    temp_line[write_pos] = '\0';
    strcpy(input_line, temp_line);
}

/* ================== FUNCTION ADDITION AREA END ====================
=====================================================================
=====================================================================*/

void parse()
{
    parseProgram();
}

void parse_V()
{
    // 모든 줄을 처리하며 토큰을 출력
    while (strlen(line) > 0 && line[0] != '\0' && line[0] != '\n')
    {
        // 줄 끝의 개행 문자 제거
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }

        lexical_analysis();

        // 각 토큰을 요구된 형식으로 출력 (중복 연산자 제거)
        for (int i = 0; i < symbol_count; i++)
        {
            Symbol *s = &symbolArray[i];

            // 중복 연산자 건너뛰기 체크
            bool skip_duplicate = false;
            if (i > 0 && (s->symbol_type == ADD_OP || s->symbol_type == SUB_OP ||
                          s->symbol_type == MULTI_OP || s->symbol_type == DEVISION_OP ||
                          s->symbol_type == ASSIGNMENT1 || s->symbol_type == ASSIGNMENT2))
            {
                // 이전 토큰이 같은 연산자인지 확인
                if (symbolArray[i - 1].symbol_type == s->symbol_type)
                {
                    skip_duplicate = true;
                }
                // assignment operator는 ASSIGNMENT1과 ASSIGNMENT2 둘 다 assignment_op로 출력되므로
                // 둘 중 하나가 이전에 있었다면 건너뛰기
                else if ((s->symbol_type == ASSIGNMENT1 || s->symbol_type == ASSIGNMENT2) &&
                         (symbolArray[i - 1].symbol_type == ASSIGNMENT1 || symbolArray[i - 1].symbol_type == ASSIGNMENT2))
                {
                    skip_duplicate = true;
                }
                // ADD_OP와 SUB_OP는 둘 다 add_operator로 출력되므로 중복 체크
                else if ((s->symbol_type == ADD_OP || s->symbol_type == SUB_OP) &&
                         (symbolArray[i - 1].symbol_type == ADD_OP || symbolArray[i - 1].symbol_type == SUB_OP))
                {
                    skip_duplicate = true;
                }
                // MULTI_OP와 DEVISION_OP는 둘 다 mult_operator로 출력되므로 중복 체크
                else if ((s->symbol_type == MULTI_OP || s->symbol_type == DEVISION_OP) &&
                         (symbolArray[i - 1].symbol_type == MULTI_OP || symbolArray[i - 1].symbol_type == DEVISION_OP))
                {
                    skip_duplicate = true;
                }
            }

            if (skip_duplicate)
            {
                continue; // 중복 연산자는 건너뛰기
            }

            switch (s->symbol_type)
            {
            case IDENTIFIER:
                printToken("ident");
                break;
            case CONST:
                printToken("const");
                break;
            case ASSIGNMENT1:
            case ASSIGNMENT2:
                printToken("assignment_op");
                break;
            case ADD_OP:
            case SUB_OP:
                printToken("add_operator");
                break;
            case MULTI_OP:
            case DEVISION_OP:
                printToken("mult_operator");
                break;
            case LEFT_PAREN:
                printToken("left_paren");
                break;
            case RIGHT_PAREN:
                printToken("right_paren");
                break;
            case SEMI_COLON:
                printToken("semi_colon");
                break;
            default:
                printToken("unknown");
                break;
            }
        }

        // 다음 줄 읽기
        if (fgets(line, sizeof(line), file) == NULL)
        {
            break;
        }
    }
}

/**
 *
 * @brief Main function for processing a file.
 *
 */
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s [-v] <filepath>\n", argv[0]);
        return 1;
    }

    int verbose = 0;
    char *filepath = NULL;

    if (strcmp("-v", argv[1]) == 0)
    {
        verbose = 1;
        if (argc < 3)
        {
            fprintf(stderr, "Error: No file specified.\n");
            return 1;
        }
        filepath = argv[2];
    }
    else
    {
        filepath = argv[1];
    }

    file = fopen(filepath, "r");

    if (file == NULL)
    {
        fprintf(stderr, "Error: Could not open file %s\n", filepath);
        return 1;
    }

    // Gets input line for the first time
    fgets(line, sizeof(line), file);

    // Depending on the verbose flag, call the appropriate function
    if (verbose)
    {
        parse_V();
    }
    else
    {
        parse();
    }

    fclose(file);
    return 0;
}
