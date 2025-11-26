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




// ================= GLOBAL VARIABLES SECTION END =============
//=============================================================
char line[1024];
FILE *file;
Ident idArray[256];

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



/**
 * @brief This function is the starting point of this project with no option -v (print option a).
 *
 * You can freely modify this code or add more functions.
 * However, you SHOULD use the print functions below when you need to print some lines on screen,
 * or you might risk receiving 0 points even if the program works perfectly.
 */
void parse() {
    // TODO: Implement the parsing logic here
}

/**
 * @brief This function is the starting point of this project with option -v (print option b).
 *
 * You can freely modify this code or add more functions.
 * However, you SHOULD use the print functions below when you need to print some lines on screen,
 * or you might risk receiving 0 points even if the program works perfectly.
 */
void parse_V() {
    // TODO: Implement the parsing logic here for verbose output
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
