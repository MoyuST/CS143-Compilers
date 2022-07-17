
(*
 *  execute "coolc bad.cl" to see the error messages that the coolc parser
 *  generates
 *
 *  execute "myparser bad.cl" to see the error messages that your parser
 *  generates
 *)

(* no error *)
class A {
};

(* error:  b is not a type identifier *)
Class b inherits A {
};

(* error:  a is not a type identifier *)
Class C inherits a {
};

(* error:  keyword inherits is misspelled *)
Class D inherts A {
};

(* error:  closing brace is missing *)
Class E inherits A {
;

(* error:  closing brace is missing *)
Class F inherits A {
    correct1() : A { {1+2; 2+3;} }; (* correct *)
    error1 B : v; (* malformed *)
    correct2() : A { 1+2 };
    error2(w : C) : B { }; (* malformed *)
    letnormal() : A { let e : E, w: W, w : W in 1 };
    letmalformed() : A { let e : w, w: W, w : w in 1 };
    blocknormal() : A { {1+2;3+4;5;6;6;} };
    blockmalformed2() : A { {1+2;+4;5;6;6;} };
    blockmalformed3() : A { {1+2;+4;5;6;;} };
    blockmalformed4() : A { {;5;6;} };
    blocknormal2() : A { {5;6;} };
};