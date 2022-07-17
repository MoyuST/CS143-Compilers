/*
*  cool.y
*              Parser definition for the COOL language.
*
*/
%{
  #include <iostream>
  #include "cool-tree.h"
  #include "stringtab.h"
  #include "utilities.h"
  
  extern char *curr_filename;
  
  
  /* Locations */
  #define YYLTYPE int              /* the type of locations */
  #define cool_yylloc curr_lineno  /* use the curr_lineno from the lexer
  for the location of tokens */
    
    extern int node_lineno;          /* set before constructing a tree node
    to whatever you want the line number
    for the tree node to be */
      
      
      #define YYLLOC_DEFAULT(Current, Rhs, N)         \
      Current = Rhs[1];                             \
      node_lineno = Current;
    
    
    #define SET_NODELOC(Current)  \
    node_lineno = Current;
    
    /* IMPORTANT NOTE ON LINE NUMBERS
    *********************************
    * The above definitions and macros cause every terminal in your grammar to 
    * have the line number supplied by the lexer. The only task you have to
    * implement for line numbers to work correctly, is to use SET_NODELOC()
    * before constructing any constructs from non-terminals in your grammar.
    * Example: Consider you are matching on the following very restrictive 
    * (fictional) construct that matches a plus between two integer constants. 
    * (SUCH A RULE SHOULD NOT BE  PART OF YOUR PARSER):
    
    plus_consts	: INT_CONST '+' INT_CONST 
    
    * where INT_CONST is a terminal for an integer constant. Now, a correct
    * action for this rule that attaches the correct line number to plus_const
    * would look like the following:
    
    plus_consts	: INT_CONST '+' INT_CONST 
    {
      // Set the line number of the current non-terminal:
      // ***********************************************
      // You can access the line numbers of the i'th item with @i, just
      // like you acess the value of the i'th expression with $i.
      //
      // Here, we choose the line number of the last INT_CONST (@3) as the
      // line number of the resulting expression (@$). You are free to pick
      // any reasonable line as the line number of non-terminals. If you 
      // omit the statement @$=..., bison has default rules for deciding which 
      // line number to use. Check the manual for details if you are interested.
      @$ = @3;
      
      
      // Observe that we call SET_NODELOC(@3); this will set the global variable
      // node_lineno to @3. Since the constructor call "plus" uses the value of 
      // this global, the plus node will now have the correct line number.
      SET_NODELOC(@3);
      
      // construct the result node:
      $$ = plus(int_const($1), int_const($3));
    }
    
    */
    
    /* record nested let expression */
    #include <stack>
    typedef struct let_stack_elem__ {
      Symbol symbol1_;
      Symbol symbol2_;
      Expression expression1_;
      Expression expression2_;

      let_stack_elem__(Symbol s1, Symbol s2, Expression e1, Expression e2): symbol1_(s1), symbol2_(s2), expression1_(e1), expression2_(e2) {};

    } let_stack_elem_;

    std::stack<let_stack_elem_> let_stack_;
    
    void yyerror(char *s);        /*  defined below; called for each parse error */
    extern int yylex();           /*  the entry point to the lexer  */
    
    /************************************************************************/
    /*                DONT CHANGE ANYTHING IN THIS SECTION                  */
    
    Program ast_root;	      /* the result of the parse  */
    Classes parse_results;        /* for use in semantic analysis */
    int omerrs = 0;               /* number of errors in lexing and parsing */
    %}
    
    /* A union of all the types that can be the result of parsing actions. */
    %union {
      Boolean boolean;
      Symbol symbol;
      Program program;
      Class_ class_;
      Classes classes;
      Feature feature;
      Features features;
      Formal formal;
      Formals formals;
      Case case_;
      Cases cases;
      Expression expression;
      Expressions expressions;
      char *error_msg;
    }
    
    /* 
    Declare the terminals; a few have types for associated lexemes.
    The token ERROR is never used in the parser; thus, it is a parse
    error when the lexer returns it.
    
    The integer following token declaration is the numeric constant used
    to represent that token internally.  Typically, Bison generates these
    on its own, but we give explicit numbers to prevent version parity
    problems (bison 1.25 and earlier start at 258, later versions -- at
    257)
    */
    %token CLASS 258 ELSE 259 FI 260 IF 261 IN 262 
    %token INHERITS 263 LET 264 LOOP 265 POOL 266 THEN 267 WHILE 268
    %token CASE 269 ESAC 270 OF 271 DARROW 272 NEW 273 ISVOID 274
    %token <symbol>  STR_CONST 275 INT_CONST 276 
    %token <boolean> BOOL_CONST 277
    %token <symbol>  TYPEID 278 OBJECTID 279 
    %token ASSIGN 280 NOT 281 LE 282 ERROR 283
    
    /*  DON'T CHANGE ANYTHING ABOVE THIS LINE, OR YOUR PARSER WONT WORK       */
    /**************************************************************************/
    
    /* Complete the nonterminal list below, giving a type for the semantic
    value of each non terminal. (See section 3.6 in the bison 
    documentation for details). */
    
    /* Declare types for the grammar's non-terminals. */
    %type <program> program
    %type <classes> class_list
    %type <class_> class
    
    /* You will want to change the following line. */
    /* _s in the non-terminal name short for '_symbol' */
    %type <features> feature_list
    %type <feature> feature_s
    %type <formals> formal_list
    %type <formal> formal_s
    %type <expression> expression_s
    %type <cases> case_list
    %type <case_> case_expr
    %type <expressions> expressions_by_comma
    %type <expressions> expression_by_colon
    %type <error_msg> error
    
    /* Precedence declarations go here. */
    %right IN
    %right ASSIGN
    %left NOT
    %nonassoc  LE '<' '='
    %left '+' '-'
    %left '*' '/'
    %left ISVOID
    %left '~'
    %left '@'
    %left '.'
    
    %%
    
    /* 
    Save the root of the abstract syntax tree in a global variable.
    */
    program	: class_list	{ @$ = @1; ast_root = program($1); }
    ;
    
    class_list
    : class			/* single class */
    { @$ = @1;SET_NODELOC(@1);$$ = single_Classes($1);
    parse_results = $$; }
    | class_list class	/* several classes */
    { @$ = @2;SET_NODELOC(@2);$$ = append_Classes($1,single_Classes($2)); 
    parse_results = $$; }
    | error class
    { @$ = @2; ;SET_NODELOC(@2);$$ = single_Classes($2);
    parse_results = $$; yyerrok;}
    | class_list error class
    { @$ = @3; ;SET_NODELOC(@3); $$ = append_Classes($1,single_Classes($3)); yyerrok;}
    ;
    
    /* If no parent is specified, the class inherits from the Object class. */
    class	
    : CLASS TYPEID '{' '}' ';'
    { @$ = @5;SET_NODELOC(@5);$$ = class_($2,idtable.add_string("Object"),nil_Features(),
    stringtable.add_string(curr_filename)); }
    | CLASS TYPEID '{' feature_list '}' ';'
    { @$ = @6;SET_NODELOC(@6);$$ = class_($2,idtable.add_string("Object"),$4,
    stringtable.add_string(curr_filename)); }
    | CLASS TYPEID INHERITS TYPEID '{' '}' ';'
    { @$ = @7;SET_NODELOC(@7);$$ = class_($2,$4,nil_Features(),stringtable.add_string(curr_filename)); }
    | CLASS TYPEID INHERITS TYPEID '{' feature_list '}' ';'
    { @$ = @8;SET_NODELOC(@8);$$ = class_($2,$4,$6,stringtable.add_string(curr_filename)); }
    | CLASS error '}' ';'
    { @$ = @4;SET_NODELOC(@4); }
    ;
    
    /* Feature list may be empty, but no empty features in list. */
    feature_list
    :	feature_s /* single feature */
    { @$ = @1;SET_NODELOC(@1);$$ = single_Features($1); }
    | feature_list feature_s /* several features */
    { @$ = @2;SET_NODELOC(@2);$$ =  append_Features($1, single_Features($2));}
    | error feature_s
    { @$ = @2;SET_NODELOC(@2);$$ = single_Features($2); }
    | feature_list error feature_s
    { @$ = @3;SET_NODELOC(@3);$$ = append_Features($1, single_Features($3)); }
    ;

    feature_s 
    : OBJECTID '(' ')' ':' TYPEID '{' expression_s '}' ';'
    { @$ = @9;SET_NODELOC(@9);$$ = method($1, nil_Formals(), $5, $7); }
    | OBJECTID '(' formal_list ')' ':' TYPEID '{' expression_s '}' ';'
    { @$ = @10;SET_NODELOC(@10);$$ = method($1, $3, $6, $8); }
    | OBJECTID ':' TYPEID ';'
    { @$ = @4;SET_NODELOC(@4);$$ = attr($1, $3, no_expr()); }
    | OBJECTID ':' TYPEID ASSIGN expression_s ';'
    { @$ = @6;SET_NODELOC(@6);$$ = attr($1, $3, $5); }
    | OBJECTID error ';'
    { @$ = @3;SET_NODELOC(@3); }
    ;

    formal_list 
    : formal_s /* single formal */
    { @$ = @1;SET_NODELOC(@1);$$ = single_Formals($1); }
    | formal_list ',' formal_s /* several formals */
    { @$ = @3;SET_NODELOC(@3);$$ = append_Formals($1, single_Formals($3)); }
    ;

    formal_s
    : OBJECTID ':' TYPEID
    { @$ = @3;SET_NODELOC(@3);$$ = formal($1, $3); }
    ;

    expression_s
    : OBJECTID ASSIGN expression_s
    { @$ = @3;SET_NODELOC(@3);$$ = assign($1, $3); }
    | expression_s '@' TYPEID '.' OBJECTID '(' expressions_by_comma ')'
    { @$ = @8;SET_NODELOC(@8);$$ = static_dispatch($1, $3, $5, $7); }
    | expression_s '.' OBJECTID '(' expressions_by_comma ')'
    { @$ = @6;SET_NODELOC(@6);$$ = dispatch($1, $3, $5); }
    | OBJECTID '(' expressions_by_comma ')'
    { @$ = @3;SET_NODELOC(@3);$$ = dispatch(object(idtable.add_string("self")), $1, $3); }
    | IF expression_s THEN expression_s ELSE expression_s FI
    { @$ = @7;SET_NODELOC(@7);$$ = cond($2, $4, $6); }
    | WHILE expression_s LOOP expression_s POOL
    { @$ = @5;SET_NODELOC(@5);$$ = loop($2, $4);}
    | '{' expression_by_colon ';' '}'
    { @$ = @4;SET_NODELOC(@4);$$ = block($2); }
    | LET let_assign_nested
    {
      if(!let_stack_.empty()){
        @$ = @1;SET_NODELOC(@1);
        Expression expression_ptr = let(let_stack_.top().symbol1_, let_stack_.top().symbol2_, 
            let_stack_.top().expression1_, let_stack_.top().expression2_);
        let_stack_.pop();
        while(!let_stack_.empty()) {
          @$ = @1;SET_NODELOC(@1);
          expression_ptr = let(let_stack_.top().symbol1_, let_stack_.top().symbol2_, 
            let_stack_.top().expression1_, expression_ptr);
          let_stack_.pop();
        };
        $$ = expression_ptr;
      }
    }
    | CASE expression_s OF case_list ESAC
    { @$ = @5;SET_NODELOC(@5);$$ = typcase($2, $4); }
    | NEW TYPEID
    { @$ = @2;SET_NODELOC(@2);$$ = new_($2); }
    | ISVOID expression_s
    { @$ = @2;SET_NODELOC(@2);$$ = isvoid($2); }
    | expression_s '+' expression_s
    { @$ = @3;SET_NODELOC(@3);$$ = plus($1, $3); }
    | expression_s '-' expression_s
    { @$ = @3;SET_NODELOC(@3);$$ = sub($1, $3); }
    | expression_s '*' expression_s
    { @$ = @3;SET_NODELOC(@3);$$ = mul($1, $3); }
    | expression_s '/' expression_s
    { @$ = @3;SET_NODELOC(@3);$$ = divide($1, $3); }
    | '~' expression_s
    { @$ = @2;SET_NODELOC(@2);$$ = neg($2); }
    | expression_s '<' expression_s
    { @$ = @3;SET_NODELOC(@3);$$ = lt($1, $3); }
    | expression_s LE expression_s
    { @$ = @3;SET_NODELOC(@3);$$ = leq($1, $3); }
    | expression_s '=' expression_s
    { @$ = @3;SET_NODELOC(@3);$$ = eq($1, $3); }
    | NOT expression_s
    { @$ = @2;SET_NODELOC(@2);$$ = comp($2); }
    | '(' expression_s ')'
    { @$ = @3;SET_NODELOC(@3);$$ =$2; }
    | OBJECTID
    { @$ = @1;SET_NODELOC(@1);$$ = object($1); }
    | INT_CONST
    { @$ = @1;SET_NODELOC(@1);$$ = int_const($1); }
    | STR_CONST
    { @$ = @1;SET_NODELOC(@1);$$ = string_const($1); }
    | BOOL_CONST
    { @$ = @1;SET_NODELOC(@1);$$ = bool_const($1); }
    ;

    case_list
    : case_expr /* single case */
    { @$ = @1;SET_NODELOC(@1);$$ = single_Cases($1); }
    | case_list case_expr
    { @$ = @2;SET_NODELOC(@2);$$ = append_Cases($1, single_Cases($2)); }
    ;

    case_expr
    : OBJECTID ':' TYPEID DARROW expression_s ';'
    { @$ = @6;SET_NODELOC(@6);$$ = branch($1, $3, $5); }
    ;

    let_assign_nested
    : let_assign_nested_left_part OBJECTID ':' TYPEID IN expression_s
    { @$ = @6;SET_NODELOC(@6);let_stack_.push(let_stack_elem_($2, $4, no_expr(), $6)); }
    | let_assign_nested_left_part OBJECTID ':' TYPEID ASSIGN expression_s IN expression_s
    { @$ = @8;SET_NODELOC(@8);let_stack_.push(let_stack_elem_($2, $4, $6, $8)); }
    | let_assign_nested_left_part error IN expression_s
    { @$ = @4;SET_NODELOC(@4); }
    ;

    let_assign_nested_left_part
    : /* empty expression */
    { /* do nothing */ }
    | let_assign_nested_left_part OBJECTID ':' TYPEID ','
    { @$ = @4;SET_NODELOC(@4);let_stack_.push(let_stack_elem_($2, $4, no_expr(), NULL)); }
    | let_assign_nested_left_part OBJECTID ':' TYPEID ASSIGN expression_s ','
    { @$ = @7;SET_NODELOC(@7);let_stack_.push(let_stack_elem_($2, $4, $6, NULL)); }
    | let_assign_nested_left_part error ',' 
    { @$ = @3;SET_NODELOC(@3); }
    ;

    expressions_by_comma
    : /* empty */
    { $$ = nil_Expressions(); }
    | expression_s
    { @$ = @1;SET_NODELOC(@1);$$ = single_Expressions($1); }
    | expressions_by_comma ',' expression_s
    { @$ = @3;SET_NODELOC(@3);$$ = append_Expressions($1, single_Expressions($3)); }
    ;

    expression_by_colon
    : expression_s
    { @$ = @1;SET_NODELOC(@1);$$ = single_Expressions($1); }
    | expression_by_colon ';' expression_s
    { @$ = @3;SET_NODELOC(@3);$$ = append_Expressions($1, single_Expressions($3)); }
    | error ';' expression_s
    { @$ = @3;SET_NODELOC(@3); $$ = single_Expressions($3); }
    | expression_by_colon ';' error 
    { @$ = @3;SET_NODELOC(@3);$$ = $1; }
    ;

    /* end of grammar */
    %%
    
    /* This function is called automatically when Bison detects a parse error. */
    void yyerror(char *s)
    {
      extern int curr_lineno;
      
      cerr << "\"" << curr_filename << "\", line " << curr_lineno << ": " \
      << s << " at or near ";
      print_cool_token(yychar);
      cerr << endl;
      omerrs++;
      
      if(omerrs>50) {fprintf(stdout, "More than 50 errors\n"); exit(1);}
    }
    
    