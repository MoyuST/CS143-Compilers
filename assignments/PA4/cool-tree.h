#ifndef COOL_TREE_H
#define COOL_TREE_H
//////////////////////////////////////////////////////////
//
// file: cool-tree.h
//
// This file defines classes for each phylum and constructor
//
//////////////////////////////////////////////////////////


#include "semant.h"
#include "tree.h"
#include "cool-tree.handcode.h"
#include "stringtab.h"
#include <string>
#include <vector>
#include <map>
#include <iostream>

// class ClassTable;
template <class SYM, class DAT>
class SymbolTable;

class ClassTable;
typedef ClassTable *ClassTableP;

class program_class;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable {
private:
  typedef struct _str_node_ {
    std::string _name;
    _str_node_* _parent;
    std::vector<_str_node_*> _children;
    _str_node_(){
      _name="";
      _parent=NULL;
      _children.clear();
    }
  } _str_node;

  int semant_errors;
  void install_basic_classes();
  ostream& error_stream;
  std::map<std::string, Symbol> _typename_symbol_pair;
  std::map<std::string, _str_node*> _typename_node_pair;
  std::map<std::string, std::string> _child_parent_pair;
  _str_node* _ast_root;

public:
  ClassTable(Classes);
  int errors() { return semant_errors; }
  ostream& semant_error();
  ostream& semant_error(Class_ c);
  ostream& semant_error(Symbol filename, tree_node *t);
  bool constuct_inherit_tree(Classes ast_root_classes);
  bool steq(std::string left, std::string right, std::string cur_type); // smaller and equal to
  std::string lub(std::string left, std::string right, std::string cur_type); // least upper bound

  std::map<std::string, std::string> class_attribute_type;
  std::map<std::string, std::vector<std::string> > class_method_signature;

  // type checking environment
  Class_ C;
  // M is not necessarily needed which is covered by class_method_signature

};

// define the class for phylum
// define simple phylum - Program
typedef class Program_class *Program;

class Program_class : public tree_node {
public:
   tree_node *copy()		 { return copy_Program(); }
   virtual Program copy_Program() = 0;
   void parse(ClassTable * class_tbl);

#ifdef Program_EXTRAS
   Program_EXTRAS
#endif
};


// define simple phylum - Class_
typedef class Class__class *Class_;

class Class__class : public tree_node {
public:
   tree_node *copy()		 { return copy_Class_(); }
   virtual Class_ copy_Class_() = 0;
   virtual void parse(SymbolTable<std::string, std::string>& O, ClassTable * class_tbl) = 0;
   virtual Symbol get_name()=0;
   virtual Symbol get_parent_name()=0;
   virtual Symbol get_file_name()=0;
   virtual std::vector<std::vector<std::vector<std::string> > > get_attr_method()=0;

#ifdef Class__EXTRAS
   Class__EXTRAS
#endif
};


// define simple phylum - Feature
typedef class Feature_class *Feature;

class Feature_class : public tree_node {
public:
   tree_node *copy()		 { return copy_Feature(); }
   virtual Feature copy_Feature() = 0;
   virtual std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl) = 0;
   virtual std::vector<std::string> get_parameters()=0;

#ifdef Feature_EXTRAS
   Feature_EXTRAS
#endif
};


// define simple phylum - Formal
typedef class Formal_class *Formal;

class Formal_class : public tree_node {
public:
   tree_node *copy()		 { return copy_Formal(); }
   virtual Formal copy_Formal() = 0;
   virtual std::string add_parameter(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl,
               std::vector<std::string *>& str_ptrs) = 0;
   virtual std::string get_type()=0;

#ifdef Formal_EXTRAS
   Formal_EXTRAS
#endif
};


// define simple phylum - Expression
typedef class Expression_class *Expression;

class Expression_class : public tree_node {
public:
   tree_node *copy()		 { return copy_Expression(); }
   virtual std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl) = 0;
   virtual Expression copy_Expression() = 0;

#ifdef Expression_EXTRAS
   Expression_EXTRAS
#endif
};


// define simple phylum - Case
typedef class Case_class *Case;

class Case_class : public tree_node {
public:
   tree_node *copy()		 { return copy_Case(); }
   virtual Case copy_Case() = 0;
   virtual std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl) = 0;

#ifdef Case_EXTRAS
   Case_EXTRAS
#endif
};


// define the class for phylum - LIST
// define list phlyum - Classes
typedef list_node<Class_> Classes_class;
typedef Classes_class *Classes;


// define list phlyum - Features
typedef list_node<Feature> Features_class;
typedef Features_class *Features;


// define list phlyum - Formals
typedef list_node<Formal> Formals_class;
typedef Formals_class *Formals;


// define list phlyum - Expressions
typedef list_node<Expression> Expressions_class;
typedef Expressions_class *Expressions;


// define list phlyum - Cases
typedef list_node<Case> Cases_class;
typedef Cases_class *Cases;


// define the class for constructors
// define constructor - program
class program_class : public Program_class {
protected:
   Classes classes;
public:
   program_class(Classes a1) {
      classes = a1;
   }
   Program copy_Program();
   void dump(ostream& stream, int n);
   void parse(SymbolTable<std::string, std::string>& O, ClassTable * class_tbl){
      for(int i = classes->first(); classes->more(i); i = classes->next(i)){
         classes->nth(i)->parse(O, class_tbl);
      }
   }
   Classes get_classses() {return classes;}

#ifdef Program_SHARED_EXTRAS
   Program_SHARED_EXTRAS
#endif
#ifdef program_EXTRAS
   program_EXTRAS
#endif
};


// define constructor - class_
class class__class : public Class__class {
protected:
   Symbol name;
   Symbol parent;
   Features features;
   Symbol filename;
public:
   class__class(Symbol a1, Symbol a2, Features a3, Symbol a4) {
      name = a1;
      parent = a2;
      features = a3;
      filename = a4;
   }
   Class_ copy_Class_();
   void dump(ostream& stream, int n);
   Symbol get_name() {return name;};
   Symbol get_parent_name() {return parent;};
   Symbol get_file_name() {return filename;};
   std::vector<std::vector<std::vector<std::string> > > get_attr_method(){
      std::vector<std::vector<std::string> > attr;
      std::vector<std::vector<std::string> > method;

      for(int i = features->first(); features->more(i); i = features->next(i)){
         // auto cur_rt = features->nth(i)->get_parameters();
         if(features->nth(i)->get_parameters()[0]=="attribute"){
            std::vector<std::string> element;
            element.push_back(features->nth(i)->get_parameters()[1]);
            element.push_back(features->nth(i)->get_parameters()[2]);
            attr.push_back(element);
         }
         else{
            std::vector<std::string> element;
            for(size_t j=1;j<features->nth(i)->get_parameters().size();j++){
               element.push_back(features->nth(i)->get_parameters()[j]);
            }
            method.push_back(element);
         }
      }

      std::vector<std::vector<std::vector<std::string> > > rt_vecs;
      rt_vecs.push_back(attr);
      rt_vecs.push_back(method);

      return rt_vecs;

   }

   void parse(SymbolTable<std::string, std::string>& O, ClassTable * class_tbl){

      for(int i = features->first(); features->more(i); i = features->next(i)){
         features->nth(i)->rt_type(
               O,
               class_tbl->class_method_signature,
               this,
               class_tbl
         );
      }
   }


#ifdef Class__SHARED_EXTRAS
   Class__SHARED_EXTRAS
#endif
#ifdef class__EXTRAS
   class__EXTRAS
#endif
};


// define constructor - method
class method_class : public Feature_class {
protected:
   Symbol name;
   Formals formals;
   Symbol return_type;
   Expression expr;
public:
   method_class(Symbol a1, Formals a2, Symbol a3, Expression a4) {
      name = a1;
      formals = a2;
      return_type = a3;
      expr = a4;
   }
   Feature copy_Feature();
   void dump(ostream& stream, int n);
   std::vector<std::string> get_parameters(){
      std::vector<std::string> rt;
      rt.push_back("method");
      rt.push_back(std::string(name->get_string()));
      for(int i = formals->first(); formals->more(i); i = formals->next(i)){
         rt.push_back(formals->nth(i)->get_type());
      }
      rt.push_back(std::string(return_type->get_string()));
      return rt;
   }
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::vector<std::string*> str_ptrs;
      std::string rt_type_name(return_type->get_string());
      O.enterscope();
      for(int i = formals->first(); formals->more(i); i = formals->next(i)){
         formals->nth(i)->add_parameter(O, M, C, class_tbl, str_ptrs);
      }
      std::string expr_rt_type_name(expr->rt_type(O, M, C, class_tbl));
      for(size_t i=0;i<str_ptrs.size();i++){
         delete str_ptrs[i];
         str_ptrs[i]=NULL;
      }
      O.exitscope();
      if(!class_tbl->steq(expr_rt_type_name, rt_type_name, std::string(C->get_name()->get_string()))){
         class_tbl->semant_error(C) << " method " <<  std::string(name->get_string())
         << " is expected to have return type " << expr_rt_type_name
         << " but instead has type " << rt_type_name << "\n";
      }
      return expr_rt_type_name;
   }

#ifdef Feature_SHARED_EXTRAS
   Feature_SHARED_EXTRAS
#endif
#ifdef method_EXTRAS
   method_EXTRAS
#endif
};


// define constructor - attr
class attr_class : public Feature_class {
protected:
   Symbol name;
   Symbol type_decl;
   Expression init;
public:
   attr_class(Symbol a1, Symbol a2, Expression a3) {
      name = a1;
      type_decl = a2;
      init = a3;
   }
   Feature copy_Feature();
   void dump(ostream& stream, int n);
   std::vector<std::string> get_parameters(){
      std::vector<std::string> rt;
      rt.push_back("attribute");
      rt.push_back(std::string(name->get_string()));
      rt.push_back(std::string(type_decl->get_string()));
      return rt;
   }
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string x(name->get_string());
      std::string T(type_decl->get_string());
      O.enterscope();
      std::string * str_ptr = new std::string("SELF_TYPE");
      O.addid("self", str_ptr);
      std::string init_rt_type = init->rt_type(O, M, C, class_tbl);
      if(init_rt_type!="no_expr"){
         if(!class_tbl->steq(init_rt_type, T, std::string(C->get_name()->get_string()))){
            class_tbl->semant_error(C) << " attribute " << x << " of type " << T
            << " is assigned with " << " expression of returning type of " << init_rt_type << "\n";
         }
      }
      O.exitscope();
      delete str_ptr;
      str_ptr=NULL;
      return T;
   }

#ifdef Feature_SHARED_EXTRAS
   Feature_SHARED_EXTRAS
#endif
#ifdef attr_EXTRAS
   attr_EXTRAS
#endif
};


// define constructor - formal
class formal_class : public Formal_class {
protected:
   Symbol name;
   Symbol type_decl;
public:
   formal_class(Symbol a1, Symbol a2) {
      name = a1;
      type_decl = a2;
   }
   Formal copy_Formal();
   void dump(ostream& stream, int n);
   std::string get_type(){
      return std::string(type_decl->get_string());
   }
   std::string add_parameter(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl,
               std::vector<std::string *>& str_ptrs)
   {
      std::string * str_ptr = new std::string(type_decl->get_string());
      str_ptrs.push_back(str_ptr);
      O.addid(std::string(C->get_name()->get_string())+" "+std::string(name->get_string()), str_ptr);
      return std::string(type_decl->get_string());
   }


#ifdef Formal_SHARED_EXTRAS
   Formal_SHARED_EXTRAS
#endif
#ifdef formal_EXTRAS
   formal_EXTRAS
#endif
};


// define constructor - branch
class branch_class : public Case_class {
protected:
   Symbol name;
   Symbol type_decl;
   Expression expr;
public:
   branch_class(Symbol a1, Symbol a2, Expression a3) {
      name = a1;
      type_decl = a2;
      expr = a3;
   }
   Case copy_Case();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string x(name->get_string());
      std::string T(type_decl->get_string());
      O.enterscope();
      std::string * str_ptr = new std::string(T);
      O.addid(x, str_ptr);
      std::string rt_type_name = expr->rt_type(O, M, C, class_tbl);
      O.exitscope();
      delete str_ptr;
      str_ptr=NULL;
      // type = idtable.lookup_string(const_cast<char*>(rt_type_name));
      return rt_type_name;
   }

#ifdef Case_SHARED_EXTRAS
   Case_SHARED_EXTRAS
#endif
#ifdef branch_EXTRAS
   branch_EXTRAS
#endif
};


// define constructor - assign
class assign_class : public Expression_class {
protected:
   Symbol name;
   Expression expr;
public:
   assign_class(Symbol a1, Expression a2) {
      name = a1;
      expr = a2;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string class_name_str(C->get_name()->get_string());
      std::string name_str(name->get_string());
      std::string symbol_type_name;
      std::string type_name;
      name_str=class_name_str+" "+name_str;
      if(O.lookup(name_str)!=NULL){
         symbol_type_name = *(O.lookup(name_str));
      }
      else{
         symbol_type_name = "Object";
         class_tbl->semant_error(C) << "assignment operation fails becasue " <<
         name_str << " is not defined in current scope\n";
      }

      std::string expr_type_name = expr->rt_type(O, M, C, class_tbl);

      if(class_tbl->steq(symbol_type_name, expr_type_name, std::string(C->get_name()->get_string()))){
         type_name = expr_type_name;
      }
      else{
         type_name = "Object";
         class_tbl->semant_error(C) << symbol_type_name << " <= " << expr_type_name << " is not heritant\n";
      }

      type = idtable.lookup_string(const_cast<char*>(type_name.c_str()));
      return type_name;
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef assign_EXTRAS
   assign_EXTRAS
#endif
};


// define constructor - static_dispatch
class static_dispatch_class : public Expression_class {
protected:
   Expression expr;
   Symbol type_name;
   Symbol name;
   Expressions actual;
public:
   static_dispatch_class(Expression a1, Symbol a2, Symbol a3, Expressions a4) {
      expr = a1;
      type_name = a2;
      name = a3;
      actual = a4;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string T_0 = expr->rt_type(O, M, C, class_tbl);
      std::vector<std::string> T_1_to_N;
      for(int i = actual->first(); actual->more(i); i = actual->next(i)){
         T_1_to_N.push_back(actual->nth(i)->rt_type(O, M, C, class_tbl));
      }

      std::string method(name->get_string());
      std::string T(type_name->get_string());
      std::string T_method = T+" "+method;

      if(!class_tbl->steq(T_0, T, std::string(C->get_name()->get_string()))){
         class_tbl->semant_error(C) << "in static dispacth of " << method << " but "
         << T_0 << " is larger than " << T << " \n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      if(M.find(T_method)==M.end()){
         class_tbl->semant_error(C) << "method " << method << " is not defined in class "
         << T << " \n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      std::vector<std::string> required_sig = M[T_method];

      if(required_sig.size()!=T_1_to_N.size()+1){
         class_tbl->semant_error(C) << "calling method " << method << " in "
         << T << " with incorrect parameter numbers" << " \n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      size_t param_num=0, i=0;
      for(;i<T_1_to_N.size();i++){
         param_num++;
         if(!class_tbl->steq(T_1_to_N[i], required_sig[i], std::string(C->get_name()->get_string()))){
            class_tbl->semant_error(C) << "calling method " << method << " in "
            << T << " but the " << param_num << "th is needed to be less and equal to type " 
            << required_sig[i] << " while the input type is " << T_1_to_N[i] << " \n";
            type = idtable.lookup_string("Object");
            return "Object";
         }
      }

      // may need to check returning type
      std::string T_N_plus_1 = (required_sig[i]=="SELF_TYPE") ? T_0 : required_sig[i];
      type = idtable.lookup_string(const_cast<char*>(T_N_plus_1.c_str()));
      return T_N_plus_1;
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef static_dispatch_EXTRAS
   static_dispatch_EXTRAS
#endif
};


// define constructor - dispatch
class dispatch_class : public Expression_class {
protected:
   Expression expr;
   Symbol name;
   Expressions actual;
public:
   dispatch_class(Expression a1, Symbol a2, Expressions a3) {
      expr = a1;
      name = a2;
      actual = a3;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string T_0 = expr->rt_type(O, M, C, class_tbl);
      std::vector<std::string> T_1_to_N;
      for(int i = actual->first(); actual->more(i); i = actual->next(i)){
         T_1_to_N.push_back(actual->nth(i)->rt_type(O, M, C, class_tbl));
      }

      std::string T_0_ = (T_0=="SELF_TYPE") ? std::string(C->get_name()->get_string()) : T_0;
      std::string method(name->get_string());
      std::string T_0_method = T_0_+" "+method;

      if(M.find(T_0_method)==M.end()){
         class_tbl->semant_error(C) << "method " << method << " is not defined in class "
         << T_0_ << " \n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      std::vector<std::string> required_sig = M[T_0_method];

      if(required_sig.size()!=T_1_to_N.size()+1){
         class_tbl->semant_error(C) << "calling method " << method << " in "
         << T_0_ << " with incorrect parameter numbers" << " \n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      size_t param_num=0, i=0;
      for(;i<T_1_to_N.size();i++){
         param_num++;
         if(!class_tbl->steq(T_1_to_N[i], required_sig[i], std::string(C->get_name()->get_string()))){
            class_tbl->semant_error(C) << "calling method " << method << " in "
            << T_0_ << " but the " << param_num << "th is needed to be less and equal to type " 
            << required_sig[i] << " while the input type is " << T_1_to_N[i] << " \n";
            type = idtable.lookup_string("Object");
            return "Object";
         }
      }

      // may need to check returning type
      std::string T_N_plus_1 = (required_sig[i]=="SELF_TYPE") ? T_0 : required_sig[i];
      type = idtable.lookup_string(const_cast<char*>(T_N_plus_1.c_str()));
      return T_N_plus_1;
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef dispatch_EXTRAS
   dispatch_EXTRAS
#endif
};


// define constructor - cond
class cond_class : public Expression_class {
protected:
   Expression pred;
   Expression then_exp;
   Expression else_exp;
public:
   cond_class(Expression a1, Expression a2, Expression a3) {
      pred = a1;
      then_exp = a2;
      else_exp = a3;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string pred_type = pred->rt_type(O, M, C, class_tbl);
      if(pred_type!="Bool"){
         class_tbl->semant_error(C) << "conditional statement with predicate type which "
         << "is not Bool\n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      std::string T2 = then_exp->rt_type(O, M, C, class_tbl);
      std::string T3 = else_exp->rt_type(O, M, C, class_tbl);
      std::string rt_type_name = class_tbl->lub(T2, T3, std::string(C->get_name()->get_string()));
      type = idtable.lookup_string(const_cast<char*>(rt_type_name.c_str()));
      return rt_type_name;
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef cond_EXTRAS
   cond_EXTRAS
#endif
};


// define constructor - loop
class loop_class : public Expression_class {
protected:
   Expression pred;
   Expression body;
public:
   loop_class(Expression a1, Expression a2) {
      pred = a1;
      body = a2;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      if(pred->rt_type(O, M, C, class_tbl)!="Bool"){
         class_tbl->semant_error(C) << " "
         << "is not Bool\n";
         type = idtable.lookup_string("Object");
         return "Object";
      }
      body->rt_type(O, M, C, class_tbl);
      type = idtable.lookup_string("Object");
      return "Object";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef loop_EXTRAS
   loop_EXTRAS
#endif
};


// define constructor - typcase
class typcase_class : public Expression_class {
protected:
   Expression expr;
   Cases cases;
public:
   typcase_class(Expression a1, Cases a2) {
      expr = a1;
      cases = a2;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      expr->rt_type(O, M, C, class_tbl);
      std::vector<std::string> case_types;
      for(int i = cases->first(); cases->more(i); i = cases->next(i)){
         case_types.push_back(cases->nth(i)->rt_type(O, M, C, class_tbl));
      }
      std::string rt_type_name="";
      for(size_t i=0;i<case_types.size();i++){
         if(i==0){
            rt_type_name=case_types[0];
         }
         else{
            rt_type_name=class_tbl->lub(rt_type_name, case_types[i], std::string(C->get_name()->get_string()));
         }
      }
      /* no case is specified */
      if(rt_type_name==""){
         rt_type_name="Object";
      }
      type = idtable.lookup_string(const_cast<char*>(rt_type_name.c_str()));
      return rt_type_name;
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef typcase_EXTRAS
   typcase_EXTRAS
#endif
};


// define constructor - block
class block_class : public Expression_class {
protected:
   Expressions body;
public:
   block_class(Expressions a1) {
      body = a1;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string rt_type("Object");
      for(int i = body->first(); body->more(i); i = body->next(i)){
         rt_type = body->nth(i)->rt_type(O, M, C, class_tbl);
      }
      type = idtable.lookup_string(const_cast<char*>(rt_type.c_str()));
      return rt_type;
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef block_EXTRAS
   block_EXTRAS
#endif
};


// define constructor - let
class let_class : public Expression_class {
protected:
   Symbol identifier;
   Symbol type_decl;
   Expression init;
   Expression body;
public:
   let_class(Symbol a1, Symbol a2, Expression a3, Expression a4) {
      identifier = a1;
      type_decl = a2;
      init = a3;
      body = a4;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string T_0(type_decl->get_string());
      std::string init_type(init->rt_type(O, M, C, class_tbl));
      if(init_type!="no_expr"){
         if(!class_tbl->steq(init_type, T_0, std::string(C->get_name()->get_string()))){
            class_tbl->semant_error(C) << "let initialize with wrong type where " << init_type << " <= " << T_0
            << " is not satisfied\n";
            type = idtable.lookup_string("Object");
            return "Object";
         }
      }
      std::string x(C->get_name()->get_string());
      x+=" ";
      x+=identifier->get_string();
      O.enterscope();
      std::string * str_ptr = new std::string(T_0);
      O.addid(x, str_ptr);
      std::string rt_type_name = body->rt_type(O, M, C, class_tbl);
      O.exitscope();
      delete str_ptr;
      str_ptr=NULL;
      type = idtable.lookup_string(const_cast<char*>(rt_type_name.c_str()));
      return rt_type_name;
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef let_EXTRAS
   let_EXTRAS
#endif
};


// define constructor - plus
class plus_class : public Expression_class {
protected:
   Expression e1;
   Expression e2;
public:
   plus_class(Expression a1, Expression a2) {
      e1 = a1;
      e2 = a2;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);

   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      if(e1->rt_type(O, M, C, class_tbl)!="Int"){
         class_tbl->semant_error(C) << "plus operation involving non interger left operand\n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      if(e2->rt_type(O, M, C, class_tbl)!="Int"){
         class_tbl->semant_error(C) << "plus operation involving non interger right operand\n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      type = idtable.lookup_string("Int");
      return "Int";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef plus_EXTRAS
   plus_EXTRAS
#endif
};


// define constructor - sub
class sub_class : public Expression_class {
protected:
   Expression e1;
   Expression e2;
public:
   sub_class(Expression a1, Expression a2) {
      e1 = a1;
      e2 = a2;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      if(e1->rt_type(O, M, C, class_tbl)!="Int"){
         class_tbl->semant_error(C) << "plus operation involving non interger left operand\n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      if(e2->rt_type(O, M, C, class_tbl)!="Int"){
         class_tbl->semant_error(C) << "plus operation involving non interger right operand\n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      type = idtable.lookup_string("Int");
      return "Int";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef sub_EXTRAS
   sub_EXTRAS
#endif
};


// define constructor - mul
class mul_class : public Expression_class {
protected:
   Expression e1;
   Expression e2;
public:
   mul_class(Expression a1, Expression a2) {
      e1 = a1;
      e2 = a2;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      if(e1->rt_type(O, M, C, class_tbl)!="Int"){
         class_tbl->semant_error(C) << "plus operation involving non interger left operand\n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      if(e2->rt_type(O, M, C, class_tbl)!="Int"){
         class_tbl->semant_error(C) << "plus operation involving non interger right operand\n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      type = idtable.lookup_string("Int");
      return "Int";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef mul_EXTRAS
   mul_EXTRAS
#endif
};


// define constructor - divide
class divide_class : public Expression_class {
protected:
   Expression e1;
   Expression e2;
public:
   divide_class(Expression a1, Expression a2) {
      e1 = a1;
      e2 = a2;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      if(e1->rt_type(O, M, C, class_tbl)!="Int"){
         class_tbl->semant_error(C) << "plus operation involving non interger left operand\n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      if(e2->rt_type(O, M, C, class_tbl)!="Int"){
         class_tbl->semant_error(C) << "plus operation involving non interger right operand\n";
         type = idtable.lookup_string("Object");
         return "Object";
      }

      type = idtable.lookup_string("Int");
      return "Int";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef divide_EXTRAS
   divide_EXTRAS
#endif
};


// define constructor - neg
class neg_class : public Expression_class {
protected:
   Expression e1;
public:
   neg_class(Expression a1) {
      e1 = a1;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string operand_type(e1->rt_type(O, M, C, class_tbl));
      if(operand_type!="Int"){
         class_tbl->semant_error(C) << "negative operation on wrong type which is " 
            << operand_type << "\n";
            type = idtable.lookup_string("Object");
            return "Object";
      }
      type = idtable.lookup_string("Int");
      return "Int";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef neg_EXTRAS
   neg_EXTRAS
#endif
};


// define constructor - lt
class lt_class : public Expression_class {
protected:
   Expression e1;
   Expression e2;
public:
   lt_class(Expression a1, Expression a2) {
      e1 = a1;
      e2 = a2;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string operand_type(e1->rt_type(O, M, C, class_tbl));
      if(operand_type!="Int"){
         class_tbl->semant_error(C) << "negative operation on wrong type which is " 
            << operand_type << "\n";
            type = idtable.lookup_string("Object");
         return "Object";
      }
      type = idtable.lookup_string("Int");
      return "Int";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef lt_EXTRAS
   lt_EXTRAS
#endif
};


// define constructor - eq
class eq_class : public Expression_class {
protected:
   Expression e1;
   Expression e2;
public:
   eq_class(Expression a1, Expression a2) {
      e1 = a1;
      e2 = a2;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string T_1 = e1->rt_type(O, M, C, class_tbl);
      std::string T_2 = e1->rt_type(O, M, C, class_tbl);
      if(T_1!="Int"&&T_1!="String"&&T_1!="Bool"){
         class_tbl->semant_error(C) << "equal operation with left operand of type " 
            << T_1 << " which is expected to be Int, String or Bool\n";
            type = idtable.lookup_string("Object");
         return "Object";
      }
      if(T_2!="Int"&&T_2!="String"&&T_2!="Bool"){
         class_tbl->semant_error(C) << "equal operation with right operand of type " 
            << T_2 << " which is expected to be Int, String or Bool\n";
            type = idtable.lookup_string("Object");
         return "Object";
      }
      if(T_1!=T_2){
         class_tbl->semant_error(C) << "equal operation with " << T_1 << " and " << T_2 
            << " is not allowed\n";
            type = idtable.lookup_string("Object");
         return "Object";
      }

      type = idtable.lookup_string("Bool");
      return "Bool";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef eq_EXTRAS
   eq_EXTRAS
#endif
};


// define constructor - leq
class leq_class : public Expression_class {
protected:
   Expression e1;
   Expression e2;
public:
   leq_class(Expression a1, Expression a2) {
      e1 = a1;
      e2 = a2;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string operand_type(e1->rt_type(O, M, C, class_tbl));
      if(operand_type!="Int"){
         class_tbl->semant_error(C) << "negative operation on wrong type which is " 
            << operand_type << "\n";
            type = idtable.lookup_string("Object");
            return "Object";
      }
      type = idtable.lookup_string("Int");
      return "Int";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef leq_EXTRAS
   leq_EXTRAS
#endif
};


// define constructor - comp
class comp_class : public Expression_class {
protected:
   Expression e1;
public:
   comp_class(Expression a1) {
      e1 = a1;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string rt_type_name = e1->rt_type(O, M, C, class_tbl);
      type = idtable.lookup_string(const_cast<char*>(rt_type_name.c_str()));
      return rt_type_name;
   }
#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef comp_EXTRAS
   comp_EXTRAS
#endif
};


// define constructor - int_const
class int_const_class : public Expression_class {
protected:
   Symbol token;
public:
   int_const_class(Symbol a1) {
      token = a1;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      type = idtable.lookup_string("Int");
      return "Int";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef int_const_EXTRAS
   int_const_EXTRAS
#endif
};


// define constructor - bool_const
class bool_const_class : public Expression_class {
protected:
   Boolean val;
public:
   bool_const_class(Boolean a1) {
      val = a1;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      type = idtable.lookup_string("Bool");
      return "Bool";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef bool_const_EXTRAS
   bool_const_EXTRAS
#endif
};


// define constructor - string_const
class string_const_class : public Expression_class {
protected:
   Symbol token;
public:
   string_const_class(Symbol a1) {
      token = a1;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      type = idtable.lookup_string("String");
      return "String";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef string_const_EXTRAS
   string_const_EXTRAS
#endif
};


// define constructor - new_
class new__class : public Expression_class {
protected:
   Symbol type_name;
public:
   new__class(Symbol a1) {
      type_name = a1;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string type_name_str(type_name->get_string());
      type = idtable.lookup_string(const_cast<char*>(type_name_str.c_str()));
      return type_name_str;
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef new__EXTRAS
   new__EXTRAS
#endif
};


// define constructor - isvoid
class isvoid_class : public Expression_class {
protected:
   Expression e1;
public:
   isvoid_class(Expression a1) {
      e1 = a1;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      e1->rt_type(O, M, C, class_tbl);
      type = idtable.lookup_string("Bool");
      return "Bool";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef isvoid_EXTRAS
   isvoid_EXTRAS
#endif
};


// define constructor - no_expr
class no_expr_class : public Expression_class {
protected:
public:
   no_expr_class() {
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      return "no_expr";
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef no_expr_EXTRAS
   no_expr_EXTRAS
#endif
};


// define constructor - object
class object_class : public Expression_class {
protected:
   Symbol name;
public:
   object_class(Symbol a1) {
      name = a1;
   }
   Expression copy_Expression();
   void dump(ostream& stream, int n);
   std::string rt_type(
               SymbolTable<std::string, std::string> &O,
               std::map<std::string,std::vector<std::string> > &M,
               Class_ C,
               ClassTable * class_tbl)
   {
      std::string class_name_str(C->get_name()->get_string());
      std::string name_str(name->get_string());
      name_str=class_name_str+" "+name_str;
      std::string type_name;
      if(O.lookup(name_str)!=NULL){
         type_name = *(O.lookup(name_str));
      }
      else{
         type_name = "Object";
         class_tbl->semant_error(C) << "object operation (" <<
         name_str << ") is not defined in current scope\n";
      }

      type = idtable.lookup_string(const_cast<char*>(type_name.c_str()));
      return type_name;
   }

#ifdef Expression_SHARED_EXTRAS
   Expression_SHARED_EXTRAS
#endif
#ifdef object_EXTRAS
   object_EXTRAS
#endif
};


// define the prototypes of the interface
Classes nil_Classes();
Classes single_Classes(Class_);
Classes append_Classes(Classes, Classes);
Features nil_Features();
Features single_Features(Feature);
Features append_Features(Features, Features);
Formals nil_Formals();
Formals single_Formals(Formal);
Formals append_Formals(Formals, Formals);
Expressions nil_Expressions();
Expressions single_Expressions(Expression);
Expressions append_Expressions(Expressions, Expressions);
Cases nil_Cases();
Cases single_Cases(Case);
Cases append_Cases(Cases, Cases);
Program program(Classes);
Class_ class_(Symbol, Symbol, Features, Symbol);
Feature method(Symbol, Formals, Symbol, Expression);
Feature attr(Symbol, Symbol, Expression);
Formal formal(Symbol, Symbol);
Case branch(Symbol, Symbol, Expression);
Expression assign(Symbol, Expression);
Expression static_dispatch(Expression, Symbol, Symbol, Expressions);
Expression dispatch(Expression, Symbol, Expressions);
Expression cond(Expression, Expression, Expression);
Expression loop(Expression, Expression);
Expression typcase(Expression, Cases);
Expression block(Expressions);
Expression let(Symbol, Symbol, Expression, Expression);
Expression plus(Expression, Expression);
Expression sub(Expression, Expression);
Expression mul(Expression, Expression);
Expression divide(Expression, Expression);
Expression neg(Expression);
Expression lt(Expression, Expression);
Expression eq(Expression, Expression);
Expression leq(Expression, Expression);
Expression comp(Expression);
Expression int_const(Symbol);
Expression bool_const(Boolean);
Expression string_const(Symbol);
Expression new_(Symbol);
Expression isvoid(Expression);
Expression no_expr();
Expression object(Symbol);


#endif
