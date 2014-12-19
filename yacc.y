/* Author: Martin Brugnara #157791 <martin.brugnara@studenti.unitn.it> */

%{
    #include <stdlib.h>
    #include <stdio.h>
    #include "interpreter.h"

    // TODO: include
    /* GLOBAL VARS */
    symrec * symTable;

    // ext references (dark magic)
    int yylex();
%}
%error-verbose

%union { // yylval_t
    int     iVal;
    float   rVal;
    int     bVal;

    struct symrec   * sRec;
    struct nodeType * nPtr;

    varTypeEnum varType;
};

/*
    Define association between llval &
    llval_type (return argv of lex matching)
    format:
        %token <field_name of union> TOKEN
*/
%token <iVal> INTEGER
%token <rVal> REALNUM
%token <bVal> BOOLEAN
%token <sRec>   VARIABLE

%token WHILE IF PRINT FOR TO
%nonassoc IFX
%nonassoc ELSE

%left EQ
%left DEQ NE GT LT GTE LTE
%left PLUS MIN
%left MUL DIV
%nonassoc UMINUS RCURLY LCURLY LP RP COMMA SEMICOLON INTEGER REALNUM BOOLEAN MAIN

%type <nPtr> stmt dec expr stmt_list opt_stmt_list opt_dec_list

%%

program: opt_dec_list
        MAIN
        opt_stmt_list   {
                            /* since I do not give a fuck about func
                                I deliberately skip scope creation */
                            ex($1, symTable); // Populate Global VARS
                            ex($3, symTable); // Exec Main
                            exit(0); // exit success
                        }
        ;

opt_dec_list: /* empty */
            | opt_dec_list dec  {$$ = opr(SEMICOLON, 2, $1, $2);}
            ;

dec: VARIABLE EQ expr SEMICOLON {$$ = opr(EQ,2,id($1),$3);}
   ;

opt_stmt_list: /* empty */
            | stmt_list
            ;

stmt_list: stmt
         | stmt_list stmt   {$$ = opr(SEMICOLON, 2, $1, $2);}
         ;

stmt: SEMICOLON                                     {$$ = opr(SEMICOLON, 2, NULL, NULL);}
    | expr SEMICOLON
    | PRINT expr SEMICOLON                          {$$ = opr(PRINT,1,$2);}
    | dec
    | WHILE LP expr RP stmt                         {$$ = opr(WHILE,2,$3,$5);}
    | IF LP expr RP stmt %prec IFX                  {$$ = opr(IF,2,$3,$5);}
    | IF LP expr RP stmt ELSE stmt                  {$$ = opr(IF,3,$3,$5,$7);}
    | FOR LP VARIABLE EQ expr TO expr RP stmt       {$$ = opr(FOR,4,id($3),$5,$7,$9);}
    | LCURLY stmt_list RCURLY                       {$$ = $2;}
    ;

expr: INTEGER               {$$ = con(&$1, INTTYPE);} //manage constants
    | REALNUM               {$$ = con(&$1, REALTYPE);}
    | BOOLEAN               {$$ = con(&$1, BOOLTYPE);}
    | VARIABLE              {$$ = id($1);} //manage variables - namely an IDENTIFIER
    | MIN expr %prec UMINUS {$$ = opr(UMINUS,1,$2);}
    | expr PLUS expr        {$$ = opr(PLUS,2,$1,$3);}
    | expr MIN expr         {$$ = opr(MIN,2,$1,$3);}
    | expr MUL expr         {$$ = opr(MUL,2,$1,$3);}
    | expr DIV expr         {$$ = opr(DIV,2,$1,$3);}
    | expr LT  expr         {$$ = opr(LT,2,$1,$3);}
    | expr GT expr          {$$ = opr(GT,2,$1,$3);}
    | expr LTE expr         {$$ = opr(LTE,2,$1,$3);}
    | expr GTE expr         {$$ = opr(GTE,2,$1,$3);}
    | expr NE expr          {$$ = opr(NE,2,$1,$3);}
    | expr DEQ expr         {$$ = opr(DEQ,2,$1,$3);}
    | LP expr RP            {$$ = $2;}
    ;

%%

int main(){
    yyparse();
    return 0;
}
