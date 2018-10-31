/* Nathaniel Osterberg
CS210_101

LEXICAL ANALYZER
**************************************************************************************
  This program is a lexical analyzer that takes a text input file and parses the text
  into lexemes, then categorizes the lexemes into their appropriate tokens and outputs
  each lexeme with its categorization into the output file.
**************************************************************************************
***** THIS LEXER IS BASED ON A SIMPLE LEXICAL ANALYZER FROM "CONCEPTS OF PROGRAMMING LANGUAGES" 11TH EDITION BY ROBERT W. SEBESTA *****
rev. 10/11/2018:
Built framework for logic

rev. 10/13/2018:
Sorted identifiers and numeric literals

rev. 10/14/2018:
Sorted keywords and operators

rev. 10/15/2018:
Sorted comments, strings, and character literals

rev. 10/23/2018:
Reworked operator identification

rev. 10/24/2018:
Reworked operator identification again

rev. 10/25/2018:
Cleaned and finished commenting code
*/

#include <iostream>
#include <stdlib.h>         // stdlib.h only used for exit(1) function
#include <fstream>
#include <string>
#include <cctype>

using namespace std;

// function prototypes
void getInputFile(string fileIn);
void getChar(); // function that gets a character from the infile
int lex();
void getNonBlank();
void addChar();
int lookup(char);
void keywordOrIdent(); // function to choose between keyword or identifier
void sortOper(); // function to check for operators
void commentCheckStar();
void commentCheckSlash();

// global variables
ifstream inputFile;     // input stream
ofstream outputFile;    // output stream
string fileIn;          // string for file name
string fileOut;         // string for output file name
string fileName;        // rename for outfile
int charClass;          // int to sort character class
char lexeme[1000];       // char array to hold lexeme
char nextChar;          // char to peek at next char
int lexLen;             // size of lexeme
int token;              // value assigned to sort token
int nextToken;          // value assigned to next token
char ch;                // temp character value
char tempX = ch;
string tokenName;       // string associated with token for output

string keywordArray[37] = {"accessor", "and", "array", "begin", "bool", "case",
    "character", "constant", "else", "elsif", "end", "exit", "function",
    "if", "in", "integer", "interface", "is", "loop", "module", "mutator",
    "natural", "null", "of", "or", "others", "out", "positive", "procedure",
    "range", "return", "struct", "subtype", "then", "type", "when", "while"};

// character classes
#define LETTER 0
#define DIGIT 1
#define PUNCTUATION 2

// Token Codes
#define COMMENT_LIT 10
#define IDENT_LIT 11
#define STRING_LIT 12
#define KEYWORD_LIT 13
#define CHAR_LIT 14
#define OPERATOR_LIT 15
#define INT_LIT 16
#define UNKNOWN 17

int main(int argc, const char *argv[])
{
        if (argc != 2)
        {
            cout << "USAGE: " << argv[0] << endl;
            exit(1);
            //return 0;
        }
        getInputFile(argv[1]);

        inputFile.close();          // close if stream
        outputFile.close();         // close of stream
    return 0;
}

//************************************************************************************
//      void getInputFile();
//          function to specify the input file to be analyzed
//************************************************************************************

void getInputFile(string fileIn)
{
    string fileExt = ".lexer";                  // names file extension
    fileName = fileIn;                          // input file name
    fileOut = fileIn + fileExt;                 // output file name + new extension

    inputFile.open(fileIn);
    outputFile.open(fileOut);

    if (!inputFile)
    {
        cout << "ERROR - Cannot open input file. " << fileIn;   // stream validation
        exit(1);
    }
    if (!outputFile)                                            // stream validation
    {
        cout << "ERROR - Cannot open output file. " << fileOut;
        exit(1);
    }
    getChar();                                  // get first character
    do
    {
        lex();                                  // put it into lexeme
    }while(nextToken != EOF);                   // until EOF
}

//************************************************************************************
//      void getChar();
//          function to get the next character from the input file
//************************************************************************************

void getChar()
{
    ch = inputFile.get();                                   // get character from file

    if((nextChar = ch) != EOF)
    {
        if (nextChar == '/')                                // check to see if char is /
        {
            charClass = COMMENT_LIT;
        }
        else if(nextChar == '"')                            // check to see if char is "
        {
            charClass = STRING_LIT;
        }
        else if((int)nextChar == 39)                        // static cast of ASCII value for '
        {
            charClass = CHAR_LIT;
        }
        else if((isalpha(nextChar)) || nextChar == '_')     // check to see if char is letter or digit
        {
            charClass = LETTER;
        }
        else if((isdigit(nextChar)) || nextChar == '#')     // check to see if char is a digit or #
        {
            charClass = DIGIT;
        }
        else if (nextChar == ':' || nextChar == '.' || nextChar == '<' || nextChar == '>' || nextChar == '*' || nextChar == '!' || nextChar == '=' || nextChar == '(' ||
        nextChar == ')' || nextChar == '+' || nextChar == '-' || nextChar == '/' || nextChar == '|' || nextChar == '&' || nextChar == ',' || nextChar == ';' ||
        nextChar == '[' || nextChar == ']')                 // check to see if character is in operator list
        {
            charClass = PUNCTUATION;
        }
        else
            charClass = UNKNOWN;                            // unknown class
    }
    else
    {
        charClass = EOF;
    }
}

//************************************************************************************
//      int lex();
//          function that sorts the character class of the character being read
//************************************************************************************

int lex()
{
    lexLen = 0;             // lex length set to 0
    getNonBlank();
    switch(charClass)
    {
    case COMMENT_LIT:
        lookup(nextChar); // case will run function lookup on char to decide what to do
        break;
    case STRING_LIT:
        lookup(nextChar);
        break;
    case CHAR_LIT:
        lookup(nextChar);
        break;
    case LETTER:
        addChar();
        getChar();
        while(charClass == LETTER || charClass == DIGIT || charClass == '_') // check for underscore for identifiers
        {
            addChar();
            getChar();
        }
        keywordOrIdent();
        break;
    case DIGIT:
        addChar();
        getChar();
        while (charClass == DIGIT || charClass == '.')                      // check for decimal point in numeric literal
        {
            addChar();
            getChar();
        }
        nextToken = INT_LIT;
        tokenName = " (numeric literal)";
        break;
    case PUNCTUATION:
        addChar();
        getChar();
        sortOper();
        break;
    case UNKNOWN:           // loops for symbols that are not letters or numbers
        lookup(nextChar);
        getChar();
        tokenName = " (unknown)";
        break;
    case EOF:               // prints EOF
        exit(1);            // exits at end of file
        break;
    }
    outputFile << lexeme << tokenName << endl;              // output into output file
    return nextToken;
}

//************************************************************************************
//      void getNonBlank();
//          function that gets the next non-blank character
//************************************************************************************

void getNonBlank()
{
    while(isspace(nextChar))    // checks if next Char is a space and if so, gets nextChar
        getChar();
}

//************************************************************************************
//      void addChar();
//          function that adds next char to the lexeme
//************************************************************************************

void addChar()
{
    if (lexLen <= 997)       // size needs to give room for EOF characters
    {
        lexeme[lexLen++] = nextChar;    // adds nextChar to lexeme
        lexeme[lexLen] = 0;             // resets lexeme to 0
    }
    else
        cout << "ERROR - Lexeme is too long \n";    // error handling
}

//************************************************************************************
//      int lookup(char ch)
//          function to look up operators and identify them and return the token.
//************************************************************************************

int lookup(char ch)
{
    switch((int)ch)             // static cast ch to integer so able to test against ascii values
    {
        case 34:                // case 34 is quotation mark
            addChar();          // adds next char to lexeme
            getChar();
            do                  // continues to add characters to the lexeme until the next character is quotation mark (")
            {
                addChar();
                getChar();
            }
            while (nextChar != '"');
            addChar();
            getChar();
            nextToken = STRING_LIT;     // nextToken set to STRING_LIT value
            tokenName = " (string)";
            break;
        case 39:                        // case 39 is single quote
            addChar();                  // adds next character to the lexeme
            getChar();
            if ((int)nextChar != 39)    // checks if nextChar cast to an integer is equal to the ascii value for single quote. (39)
            {
                addChar();              // if true, it adds another character.
                getChar();
            }
            addChar();                  // adds final character (which should be a single quote)
            getChar();
            nextToken = CHAR_LIT;       // sets nextToken to CHAR-LIT
            tokenName = " (character literal)"; // token name set
            break;
        case 47:                        // case 47
            addChar();                  // if the case is '/', nextChar will be added to lexeme
            getChar();
            if (nextChar == '*')        // nextChar will be checked for '*'
            {
                commentCheckStar();     // if it is a star, will look at the next character to check its val
            }
            else
            {
                nextToken = OPERATOR_LIT;   // token assigned as operator
                tokenName = " (operator)";
            }
            break;
        default:
            addChar();
            nextToken = UNKNOWN;        // token set to be unknown case lexeme ins't identified
            break;
    }
    return nextToken;
}

//****************************************************************************
//      void keywordOrIdent();
//          function to check if lexeme is keyword or identifier
//****************************************************************************

void keywordOrIdent()
{
    string tempString(lexeme);                  // converts lexeme to string
    int x = 37;                                 // length of the keywordArray
    for (int index = 0; index < x; index++)     // for loop indexes through keyword array
    {
        if (tempString == keywordArray[index])  // tempString is compared to array index value
        {
            nextToken = KEYWORD_LIT;            // if true, nextToken set to KEYWORD_LIT
            tokenName = " (keyword)";           // output for keyword
            break;
        }
        else
        {
            nextToken =  IDENT_LIT;             // if false, nextToken set to IDENT_LIT
            tokenName = " (identifier)";        // output for identifier
        }
    }
}

//****************************************************************************
//      void sortOper();
//          function to sort whether a token is an operator or unknown
//****************************************************************************

void sortOper()
{
    if (ch == ':' || ch == '.' || ch == '<' || ch == '>' || ch == '=' || ch == '*' || ch == '!')    // if ch is first character of a 2 character operator, will return true
    {
        addChar();              // gets next character and adds to lexeme
        getChar();
        if (ch == ':' || ch == '.' || ch == '<' || ch == '>' || ch == '*' || ch == '=')  // checks next character to see if it is the second character in a two character operator
        {
            nextToken = OPERATOR_LIT;       // if so, set to true
            tokenName = " (operator)";
        }
        else
        {
            nextToken = OPERATOR_LIT;       // catch all *********** This is a spot in the code I can clean up more **********
            tokenName = " (operator)";
        }
    }
    else
    {
        nextToken = OPERATOR_LIT;       // if operator is 1 character
        tokenName = " (operator)";
    }
}

//****************************************************************************
//      void commentCheckStar();
//          function that does do while loop while nextChar is not an *.
//****************************************************************************

void commentCheckStar()
{
    do                                          // will add new char to lexeme
    {
        addChar();
        getChar();
    }
    while(nextChar != '*');                     // will end do loop when nextChar is '*'
    if (nextChar == '*')
    {
        commentCheckSlash();                    // function call to commentCheckSlash();
    }
}

//***************************************************************************
//      void commentCheckSlash();
//          function to check if lexeme is '/', else it will call commentCheckStar();
//***************************************************************************

void commentCheckSlash()
{
    addChar();                                  // adds next character to lexeme
    getChar();
    if(nextChar == '/')                         // checks for char if is /
    {
        addChar();                              // adds next character after slash to lexeme
        getChar();
        nextToken = COMMENT_LIT;                // categorizes lexeme as comment
        tokenName = " (comment)";
    }
    else
    {
        commentCheckStar();                     // if next char is not slash, returns to check for a star
    }
}
