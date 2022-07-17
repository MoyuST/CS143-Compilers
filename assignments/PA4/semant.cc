

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include "semant.h"
#include "utilities.h"
#include <queue>
#include <set>
#include <vector>


extern int semant_debug;
extern char *curr_filename;

//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
static Symbol 
    arg,
    arg2,
    Bool,
    concat,
    cool_abort,
    copy,
    Int,
    in_int,
    in_string,
    IO,
    length,
    Main,
    main_meth,
    No_class,
    No_type,
    Object,
    out_int,
    out_string,
    prim_slot,
    self,
    SELF_TYPE,
    Str,
    str_field,
    substr,
    type_name,
    val;
//
// Initializing the predefined symbols.
//
static void initialize_constants(void)
{
    arg         = idtable.add_string("arg");
    arg2        = idtable.add_string("arg2");
    Bool        = idtable.add_string("Bool");
    concat      = idtable.add_string("concat");
    cool_abort  = idtable.add_string("abort");
    copy        = idtable.add_string("copy");
    Int         = idtable.add_string("Int");
    in_int      = idtable.add_string("in_int");
    in_string   = idtable.add_string("in_string");
    IO          = idtable.add_string("IO");
    length      = idtable.add_string("length");
    Main        = idtable.add_string("Main");
    main_meth   = idtable.add_string("main");
    //   _no_class is a symbol that can't be the name of any 
    //   user-defined class.
    No_class    = idtable.add_string("_no_class");
    No_type     = idtable.add_string("_no_type");
    Object      = idtable.add_string("Object");
    out_int     = idtable.add_string("out_int");
    out_string  = idtable.add_string("out_string");
    prim_slot   = idtable.add_string("_prim_slot");
    self        = idtable.add_string("self");
    SELF_TYPE   = idtable.add_string("SELF_TYPE");
    Str         = idtable.add_string("String");
    str_field   = idtable.add_string("_str_field");
    substr      = idtable.add_string("substr");
    type_name   = idtable.add_string("type_name");
    val         = idtable.add_string("_val");
}



ClassTable::ClassTable(Classes classes) : semant_errors(0) , error_stream(cerr) {
    
}

void ClassTable::install_basic_classes() {

    // The tree package uses these globals to annotate the classes built below.
   // curr_lineno  = 0;
    Symbol filename = stringtable.add_string("<basic class>");
    
    // The following demonstrates how to create dummy parse trees to
    // refer to basic Cool classes.  There's no need for method
    // bodies -- these are already built into the runtime system.
    
    // IMPORTANT: The results of the following expressions are
    // stored in local variables.  You will want to do something
    // with those variables at the end of this method to make this
    // code meaningful.

    // 
    // The Object class has no parent class. Its methods are
    //        abort() : Object    aborts the program
    //        type_name() : Str   returns a string representation of class name
    //        copy() : SELF_TYPE  returns a copy of the object
    //
    // There is no need for method bodies in the basic classes---these
    // are already built in to the runtime system.

    Class_ Object_class =
	class_(Object, 
	       No_class,
	       append_Features(
			       append_Features(
					       single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
					       single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
			       single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	       filename);

    // 
    // The IO class inherits from Object. Its methods are
    //        out_string(Str) : SELF_TYPE       writes a string to the output
    //        out_int(Int) : SELF_TYPE            "    an int    "  "     "
    //        in_string() : Str                 reads a string from the input
    //        in_int() : Int                      "   an int     "  "     "
    //
    Class_ IO_class = 
	class_(IO, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       single_Features(method(out_string, single_Formals(formal(arg, Str)),
										      SELF_TYPE, no_expr())),
							       single_Features(method(out_int, single_Formals(formal(arg, Int)),
										      SELF_TYPE, no_expr()))),
					       single_Features(method(in_string, nil_Formals(), Str, no_expr()))),
			       single_Features(method(in_int, nil_Formals(), Int, no_expr()))),
	       filename);  

    //
    // The Int class has no methods and only a single attribute, the
    // "val" for the integer. 
    //
    Class_ Int_class =
	class_(Int, 
	       Object,
	       single_Features(attr(val, prim_slot, no_expr())),
	       filename);

    //
    // Bool also has only the "val" slot.
    //
    Class_ Bool_class =
	class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename);

    //
    // The class Str has a number of slots and operations:
    //       val                                  the length of the string
    //       str_field                            the string itself
    //       length() : Int                       returns length of the string
    //       concat(arg: Str) : Str               performs string concatenation
    //       substr(arg: Int, arg2: Int): Str     substring selection
    //       
    Class_ Str_class =
	class_(Str, 
	       Object,
	       append_Features(
			       append_Features(
					       append_Features(
							       append_Features(
									       single_Features(attr(val, Int, no_expr())),
									       single_Features(attr(str_field, prim_slot, no_expr()))),
							       single_Features(method(length, nil_Formals(), Int, no_expr()))),
					       single_Features(method(concat, 
								      single_Formals(formal(arg, Str)),
								      Str, 
								      no_expr()))),
			       single_Features(method(substr, 
						      append_Formals(single_Formals(formal(arg, Int)), 
								     single_Formals(formal(arg2, Int))),
						      Str, 
						      no_expr()))),
	       filename);
}

////////////////////////////////////////////////////////////////////
//
// semant_error is an overloaded function for reporting errors
// during semantic analysis.  There are three versions:
//
//    ostream& ClassTable::semant_error()                
//
//    ostream& ClassTable::semant_error(Class_ c)
//       print line number and filename for `c'
//
//    ostream& ClassTable::semant_error(Symbol filename, tree_node *t)  
//       print a line number and filename
//
///////////////////////////////////////////////////////////////////

ostream& ClassTable::semant_error(Class_ c)
{                                                             
    return semant_error(c->get_filename(),c);
}    

ostream& ClassTable::semant_error(Symbol filename, tree_node *t)
{
    error_stream << filename << ":" << t->get_line_number() << ": ";
    return semant_error();
}

ostream& ClassTable::semant_error()                  
{                                                 
    semant_errors++;                            
    return error_stream;
} 

bool ClassTable::constuct_inherit_tree(Classes ast_root_classes)
{
    // collect information of <child_class_name, symbol_in_idtable>
    //                        <child_class_name, parent_class_name>
    //                        <class_name\battribute_name, type_name>
    //                        <class_name\bmethod_name, (sig1, sig2, ..., return_type)>
    std::map<std::string, std::string> class_attribute_type_one_level;
    std::map<std::string, std::vector<std::string> > class_method_signature_one_level;
    std::map<std::string, std::vector<std::string> > class_attr_list;
    std::map<std::string, std::vector<std::string> > class_method_list;
    
    for(int i = ast_root_classes->first(); ast_root_classes->more(i); i = ast_root_classes->next(i)){
        std::string class_name(ast_root_classes->nth(i)->get_name()->get_string());
        // cannot inherit or redefine Int, String and Bool
        if(class_name=="Int"||class_name=="String"||class_name=="Bool"){
            error_stream << ast_root_classes->nth(i)->get_file_name() << ":" << class_name 
            << "cannot inherit or redefine class Int, String and Bool\n";
            assert(0);
            return false;
        }

        // check whether class is redefined
        if(_typename_symbol_pair.find(class_name)!=_typename_symbol_pair.end()){
            error_stream << ast_root_classes->nth(i)->get_file_name() << ":" << class_name << " class redefined\n";
            assert(0);
            return false;
        }
        _typename_symbol_pair[class_name] = idtable.add_string(const_cast<char*>(class_name.c_str()));
        std::string class_parent_name(ast_root_classes->nth(i)->get_parent_name()->get_string());
        _child_parent_pair[class_name] = class_parent_name;
        _typename_node_pair[class_name] = new _str_node();
        _typename_node_pair[class_name]->_name = class_name;
        _typename_node_pair[class_name]->_parent = NULL;

        std::vector<std::vector<std::vector<std::string> > > rt 
            = ast_root_classes->nth(i)->get_attr_method();
        // recording methods and attributes inside class declaration excluding those from parents
        class_attr_list[class_name]=std::vector<std::string>();
        class_method_list[class_name]=std::vector<std::string>();
        // attributes
        for(size_t i=0;i<rt[0].size();i++){
            class_attr_list[class_name].push_back(rt[0][i][0]);
            class_attribute_type_one_level[class_name+" "+rt[0][i][0]]=rt[0][i][1];
        }
        // methods
        for(size_t i=0;i<rt[1].size();i++){
            class_method_list[class_name].push_back(rt[1][i][0]);
            class_method_signature_one_level[class_name+" "+rt[1][i][0]]=std::vector<std::string>(rt[1][i].begin()+1, rt[1][i].end());
        }
    }

    // check whether all parent classes are defined
    // construct inheritance tree and 
    for(std::map<std::string, std::string>::iterator it=_child_parent_pair.begin();it!=_child_parent_pair.end();it++){
        idtable.lookup_string(const_cast<char*>(it->second.c_str()));
        if(_typename_node_pair.find(it->second)==_typename_node_pair.end()){
            _typename_node_pair[it->second] = new _str_node();
            _typename_node_pair[it->second]->_name=it->second;
            _typename_node_pair[it->second]->_parent=NULL;
        }
        _typename_node_pair[it->first]->_parent=_typename_node_pair[it->second];
        _typename_node_pair[it->second]->_children.push_back(_typename_node_pair[it->first]);
    }

    // check whether inheritant tree is acyclic using BFS
    {
        std::queue<_str_node*> next_nodes;
        std::set<std::string> visited_nodes;
        next_nodes.push(_typename_node_pair["Object"]);
        while(!next_nodes.empty()){
            _str_node* cur_node = next_nodes.front();
            next_nodes.pop();
            // a circle exists
            if(visited_nodes.find(cur_node->_name)!=visited_nodes.end()){
                error_stream << " class inheritance tree is not acyclic\n";
                assert(0);
                return false;
            }
            visited_nodes.insert(cur_node->_name);
            for(std::vector<_str_node_*>::iterator it=cur_node->_children.begin();
                it!=cur_node->_children.end();it++){
                next_nodes.push(*it);
            }
        }
    }

    // inheritant tree is valid so far
    // building basic types
    std:std::vector<std::string> basic_types;
    basic_types.push_back("Object");
    basic_types.push_back("IO");
    basic_types.push_back("Int");
    basic_types.push_back("String");
    basic_types.push_back("Bool");

    for(size_t i=0;i<basic_types.size();i++){
        if(_typename_node_pair.find(basic_types[i])==_typename_node_pair.end()){
            _typename_node_pair[basic_types[i]] = new _str_node();
            _typename_node_pair[basic_types[i]]->_name=basic_types[i];
        }
    }

    /* Object */
    // if(_typename_node_pair.find("Object")==_typename_node_pair.end()){
    //     _typename_node_pair["Object"] = new _str_node();
    //     _typename_node_pair["Object"]->_name="Object";
    // }
    _typename_node_pair["Object"]->_parent=NULL;
    _ast_root = _typename_node_pair["Object"];

    class_attr_list["Object"]=std::vector<std::string>();
    class_method_list["Object"]=std::vector<std::string>();
    class_method_list["Object"].push_back("abort");
    class_method_list["Object"].push_back("type_name");
    class_method_list["Object"].push_back("copy");
    class_method_signature_one_level["Object abort"]=std::vector<std::string>();
    class_method_signature_one_level["Object abort"].push_back("Object");
    class_method_signature_one_level["Object type_name"]=std::vector<std::string>();
    class_method_signature_one_level["Object type_name"].push_back("String");
    class_method_signature_one_level["Object copy"]=std::vector<std::string>();
    class_method_signature_one_level["Object copy"].push_back("SELF_TYPE");

    /* IO */
    _typename_node_pair["IO"]->_parent=_typename_node_pair["Object"];
    _child_parent_pair["IO"]="Object";
    class_attr_list["IO"]=std::vector<std::string>();
    class_method_list["IO"]=std::vector<std::string>();
    class_method_list["IO"].push_back("out_string");
    class_method_list["IO"].push_back("out_int");
    class_method_list["IO"].push_back("in_string");
    class_method_list["IO"].push_back("in_int");
    class_method_signature_one_level["IO out_string"]=std::vector<std::string>();
    class_method_signature_one_level["IO out_string"].push_back("String");
    class_method_signature_one_level["IO out_string"].push_back("SELF_TYPE");
    class_method_signature_one_level["IO out_int"]=std::vector<std::string>();
    class_method_signature_one_level["IO out_int"].push_back("Int");
    class_method_signature_one_level["IO out_int"].push_back("SELF_TYPE");
    class_method_signature_one_level["IO in_string"]=std::vector<std::string>();
    class_method_signature_one_level["IO in_string"].push_back("String");
    class_method_signature_one_level["IO in_int"]=std::vector<std::string>();
    class_method_signature_one_level["IO in_int"].push_back("Int");

    /* Int */
    _typename_node_pair["Int"]->_parent=_typename_node_pair["Object"];
    _child_parent_pair["Int"]="Object";
    class_attr_list["Int"]=std::vector<std::string>();
    class_method_list["Int"]=std::vector<std::string>();

    /* String */
    _typename_node_pair["String"]->_parent=_typename_node_pair["Object"];
    _child_parent_pair["String"]="Object";
    class_attr_list["String"]=std::vector<std::string>();
    class_method_list["String"]=std::vector<std::string>();
    class_method_list["String"].push_back("length");
    class_method_list["String"].push_back("concat");
    class_method_list["String"].push_back("substr");
    class_method_signature_one_level["String length"]=std::vector<std::string>();
    class_method_signature_one_level["String length"].push_back("Int");
    class_method_signature_one_level["String concat"]=std::vector<std::string>();
    class_method_signature_one_level["String concat"].push_back("String");
    class_method_signature_one_level["String concat"].push_back("String");
    class_method_signature_one_level["String substr"]=std::vector<std::string>();
    class_method_signature_one_level["String substr"].push_back("Int");
    class_method_signature_one_level["String substr"].push_back("Int");
    class_method_signature_one_level["String substr"].push_back("String");

    /* Bool */
    _typename_node_pair["Bool"]->_parent=_typename_node_pair["Object"];
    _child_parent_pair["Bool"]="Object";
    class_attr_list["Bool"]=std::vector<std::string>();
    class_method_list["Bool"]=std::vector<std::string>();

    // generating complete attribute and method information for each class using BFS
    {
        std::queue<std::string> next_class;
        next_class.push("Object");
        while(!next_class.empty()){
            std::string cur_class = next_class.front();
            next_class.pop();
            for(std::map<std::string, std::string>::iterator it=_child_parent_pair.begin();it!=_child_parent_pair.end();it++){
                if(it->second==cur_class){
                    next_class.push(it->first);
                }
            }

            if(_typename_node_pair[cur_class]->_parent!=NULL){
                std::string parent_name = _typename_node_pair[cur_class]->_parent->_name;

                // copy inheritant attributes
                std::vector<std::string>& attr_list = class_attr_list[parent_name];
                for(size_t j=0;j<attr_list.size();j++){
                    class_attribute_type[cur_class+" "+attr_list[j]]
                        =class_attribute_type[parent_name+" "+attr_list[j]];
                }

                // copy inheritant methods
                std::vector<std::string>& method_list = class_method_list[parent_name];
                for(size_t j=0;j<method_list.size();j++){
                    class_method_signature[cur_class+" "+method_list[j]]
                        =class_method_signature[cur_class+" "+method_list[j]];
                }
            }

            // replace/add new attributes
            std::vector<std::string>& attr_list = class_attr_list[cur_class];
            for(size_t j=0;j<attr_list.size();j++){
                class_attribute_type[cur_class+" "+attr_list[j]]
                    =class_attribute_type_one_level[cur_class+" "+attr_list[j]];
            }

            // replace/add new methods
            std::vector<std::string>& method_list = class_method_list[cur_class];
            for(size_t j=0;j<method_list.size();j++){
                class_method_signature[cur_class+" "+method_list[j]]
                    =class_method_signature_one_level[cur_class+" "+method_list[j]];
            }

            // handle self type
            class_attribute_type[cur_class+" self"]
                ="SELF_TYPE";
        }
    }

    return true;
}

bool ClassTable::steq(std::string left, std::string right, std::string cur_type){
    if(left=="SELF_TYPE" && right=="SELF_TYPE"){
        return true;
    }

    if(left=="SELF_TYPE"){
        return steq(cur_type, right, cur_type);
    }

    if(right=="SELF_TYPE"){
        return false;
    }

    // check whether left is the child of the right
    _str_node * left_ptr = _typename_node_pair[left];

    while(left_ptr->_parent!=NULL){
        if(left_ptr->_name==right){
            return true;
        }
        left_ptr=left_ptr->_parent;
    }

    return false;
    
}

std::string ClassTable::lub(std::string left, std::string right, std::string cur_type){
    if(left=="SELF_TYPE" && right=="SELF_TYPE"){
        return "SELF_TYPE";
    }

    if(left=="SELF_TYPE"){
        return lub(cur_type, right, cur_type);
    }

    if(right=="SELF_TYPE"){
        return lub(cur_type, left, cur_type);
    }

    // handling normal lub (least upper bound)
    {
        std::set<std::string> left_parents;
        _str_node * left_ptr = _typename_node_pair[left];

        while(left_ptr->_parent!=NULL){
            left_parents.insert(left_ptr->_name);
            left_ptr=left_ptr->_parent;
        }

        _str_node * right_ptr=_typename_node_pair[right];
        while(right_ptr->_parent!=NULL){
            if(left_parents.find(right_ptr->_name)!=left_parents.end()){
                return right_ptr->_name;
            }
            right_ptr=right_ptr->_parent;
        }

    }

    // escaping compile error
    return "Object";

}

/*   This is the entry point to the semantic checker.

     Your checker should do the following two things:

     1) Check that the program is semantically correct
     2) Decorate the abstract syntax tree with type information
        by setting the `type' field in each Expression node.
        (see `tree.h')

     You are free to first do 1), make sure you catch all semantic
     errors. Part 2) can be done in a second stage, when you want
     to build mycoolc.
 */
void program_class::semant()
{
    initialize_constants();

    // first pass: 
    //              constructing inheritance tree
    //              collecting attributes and methods for each class
    /* ClassTable constructor may do some semantic analysis */
    ClassTable *classtable = new ClassTable(classes);
    classtable->constuct_inherit_tree(this->get_classses());
    SymbolTable<std::string, std::string> O;
    O.enterscope();
    std::vector<std::string *> str_ptrs;
    for(std::map<std::string, std::string>::iterator it=classtable->class_attribute_type.begin();
        it!=classtable->class_attribute_type.end();
        it++
    ){
        std::string * str_ptr = new std::string(it->second);
        str_ptrs.push_back(str_ptr);
        O.addid(it->first, str_ptr);
    }

    /* some semantic analysis code may go here */
    this->parse(O, classtable);

    O.exitscope();
    for(size_t i=0;i<str_ptrs.size();i++){
        delete str_ptrs[i];
        str_ptrs[i]=NULL;
    }

    if (classtable->errors()) {
	    cerr << "Compilation halted due to static semantic errors." << endl;
	    exit(1);
    }
}