
//**************************************************************
//
// Code generator SKELETON
//
// Read the comments carefully. Make sure to
//    initialize the base class tags in
//       `CgenClassTable::CgenClassTable'
//
//    Add the label for the dispatch tables to
//       `IntEntry::code_def'
//       `StringEntry::code_def'
//       `BoolConst::code_def'
//
//    Add code to emit everyting else that is needed
//       in `CgenClassTable::code'
//
//
// The files as provided will produce code to begin the code
// segments, declare globals, and emit constants.  You must
// fill in the rest.
//
//**************************************************************

#include "cgen.h"
#include "cgen_gc.h"
#include <memory>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <sys/stat.h>

extern void emit_string_constant(ostream& str, char *s);
extern int cgen_debug;
extern std::vector<StringEntryP> CgenClassTable::class_nameTab_entries;
extern std::map<StringEntryP, int> CgenClassTable::class_tag_pair;
extern int CgenClassTable::next_class_tag;
extern std::map<std::string, size_t> CgenClassTable::method_class_offset;

static SymbolTable<Symbol, StringEntry> cur_attr_posi_info;
static StrTable attr_offset_stringtable;
static int label_idx=0;
static CgenNodeP cur_node_global; // used when generating codes for method
static size_t next_temp_number_in_stack=0; // used to locate allocated local variable
static size_t total_number_of_temp_global=0; // used to record number of all temporaries of current method

//
// Three symbols from the semantic analyzer (semant.cc) are used.
// If e : No_type, then no code is generated for e.
// Special code is generated for new SELF_TYPE.
// The name "self" also generates code different from other references.
//
//////////////////////////////////////////////////////////////////////
//
// Symbols
//
// For convenience, a large number of symbols are predefined here.
// These symbols include the primitive type and method names, as well
// as fixed names used by the runtime system.
//
//////////////////////////////////////////////////////////////////////
Symbol 
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

static char *gc_init_names[] =
  { "_NoGC_Init", "_GenGC_Init", "_ScnGC_Init" };
static char *gc_collect_names[] =
  { "_NoGC_Collect", "_GenGC_Collect", "_ScnGC_Collect" };


//  BoolConst is a class that implements code generation for operations
//  on the two booleans, which are given global names here.
BoolConst falsebool(FALSE);
BoolConst truebool(TRUE);

//*********************************************************
//
// Define method for code generation
//
// This is the method called by the compiler driver
// `cgtest.cc'. cgen takes an `ostream' to which the assembly will be
// emmitted, and it passes this and the class list of the
// code generator tree to the constructor for `CgenClassTable'.
// That constructor performs all of the work of the code
// generator.
//
//*********************************************************

void program_class::cgen(ostream &os) 
{
  // spim wants comments to start with '#'
  // os << "# start of generated code\n";

  initialize_constants();
  CgenClassTable *codegen_classtable = new CgenClassTable(classes,os);

  // os << "\n# end of generated code\n";
}


//////////////////////////////////////////////////////////////////////////////
//
//  emit_* procedures
//
//  emit_X  writes code for operation "X" to the output stream.
//  There is an emit_X for each opcode X, as well as emit_ functions
//  for generating names according to the naming conventions (see emit.h)
//  and calls to support functions defined in the trap handler.
//
//  Register names and addresses are passed as strings.  See `emit.h'
//  for symbolic names you can use to refer to the strings.
//
//////////////////////////////////////////////////////////////////////////////

static void emit_load(char *dest_reg, int offset, char *source_reg, ostream& s)
{
  s << LW << dest_reg << " " << offset * WORD_SIZE << "(" << source_reg << ")" 
    << endl;
}

static void emit_store(char *source_reg, int offset, char *dest_reg, ostream& s)
{
  s << SW << source_reg << " " << offset * WORD_SIZE << "(" << dest_reg << ")"
      << endl;
}

static void emit_load_imm(char *dest_reg, int val, ostream& s)
{ s << LI << dest_reg << " " << val << endl; }

static void emit_load_address(char *dest_reg, char *address, ostream& s)
{ s << LA << dest_reg << " " << address << endl; }

static void emit_partial_load_address(char *dest_reg, ostream& s)
{ s << LA << dest_reg << " "; }

static void emit_load_bool(char *dest, const BoolConst& b, ostream& s)
{
  emit_partial_load_address(dest,s);
  b.code_ref(s);
  s << endl;
}

static void emit_load_string(char *dest, StringEntry *str, ostream& s)
{
  emit_partial_load_address(dest,s);
  str->code_ref(s);
  s << endl;
}

static void emit_load_int(char *dest, IntEntry *i, ostream& s)
{
  emit_partial_load_address(dest,s);
  i->code_ref(s);
  s << endl;
}

static void emit_move(char *dest_reg, char *source_reg, ostream& s)
{ s << MOVE << dest_reg << " " << source_reg << endl; }

static void emit_neg(char *dest, char *src1, ostream& s)
{ s << NEG << dest << " " << src1 << endl; }

static void emit_add(char *dest, char *src1, char *src2, ostream& s)
{ s << ADD << dest << " " << src1 << " " << src2 << endl; }

static void emit_addu(char *dest, char *src1, char *src2, ostream& s)
{ s << ADDU << dest << " " << src1 << " " << src2 << endl; }

static void emit_addiu(char *dest, char *src1, int imm, ostream& s)
{ s << ADDIU << dest << " " << src1 << " " << imm << endl; }

static void emit_div(char *dest, char *src1, char *src2, ostream& s)
{ s << DIV << dest << " " << src1 << " " << src2 << endl; }

static void emit_mul(char *dest, char *src1, char *src2, ostream& s)
{ s << MUL << dest << " " << src1 << " " << src2 << endl; }

static void emit_sub(char *dest, char *src1, char *src2, ostream& s)
{ s << SUB << dest << " " << src1 << " " << src2 << endl; }

static void emit_sll(char *dest, char *src1, int num, ostream& s)
{ s << SLL << dest << " " << src1 << " " << num << endl; }

static void emit_jalr(char *dest, ostream& s)
{ s << JALR << "\t" << dest << endl; }

static void emit_jal(char *address,ostream &s)
{ s << JAL << address << endl; }

static void emit_return(ostream& s)
{ s << RET << endl; }

static void emit_gc_assign(ostream& s)
{ s << JAL << "_GenGC_Assign" << endl; }

static void emit_disptable_ref(Symbol sym, ostream& s)
{  s << sym << DISPTAB_SUFFIX; }

static void emit_init_ref(Symbol sym, ostream& s)
{ s << sym << CLASSINIT_SUFFIX; }

static void emit_label_ref(int l, ostream &s)
{ s << "label" << l; }

static void emit_protobj_ref(Symbol sym, ostream& s)
{ s << sym << PROTOBJ_SUFFIX; }

static void emit_method_ref(Symbol classname, Symbol methodname, ostream& s)
{ s << classname << METHOD_SEP << methodname; }

static void emit_label_def(int l, ostream &s)
{
  emit_label_ref(l,s);
  s << ":" << endl;
}

static void emit_beqz(char *source, int label, ostream &s)
{
  s << BEQZ << source << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_beq(char *src1, char *src2, int label, ostream &s)
{
  s << BEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bne(char *src1, char *src2, int label, ostream &s)
{
  s << BNE << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bleq(char *src1, char *src2, int label, ostream &s)
{
  s << BLEQ << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blt(char *src1, char *src2, int label, ostream &s)
{
  s << BLT << src1 << " " << src2 << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_blti(char *src1, int imm, int label, ostream &s)
{
  s << BLT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_bgti(char *src1, int imm, int label, ostream &s)
{
  s << BGT << src1 << " " << imm << " ";
  emit_label_ref(label,s);
  s << endl;
}

static void emit_branch(int l, ostream& s)
{
  s << BRANCH;
  emit_label_ref(l,s);
  s << endl;
}

//
// Push a register on the stack. The stack grows towards smaller addresses.
//
static void emit_push(char *reg, ostream& str)
{
  emit_store(reg,0,SP,str);
  emit_addiu(SP,SP,-4,str);
}

//
// Fetch the integer value in an Int object.
// Emits code to fetch the integer value of the Integer object pointed
// to by register source into the register dest
//
static void emit_fetch_int(char *dest, char *source, ostream& s)
{ emit_load(dest, DEFAULT_OBJFIELDS, source, s); }

//
// Emits code to store the integer value contained in register source
// into the Integer object pointed to by dest.
//
static void emit_store_int(char *source, char *dest, ostream& s)
{ emit_store(source, DEFAULT_OBJFIELDS, dest, s); }


static void emit_test_collector(ostream &s)
{
  emit_push(ACC, s);
  emit_move(ACC, SP, s); // stack end
  emit_move(A1, ZERO, s); // allocate nothing
  s << JAL << gc_collect_names[cgen_Memmgr] << endl;
  emit_addiu(SP,SP,4,s);
  emit_load(ACC,0,SP,s);
}

static void emit_gc_check(char *source, ostream &s)
{
  if (source != (char*)A1) emit_move(A1, source, s);
  s << JAL << "_gc_check" << endl;
}


///////////////////////////////////////////////////////////////////////////////
//
// coding strings, ints, and booleans
//
// Cool has three kinds of constants: strings, ints, and booleans.
// This section defines code generation for each type.
//
// All string constants are listed in the global "stringtable" and have
// type StringEntry.  StringEntry methods are defined both for String
// constant definitions and references.
//
// All integer constants are listed in the global "inttable" and have
// type IntEntry.  IntEntry methods are defined for Int
// constant definitions and references.
//
// Since there are only two Bool values, there is no need for a table.
// The two booleans are represented by instances of the class BoolConst,
// which defines the definition and reference methods for Bools.
//
///////////////////////////////////////////////////////////////////////////////

//
// Strings
//
void StringEntry::code_ref(ostream& s)
{
  s << STRCONST_PREFIX << index;
}

//
// Emit code for a constant String.
// You should fill in the code naming the dispatch table.
//

void StringEntry::code_def(ostream& s, int stringclasstag)
{
  IntEntryP lensym = inttable.add_int(len);

  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s  << LABEL                                             // label
      << WORD << stringclasstag << endl                                 // tag
      << WORD << (DEFAULT_OBJFIELDS + STRING_SLOTS + (len+4)/4) << endl // size
      << WORD;


 /***** Add dispatch information for class String ******/
      s << "String" << DISPTAB_SUFFIX;

      s << endl;                                              // dispatch table
      s << WORD;  lensym->code_ref(s);  s << endl;            // string length
  emit_string_constant(s,str);                                // ascii string
  s << ALIGN;                                                 // align to word
}

//
// StrTable::code_string
// Generate a string object definition for every string constant in the 
// stringtable.
//
void StrTable::code_string_table(ostream& s, int stringclasstag)
{  
  for (List<StringEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,stringclasstag);
}

//
// Ints
//
void IntEntry::code_ref(ostream &s)
{
  s << INTCONST_PREFIX << index;
}

//
// Emit code for a constant Integer.
// You should fill in the code naming the dispatch table.
//

void IntEntry::code_def(ostream &s, int intclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                // label
      << WORD << intclasstag << endl                      // class tag
      << WORD << (DEFAULT_OBJFIELDS + INT_SLOTS) << endl  // object size
      << WORD; 

 /***** Add dispatch information for class Int ******/
      s << "Int" << DISPTAB_SUFFIX;

      s << endl;                                          // dispatch table
      s << WORD << str << endl;                           // integer value
}


//
// IntTable::code_string_table
// Generate an Int object definition for every Int constant in the
// inttable.
//
void IntTable::code_string_table(ostream &s, int intclasstag)
{
  for (List<IntEntry> *l = tbl; l; l = l->tl())
    l->hd()->code_def(s,intclasstag);
}


//
// Bools
//
BoolConst::BoolConst(int i) : val(i) { assert(i == 0 || i == 1); }

void BoolConst::code_ref(ostream& s) const
{
  s << BOOLCONST_PREFIX << val;
}
  
//
// Emit code for a constant Bool.
// You should fill in the code naming the dispatch table.
//

void BoolConst::code_def(ostream& s, int boolclasstag)
{
  // Add -1 eye catcher
  s << WORD << "-1" << endl;

  code_ref(s);  s << LABEL                                  // label
      << WORD << boolclasstag << endl                       // class tag
      << WORD << (DEFAULT_OBJFIELDS + BOOL_SLOTS) << endl   // object size
      << WORD;

 /***** Add dispatch information for class Bool ******/
      s << "Bool" << DISPTAB_SUFFIX;

      s << endl;                                            // dispatch table
      s << WORD << val << endl;                             // value (0 or 1)
}

//////////////////////////////////////////////////////////////////////////////
//
//  CgenClassTable methods
//
//////////////////////////////////////////////////////////////////////////////

//***************************************************
//
//  Emit code to start the .data segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_data()
{
  Symbol main    = idtable.lookup_string(MAINNAME);
  Symbol string  = idtable.lookup_string(STRINGNAME);
  Symbol integer = idtable.lookup_string(INTNAME);
  Symbol boolc   = idtable.lookup_string(BOOLNAME);

  str << "\t.data\n" << ALIGN;
  //
  // The following global names must be defined first.
  //
  str << GLOBAL << CLASSNAMETAB << endl;
  str << GLOBAL; emit_protobj_ref(main,str);    str << endl;
  str << GLOBAL; emit_protobj_ref(integer,str); str << endl;
  str << GLOBAL; emit_protobj_ref(string,str);  str << endl;
  str << GLOBAL; falsebool.code_ref(str);  str << endl;
  str << GLOBAL; truebool.code_ref(str);   str << endl;
  str << GLOBAL << INTTAG << endl;
  str << GLOBAL << BOOLTAG << endl;
  str << GLOBAL << STRINGTAG << endl;

  //
  // We also need to know the tag of the Int, String, and Bool classes
  // during code generation.
  //
  str << INTTAG << LABEL
      << WORD << intclasstag << endl;
  str << BOOLTAG << LABEL 
      << WORD << boolclasstag << endl;
  str << STRINGTAG << LABEL 
      << WORD << stringclasstag << endl;    
}


//***************************************************
//
//  Emit code to start the .text segment and to
//  declare the global names.
//
//***************************************************

void CgenClassTable::code_global_text()
{
  str << GLOBAL << HEAP_START << endl
      << HEAP_START << LABEL 
      << WORD << 0 << endl
      << "\t.text" << endl
      << GLOBAL;
  emit_init_ref(idtable.add_string("Main"), str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Int"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("String"),str);
  str << endl << GLOBAL;
  emit_init_ref(idtable.add_string("Bool"),str);
  str << endl << GLOBAL;
  emit_method_ref(idtable.add_string("Main"), idtable.add_string("main"), str);
  str << endl;
}

void CgenClassTable::code_bools(int boolclasstag)
{
  falsebool.code_def(str,boolclasstag);
  truebool.code_def(str,boolclasstag);
}

void CgenClassTable::code_select_gc()
{
  //
  // Generate GC choice constants (pointers to GC functions)
  //
  str << GLOBAL << "_MemMgr_INITIALIZER" << endl;
  str << "_MemMgr_INITIALIZER:" << endl;
  str << WORD << gc_init_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_COLLECTOR" << endl;
  str << "_MemMgr_COLLECTOR:" << endl;
  str << WORD << gc_collect_names[cgen_Memmgr] << endl;
  str << GLOBAL << "_MemMgr_TEST" << endl;
  str << "_MemMgr_TEST:" << endl;
  str << WORD << (cgen_Memmgr_Test == GC_TEST) << endl;
}


//********************************************************
//
// Emit code to reserve space for and initialize all of
// the constants.  Class names should have been added to
// the string table (in the supplied code, is is done
// during the construction of the inheritance graph), and
// code for emitting string constants as a side effect adds
// the string's length to the integer table.  The constants
// are emmitted by running through the stringtable and inttable
// and producing code for each entry.
//
//********************************************************

void CgenClassTable::code_constants()
{
  //
  // Add constants that are required by the code generator.
  //
  stringtable.add_string("");
  inttable.add_string("0");

  stringtable.code_string_table(str,stringclasstag);
  inttable.code_string_table(str,intclasstag);
  code_bools(boolclasstag);
}


CgenClassTable::CgenClassTable(Classes classes, ostream& s) : nds(NULL) , str(s)
{
   stringclasstag = 0 /* Change to your String class tag here */;
   intclasstag =    0 /* Change to your Int class tag here */;
   boolclasstag =   0 /* Change to your Bool class tag here */;
   CgenClassTable::next_class_tag = 0;

   enterscope();
   if (cgen_debug) cout << "Building CgenClassTable" << endl;
   install_basic_classes();
   install_classes(classes);
   build_inheritance_tree();
   stringclasstag=CgenClassTable::class_tag_pair[stringtable.lookup_string("String")];
   intclasstag=CgenClassTable::class_tag_pair[stringtable.lookup_string("Int")];
   boolclasstag=CgenClassTable::class_tag_pair[stringtable.lookup_string("Bool")];

   code();
   exitscope();
}

void CgenClassTable::install_basic_classes()
{

// The tree package uses these globals to annotate the classes built below.
  //curr_lineno  = 0;
  Symbol filename = stringtable.add_string("<basic class>");

//
// A few special class names are installed in the lookup table but not
// the class list.  Thus, these classes exist, but are not part of the
// inheritance hierarchy.
// No_class serves as the parent of Object and the other special classes.
// SELF_TYPE is the self class; it cannot be redefined or inherited.
// prim_slot is a class known to the code generator.
//
  addid(No_class,
	new CgenNode(class_(No_class,No_class,nil_Features(),filename),
			    Basic,this));
  addid(SELF_TYPE,
	new CgenNode(class_(SELF_TYPE,No_class,nil_Features(),filename),
			    Basic,this));
  addid(prim_slot,
	new CgenNode(class_(prim_slot,No_class,nil_Features(),filename),
			    Basic,this));

// 
// The Object class has no parent class. Its methods are
//        cool_abort() : Object    aborts the program
//        type_name() : Str        returns a string representation of class name
//        copy() : SELF_TYPE       returns a copy of the object
//
// There is no need for method bodies in the basic classes---these
// are already built in to the runtime system.
//
  install_class(
   new CgenNode(
    class_(Object, 
	   No_class,
	   append_Features(
           append_Features(
           single_Features(method(cool_abort, nil_Formals(), Object, no_expr())),
           single_Features(method(type_name, nil_Formals(), Str, no_expr()))),
           single_Features(method(copy, nil_Formals(), SELF_TYPE, no_expr()))),
	   filename),
    Basic,this));

// 
// The IO class inherits from Object. Its methods are
//        out_string(Str) : SELF_TYPE          writes a string to the output
//        out_int(Int) : SELF_TYPE               "    an int    "  "     "
//        in_string() : Str                    reads a string from the input
//        in_int() : Int                         "   an int     "  "     "
//
   install_class(
    new CgenNode(
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
	   filename),	    
    Basic,this));

//
// The Int class has no methods and only a single attribute, the
// "val" for the integer. 
//
   install_class(
    new CgenNode(
     class_(Int, 
	    Object,
            single_Features(attr(val, prim_slot, no_expr())),
	    filename),
     Basic,this));

//
// Bool also has only the "val" slot.
//
    install_class(
     new CgenNode(
      class_(Bool, Object, single_Features(attr(val, prim_slot, no_expr())),filename),
      Basic,this));

//
// The class Str has a number of slots and operations:
//       val                                  ???
//       str_field                            the string itself
//       length() : Int                       length of the string
//       concat(arg: Str) : Str               string concatenation
//       substr(arg: Int, arg2: Int): Str     substring
//       
   install_class(
    new CgenNode(
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
             filename),
        Basic,this));

}

// CgenClassTable::install_class
// CgenClassTable::install_classes
//
// install_classes enters a list of classes in the symbol table.
//
void CgenClassTable::install_class(CgenNodeP nd)
{
  Symbol name = nd->get_name();

  if (probe(name))
    {
      return;
    }

  // The class name is legal, so add it to the list of classes
  // and the symbol table.
  StringEntryP str_ptr = stringtable.lookup_string(nd->get_name()->get_string());
  CgenClassTable::class_nameTab_entries.push_back(str_ptr);
  CgenClassTable::class_tag_pair[str_ptr] = CgenClassTable::next_class_tag++;
  nds = new List<CgenNode>(nd,nds);
  addid(name,nd);
}

void CgenClassTable::install_classes(Classes cs)
{
  for(int i = cs->first(); cs->more(i); i = cs->next(i))
    install_class(new CgenNode(cs->nth(i),NotBasic,this));
}

//
// CgenClassTable::build_inheritance_tree
//
void CgenClassTable::build_inheritance_tree()
{
  for(List<CgenNode> *l = nds; l; l = l->tl())
      set_relations(l->hd());
}

//
// CgenClassTable::set_relations
//
// Takes a CgenNode and locates its, and its parent's, inheritance nodes
// via the class table.  Parent and child pointers are added as appropriate.
//
void CgenClassTable::set_relations(CgenNodeP nd)
{
  CgenNode *parent_node = probe(nd->get_parent());
  nd->set_parentnd(parent_node);
  parent_node->add_child(nd);
}

void CgenNode::add_child(CgenNodeP n)
{
  children = new List<CgenNode>(n,children);
}

void CgenNode::set_parentnd(CgenNodeP p)
{
  assert(parentnd == NULL);
  assert(p != NULL);
  parentnd = p;
}



void CgenClassTable::code()
{
  if (cgen_debug) cout << "coding global data" << endl;
  code_global_data();

  if (cgen_debug) cout << "choosing gc" << endl;
  code_select_gc();

  if (cgen_debug) cout << "coding constants" << endl;
  code_constants();

//                 Add your code to emit
//                   - class_nameTab
  str << CLASSNAMETAB << LABEL;
  for(size_t i=0;i<CgenClassTable::class_nameTab_entries.size();i++){
    str << WORD; CgenClassTable::class_nameTab_entries[i]->code_ref(str); str << "\n";
  }

//                   - class_objTab objects
  str << CLASSOBJTAB << LABEL;
  for(size_t i=0;i<CgenClassTable::class_nameTab_entries.size();i++){
    str << WORD; str << CgenClassTable::class_nameTab_entries[i]->get_string() << PROTOBJ_SUFFIX; str << "\n";
    str << WORD; str << CgenClassTable::class_nameTab_entries[i]->get_string() << CLASSINIT_SUFFIX; str << "\n";
  }


//                   - dispatch tables
  {
    // std::map<CgenNodeP, std::string> class_codes_pair;
    std::map<CgenNodeP, std::vector<std::string> > class_methods;
    std::queue<CgenNodeP> next_nodes;
    next_nodes.push(root());
    while(!next_nodes.empty()){
      CgenNodeP cur_node = next_nodes.front();
      next_nodes.pop();
      class_methods[cur_node]=std::vector<std::string>();

      // push children nodes
      for (List<CgenNode>* l = cur_node->get_children(); l; l = l->tl()){
        next_nodes.push(l->hd());
      }

      // generate codes for current node
      std::string dispatab_codes = "";

      // copy dispatch table from parent
      CgenNodeP parent_node = cur_node->get_parentnd();
      std::map<std::string, size_t> inherited_method_idx;
      std::vector<std::string> rt_methods;
      if(class_methods.find(parent_node)!=class_methods.end()){
        rt_methods=class_methods[parent_node];
        for(size_t i=0;i<rt_methods.size();i++){
          std::string class_method_name=rt_methods[i];
          class_methods[cur_node].push_back(class_method_name);
          std::string method_name="";
          size_t str_idx=0;
          for(;str_idx<class_method_name.size();str_idx++){
            if(class_method_name[str_idx]=='.'){
              break;
            }
          }
          str_idx++;
          for(;str_idx<class_method_name.size();str_idx++){
            method_name+=class_method_name[str_idx];
          }
          inherited_method_idx[method_name]=i;
        }
        // dispatab_codes += class_codes_pair[parent_node];
      }

      // appending methods of itself
      std::string cur_node_name(cur_node->get_name()->get_string());
      std::vector<Symbol> rt = cur_node->get_method_names();
      for(size_t i=0; i<rt.size(); i++){
        std::string method_name=rt[i]->get_string();
        std::string class_method_name="";
        // check overriding
        if(inherited_method_idx.find(method_name)!=inherited_method_idx.end()){
          class_method_name+=cur_node_name;
          class_method_name+=METHOD_SEP;
          class_method_name+=method_name;
          class_methods[cur_node][inherited_method_idx[method_name]]=class_method_name;
        }
        else{
          class_methods[cur_node].push_back(cur_node_name+METHOD_SEP+rt[i]->get_string());
        }
      }

      str << cur_node_name << DISPTAB_SUFFIX << LABEL;
      for(size_t i=0; i<class_methods[cur_node].size(); i++){
        str << WORD << class_methods[cur_node][i] << "\n";
        size_t str_idx=0;
        std::string method_name="";
        std::string class_method_name=class_methods[cur_node][i];
        for(;str_idx<class_method_name.size();str_idx++){
          if(class_method_name[str_idx]=='.'){
            break;
          }
        }
        str_idx++;
        for(;str_idx<class_method_name.size();str_idx++){
          method_name+=class_method_name[str_idx];
        }
        CgenClassTable::method_class_offset[cur_node_name+" "+method_name]=i;
      }

      // store and output dispatch codes for current node
      // class_codes_pair[cur_node] = dispatab_codes;
      str << dispatab_codes;
    }

    // for(std::map<CgenNodeP, std::vector<std::string> >::iterator it=class_methods.begin();it!=class_methods.end();it++){
    //   std::string cur_node_name(it->first->get_name()->get_string());
    //   for(size_t i=0;i<it->second.size();i++){
    //   }
    // }
  }

//                   - prototype objects
// Int will be set to 0; Str will be set to ""; Bool will be set to 0;
// the rest of the classes will be set to 0 directly
  {
    std::queue<CgenNodeP> next_nodes;
    next_nodes.push(root());
    while(!next_nodes.empty()){
      CgenNodeP cur_node = next_nodes.front();
      next_nodes.pop();

      // push children nodes
      for (List<CgenNode>* l = cur_node->get_children(); l; l = l->tl()){
        next_nodes.push(l->hd());
      }

      std::vector<Symbol> rt = cur_node->get_attribute_types();
      const char * cur_node_name_string = cur_node->get_name()->get_string();
      str << WORD << "-1" << endl;
      str << cur_node_name_string << PROTOBJ_SUFFIX << LABEL;
      str << WORD << CgenClassTable::class_tag_pair[stringtable.lookup_string((char *)cur_node_name_string)] << endl;
      str << WORD << (3+rt.size()) << endl;
      str << WORD << cur_node_name_string << DISPTAB_SUFFIX << endl;
      
      for(size_t i=0;i<rt.size();i++){
        if(rt[i]==Str){
          str << WORD; stringtable.lookup_string("")->code_ref(str); str << endl;
        }
        else if(rt[i]==Int){
          str << WORD; inttable.lookup_string("0")->code_ref(str); str << endl;
        }
        else if(rt[i]==Bool){
          str << WORD; falsebool.code_ref(str); str << endl;
        }
        else{
          str << WORD << 0 << endl;
        }
      }
    }
  }

  if (cgen_debug) cout << "coding global text" << endl;
  code_global_text();

//                 Add your code to emit
//                   - object initializer
  {
    std::map<CgenNodeP, std::string> class_codes_pair;
    std::queue<CgenNodeP> next_nodes;
    next_nodes.push(root());
    while(!next_nodes.empty()){
      CgenNodeP cur_node = next_nodes.front();
      cur_node_global = cur_node;
      next_nodes.pop();

      // push children nodes
      for (List<CgenNode>* l = cur_node->get_children(); l; l = l->tl()){
        next_nodes.push(l->hd());
      }

      const char * cur_node_name_string = cur_node->get_name()->get_string();

      str << cur_node_name_string << CLASSINIT_SUFFIX << LABEL;

      // storing information of the caller in the stack
      emit_addiu(SP, SP, -12, str); // allocate 3 words in stack
      emit_store(FP, 3, SP, str); // store fp
      emit_store(SELF, 2, SP, str); // store caller's self
      emit_store(RA, 1, SP, str); // store ra
      emit_addiu(FP, SP, 4, str); // adjust fp to pointing to ra
      emit_move(SELF, ACC, str); // move callee's self from $a0 to $s0

      if(cur_node->get_parent() != No_class){
        str << JAL << cur_node->get_parent()->get_string() << CLASSINIT_SUFFIX << endl; // jump to parent's initialization function
      }

      // record idx of all attributes in side a class
      cur_node->setting_attr_offset();

      // generate codes for initialization
      Features features = cur_node->get_features();
      for(int i = features->first(); features->more(i); i = features->next(i)){
        if(features->nth(i)->is_attribute()){ // only consider attribute
          attr_class * attr_ptr = (attr_class *) features->nth(i); // safely cast feature to attribute
          attr_ptr->init->code(str);
          // if expression exist, then expression value need to be assigned to attribute
          // if(!attr_ptr->init->is_no_expr()){
            emit_store(ACC, attr_ptr->idx_inside_class, SELF, str);
          // }
        }
      }

      // restore informtaion of the caller
      emit_move(ACC, SELF, str); // restore $s0 to $a0
      emit_load(FP, 3, SP, str); // restore $sp
      emit_load(SELF, 2, SP, str); // restore $s0
      emit_load(RA, 1, SP, str); // prepare return address
      emit_addiu(SP, SP, 12, str); // pop stack
      emit_return(str); // jump back to next instruction after calling
    }
  }

//                   - the class methods
  {
    std::queue<CgenNodeP> next_nodes;
    next_nodes.push(root());
    while(!next_nodes.empty()){
      CgenNodeP cur_node = next_nodes.front();
      cur_node_global = cur_node;
      next_nodes.pop();
      const char * cur_node_name_string = cur_node->get_name()->get_string();

      // push children nodes
      for (List<CgenNode>* l = cur_node->get_children(); l; l = l->tl()){
        next_nodes.push(l->hd());
      }

      // no need to deal with methods of the basic types cause it is alreay handled in cool runtime
      if( cur_node->name==Object || cur_node->name==Str || cur_node->name==Int || 
          cur_node->name==Bool || cur_node->name==IO){

        continue;
      }

      // prepare local variables information of object
      cur_attr_posi_info.enterscope();
      std::vector<Symbol> attrs_list = cur_node->get_attribute_names();
      size_t cur_idx=3;
      for(size_t i=0;i<attrs_list.size();i++){
        std::ostringstream os;
        os << (cur_idx++)*4;
        os << "($s0)";
        cur_attr_posi_info.addid(
          attrs_list[i], 
          attr_offset_stringtable.add_string((char*) os.str().c_str())
        );
      }

      Features features = cur_node->get_features();
      for(int i = features->first(); features->more(i); i = features->next(i)){
        if(!features->nth(i)->is_attribute()){ // only consider methods
          next_temp_number_in_stack=0; // set next temporary number to 0

          cur_attr_posi_info.enterscope();
          method_class* method_ptr = (method_class*) features->nth(i); // safely cast feature to method
          str << cur_node_name_string << METHOD_SEP << method_ptr->name->get_string() << LABEL; // generate method label
          size_t number_of_temp = method_ptr->expr->nt();
          total_number_of_temp_global=number_of_temp;
          
          // count number of arguements
          size_t number_of_formal=0;
          Formals formals = method_ptr->formals;
          for(int i = formals->first(); formals->more(i); i = formals->next(i)){
            number_of_formal++;
          }

          // loading arguments into stack
          size_t cur_posi_idx=0;
          for(int i = formals->first(); formals->more(i); i = formals->next(i)){
            std::ostringstream os;
            os << (number_of_temp+2+(number_of_formal-(cur_posi_idx++)))*4 << "($fp)";
            cur_attr_posi_info.addid(
              ((formal_class *) formals->nth(i))->name, // safely cast to formal_class
              attr_offset_stringtable.add_string((char*) os.str().c_str())
            );
          }

          emit_addiu(SP, SP, -(number_of_temp+3)*4, str); // allocate 3-word space in stack
          emit_store(FP, number_of_temp+3, SP, str); // store $fp
          emit_store(SELF, number_of_temp-1+3, SP, str); // store $s0
          emit_store(RA, number_of_temp-2+3, SP, str); // store $ra
          emit_addiu(FP, SP, 4, str); // ajust fp just above sp
          emit_move(SELF, ACC, str); // move allocated object from $a0 to $s0 (SELF)

          method_ptr->expr->code(str); // generate codes for expression

          emit_load(FP, number_of_temp+3, SP, str); // restore $fp
          emit_load(SELF, number_of_temp-1+3, SP, str); // restore $s0
          emit_load(RA, number_of_temp-2+3, SP, str); // restore $ra

          // free stack space including space for arguments
          emit_addiu(SP, SP, 12+(number_of_formal*4)+number_of_temp*4, str); // free allocate space, totally (3+#arg+#local_var)
          emit_return(str); // return to caller
          cur_attr_posi_info.exitscope();
        }
      }

      cur_attr_posi_info.exitscope();

    }
  }

//                   - etc...

}


CgenNodeP CgenClassTable::root()
{
   return probe(Object);
}


///////////////////////////////////////////////////////////////////////
//
// CgenNode methods
//
///////////////////////////////////////////////////////////////////////

CgenNode::CgenNode(Class_ nd, Basicness bstatus, CgenClassTableP ct) :
   class__class((const class__class &) *nd),
   parentnd(NULL),
   children(NULL),
   basic_status(bstatus)
{ 
   stringtable.add_string(name->get_string());          // Add class name to string table
}





//******************************************************************
//
//   Fill in the following methods to produce code for the
//   appropriate expression.  You may add or remove parameters
//   as you wish, but if you do, remember to change the parameters
//   of the declarations in `cool-tree.h'  Sample code for
//   constant integers, strings, and booleans are provided.
//
//*****************************************************************

void assign_class::code(ostream &s) {
  expr->code(s); // calculate expression and final result is stored in $a0
  std::string attr_posi_info(cur_attr_posi_info.lookup(name)->get_string());
  s << SW << ACC << " " << attr_posi_info << endl;
  // remind GC (garbage collector) about the update of variables
  std::string offset_of_variable="", register_label="";
  size_t i=0;
  for(;i<attr_posi_info.size();i++){
    if(attr_posi_info[i]=='('){
      break;
    }
    offset_of_variable+=attr_posi_info[i];
  }

  i++;
  for(;i<attr_posi_info.size();i++){
    if(attr_posi_info[i]==')'){
      break;
    }
    register_label+=attr_posi_info[i];
  }
  s << ADDIU << A1 << " " << register_label << " " << offset_of_variable << endl; // load object address into $a1
  emit_jal("_GenGC_Assign", s); // jump to _GenGC_Assign procedure
}

void static_dispatch_class::code(ostream &s) {
  // store arguments in the stack in order
  for(int i = actual->first(); actual->more(i); i = actual->next(i)){
    actual->nth(i)->code(s); // load arguements into $a0
    emit_store(ACC, 0, SP, s); // store arguement in $a0 into stack
    emit_addiu(SP, SP, -4, s); // push into stack
  }

  expr->code(s); // generate codes object

  int label_in_use = label_idx++;

  emit_bne(ACC, ZERO, label_in_use, s); // check whether dispatched object is valid
  s << LA << ACC << " "; (stringtable.lookup_string(cur_node_global->filename->get_string()))->code_ref(s); s << endl; // load filename of the class
  emit_load_imm(T1, get_line_number(), s); // load line number
  emit_jal("_dispatch_abort", s); // jump to dispatch handler
  emit_label_def(label_in_use, s); // emit label for branch
  std::string class_method_name(type_name->get_string());
  emit_load_address(T1, ((char *) (class_method_name+DISPTAB_SUFFIX).c_str()), s); // load static dispatch table address
  class_method_name+=" ";
  class_method_name+=name->get_string();
  emit_load(T1, CgenClassTable::method_class_offset[class_method_name], T1, s); // load method address from dispatch table
  emit_jalr(T1, s); // jump to dispatch method
}

void dispatch_class::code(ostream &s) {
  // store arguments in the stack in order
  for(int i = actual->first(); actual->more(i); i = actual->next(i)){
    actual->nth(i)->code(s); // load arguements into $a0
    emit_store(ACC, 0, SP, s); // store arguement in $a0 into stack
    emit_addiu(SP, SP, -4, s); // push into stack
  }

  expr->code(s); // generate codes object

  int label_in_use = label_idx++;

  emit_bne(ACC, ZERO, label_in_use, s); // check whether dispatched object is valid
  s << LA << ACC << " "; (stringtable.lookup_string(cur_node_global->filename->get_string()))->code_ref(s); s << endl; // load filename of the class
  emit_load_imm(T1, get_line_number(), s); // load line number
  emit_jal("_dispatch_abort", s); // jump to dispatch handler
  emit_label_def(label_in_use, s); // emit label for branch
  emit_load(T1, 2, ACC, s); // load dispatch table address
  std::string class_method_name(expr->type->get_string());
  if(class_method_name=="SELF_TYPE"){
    class_method_name=cur_node_global->name->get_string();
  }

  class_method_name+=" ";
  class_method_name+=name->get_string();
  emit_load(T1, CgenClassTable::method_class_offset[class_method_name], T1, s); // load method address from dispatch table
  emit_jalr(T1, s); // jump to dispatch method

}

void cond_class::code(ostream &s) {
  int else_label_idx=label_idx++;
  int fi_label_idx=label_idx++;
  pred->code(s);
  emit_load(T1, 3, ACC, s); // load value of returning bool object of the predicate
  emit_beqz(T1, else_label_idx, s); // jump to else if predicate value is false
  then_exp->code(s); // generate codes for then expression
  emit_branch(fi_label_idx, s); // finish then expression, jump directly to fi
  emit_label_def(else_label_idx, s); // emit else label
  else_exp->code(s); // generate codes for else expression
  emit_label_def(fi_label_idx, s); // emit fi label
}

void loop_class::code(ostream &s) {
  int pred_label_idx=label_idx++;
  int pool_label_idx=label_idx++;
  emit_label_def(pred_label_idx, s); // emit predicate label
  pred->code(s); // generate codes for predicate
  emit_load(T1, 3, ACC, s); // load value of return bool object from the predicate object
  emit_beq(T1, ZERO, pool_label_idx, s); // jump to pool if predicate value is false
  body->code(s); // generate codes for body
  emit_branch(pred_label_idx, s); // jump to predicate after one loop finishes
  emit_label_def(pool_label_idx, s); // emit pool label
  emit_move(ACC, ZERO, s); // clear the return result of loop
}

void typcase_class::code(ostream &s) {
  size_t number_of_cases=0;
  std::map<int, branch_class*> sort_symbol_map; // map will automatically sort symbol by size_t
  for(int i = cases->first(); cases->more(i); i = cases->next(i)){
    // we can safely cast Case_class to brach_class
    StringEntryP rt=stringtable.lookup_string((((branch_class*) cases->nth(i))->type_decl)->get_string());
    sort_symbol_map[CgenClassTable::class_tag_pair[rt]]=((branch_class*) cases->nth(i));
    number_of_cases++;
  }
  int start_label_for_typecase=label_idx; // record the starting value of typecase
  int esac_label_idx=start_label_for_typecase++; // record the starting value of typecase
  label_idx+=(number_of_cases+2); // reserve labels for typecase #(label for all case)+#(default for no case match)+#()

  expr->code(s); // generate codes for expression part in typecase
  emit_bne(ACC, ZERO, start_label_for_typecase++, s); // check whether return value of expr is null
  s << LA << ACC << " "; (stringtable.lookup_string(cur_node_global->filename->get_string()))->code_ref(s); s << endl; // load filename of the class
  emit_load_imm(T1, get_line_number(), s); // load line number
  emit_jal("_case_abort2", s); // jump to case abort 2
  bool load_return_class_tag=false;

  // allocate space to hold return value of case
  next_temp_number_in_stack++;
  std::ostringstream os;
  os << (total_number_of_temp_global-next_temp_number_in_stack)*4;
  os << "($fp)";

  // generate codes for each case
  // case with higer tag value will be handled first
  for(
      std::map<int, branch_class*>::reverse_iterator it = sort_symbol_map.rbegin();
      it != sort_symbol_map.rend();
      it++
    ) 
  {
    cur_attr_posi_info.enterscope(); 
    StringEntryP rt=stringtable.lookup_string(it->second->type_decl->get_string());
    int class_tag=CgenClassTable::class_tag_pair[rt];
    emit_label_def(start_label_for_typecase-1, s); // emit label for case
    if(!load_return_class_tag){
      // only need to load class tag once
      emit_load(T2, 0, ACC, s); // load class tag for the class
      load_return_class_tag=true;
    }
    emit_blti(T2, class_tag, start_label_for_typecase, s); // jump to next label if current class tag is smaller
    emit_bgti(T2, class_tag, start_label_for_typecase, s); // jump to next label if current class tag is larger
    // record position of temporaries in stack
    cur_attr_posi_info.addid(
      it->second->name,
      attr_offset_stringtable.add_string((char*) os.str().c_str())
    );
    s << SW << ACC << " " << os.str() << endl; // store return result object of expr
    it->second->expr->code(s); // generate codes for expression
    emit_branch(esac_label_idx, s); // branch to esac
    start_label_for_typecase++;
    cur_attr_posi_info.exitscope();
  }
  emit_label_def(start_label_for_typecase-1, s); // emit label for no case match
  emit_jal("_case_abort", s); // jump to case abort if no case macthed
  emit_label_def(esac_label_idx, s); // emit label for esac
}

void block_class::code(ostream &s) {
  for(int i = body->first(); body->more(i); i = body->next(i)){
    body->nth(i)->code(s);
  }
}

void let_class::code(ostream &s) {
  cur_attr_posi_info.enterscope();
  init->code(s); // do initializatoin of object with return object in $a0
  std::ostringstream os;
  next_temp_number_in_stack++;
  os << (total_number_of_temp_global-next_temp_number_in_stack)*4;
  os << "($fp)";
  std::string attr_posi_info=os.str();
  // record position of temporaries in stack
  cur_attr_posi_info.addid(
    identifier,
    attr_offset_stringtable.add_string((char*) attr_posi_info.c_str())
  );
  s << SW << ACC << " " << attr_posi_info << endl; // store initialized object in $a0 to stack position
  std::string offset_of_variable="", register_label="";
  size_t i=0;
  for(;i<attr_posi_info.size();i++){
    if(attr_posi_info[i]=='('){
      break;
    }
    offset_of_variable+=attr_posi_info[i];
  }

  i++;
  for(;i<attr_posi_info.size();i++){
    if(attr_posi_info[i]==')'){
      break;
    }
    register_label+=attr_posi_info[i];
  }
  s << ADDIU << A1 << " " << register_label << " " << offset_of_variable << endl; // load object address into $a1
  emit_jal("_GenGC_Assign", s); // jump to _GenGC_Assign procedure
  body->code(s); // generate codes for body
  cur_attr_posi_info.exitscope();
}

void plus_class::code(ostream &s) {
  e1->code(s); // generate codes for expression1
  emit_store(ACC, 0, SP, s); // store result in stack
  emit_addiu(SP, SP, -4, s); // push in stack
  e2->code(s); // generate codes for expression2 with value store at $a0
  emit_jal("Object.copy", s); // copy return result of expression2 which stored at $a0
  emit_load(T2, 3, ACC, s); // load value of the return copy Int object of expresson2
  emit_load(T1, 1, SP, s); // load the return Int object of expression1
  emit_load(T1, 3, T1, s); // load value of the return copy Int object of expresson1
  emit_add(T1, T1, T2, s); // add value of e1 and e2
  emit_store(T1, 3, ACC, s); // store result in value field in copied Int object 
  emit_addiu(SP, SP, 4, s); // pop in stack
}

void sub_class::code(ostream &s) {
  e1->code(s); // generate codes for expression1
  emit_store(ACC, 0, SP, s); // store result in stack
  emit_addiu(SP, SP, -4, s); // push in stack
  e2->code(s); // generate codes for expression2 with value store at $a0
  emit_jal("Object.copy", s); // copy return result of expression2 which stored at $a0
  emit_load(T2, 3, ACC, s); // load value of the return copy Int object of expresson2
  emit_load(T1, 1, SP, s); // load the return Int object of expression1
  emit_load(T1, 3, T1, s); // load value of the return copy Int object of expresson1
  emit_sub(T1, T1, T2, s); // sub value of e1 and e2
  emit_store(T1, 3, ACC, s); // store result in value field in copied Int object 
  emit_addiu(SP, SP, 4, s); // pop in stack
}

void mul_class::code(ostream &s) {
  e1->code(s); // generate codes for expression1
  emit_store(ACC, 0, SP, s); // store result in stack
  emit_addiu(SP, SP, -4, s); // push in stack
  e2->code(s); // generate codes for expression2 with value store at $a0
  emit_jal("Object.copy", s); // copy return result of expression2 which stored at $a0
  emit_load(T2, 3, ACC, s); // load value of the return copy Int object of expresson2
  emit_load(T1, 1, SP, s); // load the return Int object of expression1
  emit_load(T1, 3, T1, s); // load value of the return copy Int object of expresson1
  emit_mul(T1, T1, T2, s); // mul value of e1 and e2
  emit_store(T1, 3, ACC, s); // store result in value field in copied Int object 
  emit_addiu(SP, SP, 4, s); // pop in stack
}

void divide_class::code(ostream &s) {
  e1->code(s); // generate codes for expression1
  emit_store(ACC, 0, SP, s); // store result in stack
  emit_addiu(SP, SP, -4, s); // push in stack
  e2->code(s); // generate codes for expression2 with value store at $a0
  emit_jal("Object.copy", s); // copy return result of expression2 which stored at $a0
  emit_load(T2, 3, ACC, s); // load value of the return copy Int object of expresson2
  emit_load(T1, 1, SP, s); // load the return Int object of expression1
  emit_load(T1, 3, T1, s); // load value of the return copy Int object of expresson1
  emit_div(T1, T1, T2, s); // div value of e1 and e2
  emit_store(T1, 3, ACC, s); // store result in value field in copied Int object 
  emit_addiu(SP, SP, 4, s); // pop in stack
}

void neg_class::code(ostream &s) {
  e1->code(s); // generate codes for expressoin
  emit_jal("Object.copy", s); // copy integer object in $a0
  emit_load(T1, 3, ACC, s); // load integer value into $t1
  emit_neg(T1, T1, s); // do negative operation on value
  emit_store(T1, 3, ACC, s); // load integer value into $t1
}

void lt_class::code(ostream &s) {
  e1->code(s); // generate codes for e1
  emit_store(ACC, 0, SP, s); // store result in stack
  emit_addiu(SP, SP, -4, s); // push in stack
  e2->code(s); // generate codes for e1
  emit_load(T2, 3, ACC, s); // load value of return object of e2 into $t2
  emit_load(T1, 1, SP, s); // load result object of e1 into $t1
  emit_load(T1, 3, T1, s); // load value of result object in $t1
  s << LA << ACC << " "; truebool.code_ref(s); s << endl; // load true into $a0
  int return_label_idx=label_idx++;
  emit_blt(T1, T2, return_label_idx, s); // if $t1<$2 jump to return label
  s << LA << ACC << " "; falsebool.code_ref(s); s << endl; // load true into $a0
  emit_label_def(return_label_idx, s); // emit return label
  emit_addiu(SP, SP, 4, s); // pop in stack
}

void eq_class::code(ostream &s) {
  e1->code(s); // generate codes in expressoin1 with return value stored in $a0
  emit_store(ACC, 0, SP, s); // store result in stack
  emit_addiu(SP, SP, -4, s); // push in stack
  e2->code(s); // generate codes in expressoin2 with return value stored in $a0
  emit_move(T2, ACC, s); // load result object of expresson2 into $t2
  // emit_load(T2, 3, T2, s); // load result value of expresson2 into $t2
  emit_load(T1, 1, SP, s); // load result of expression1 into $t1
  // emit_load(T1, 3, T1, s); // load value of result of expression1 into $t1
  s << LA << ACC << " "; truebool.code_ref(s); s << endl; // load true into $a0
  int branch_label=label_idx++;
  // if return address of e1 and e2 are the same object, then value check is not necessary
  // in other words, we could jump through value check
  emit_beq(T1, T2, branch_label, s);
  s << LA << A1 << " "; falsebool.code_ref(s); s << endl; // load false into $a1
  emit_jal("equality_test", s); // compare all value in T1 and T2
  emit_label_def(branch_label, s); // write out label
  emit_addiu(SP, SP, 4, s); // pop in stack
}

void leq_class::code(ostream &s) {
  e1->code(s); // generate codes for e1
  emit_store(ACC, 0, SP, s); // store result in stack
  emit_addiu(SP, SP, -4, s); // push in stack
  e2->code(s); // generate codes for e1
  emit_load(T2, 3, ACC, s); // load value of return object of e2 into $t2
  emit_load(T1, 1, SP, s); // load result object of e1 into $t1
  emit_load(T1, 3, T1, s); // load value of result object in $t1
  s << LA << ACC << " "; truebool.code_ref(s); s << endl; // load true into $a0
  int return_label_idx=label_idx++;
  emit_bleq(T1, T2, return_label_idx, s); // if $t1<$2 jump to return label
  s << LA << ACC << " "; falsebool.code_ref(s); s << endl; // load true into $a0
  emit_label_def(return_label_idx, s); // emit return label
  emit_addiu(SP, SP, 4, s); // pop in stack
}

void comp_class::code(ostream &s) {
  e1->code(s); // generate codes for e1
  emit_load(T1, 3, ACC, s); // store value of bool object returned by e1
  s << LA << ACC << " "; truebool.code_ref(s); s << endl; // load true into $a0
  int return_label_idx=label_idx++;
  emit_beqz(T1, return_label_idx, s); // jump to return label if $t1 is false(0)
  s << LA << ACC << " "; falsebool.code_ref(s); s << endl; // load false object into $a0
  emit_label_def(return_label_idx, s); // emit return label
}

void int_const_class::code(ostream& s)  
{
  //
  // Need to be sure we have an IntEntry *, not an arbitrary Symbol
  //
  emit_load_int(ACC,inttable.lookup_string(token->get_string()),s);
}

void string_const_class::code(ostream& s)
{
  emit_load_string(ACC,stringtable.lookup_string(token->get_string()),s);
}

void bool_const_class::code(ostream& s)
{
  emit_load_bool(ACC, BoolConst(val), s);
}

void new__class::code(ostream &s) {
  std::string class_prototype(type_name->get_string());
  class_prototype+=PROTOBJ_SUFFIX;
  emit_load_address(ACC, ((char *) class_prototype.c_str()), s); // load target object's prototype
  emit_jal("Object.copy", s); // allocate a space follow template of target object's prototype
  std::string class_init(type_name->get_string());
  class_init+=CLASSINIT_SUFFIX;
  emit_jal(((char *) class_init.c_str()), s); // do initialization on allocated space returned at $a0 
}

void isvoid_class::code(ostream &s) {
  e1->code(s); // generate codes for e1
  emit_move(T1, ACC, s); // move return object of e1 to $t1
  s << LA << ACC << " "; truebool.code_ref(s); s << endl; // load true into $a0
  int return_label_idx=label_idx++;
  emit_beqz(T1, return_label_idx, s); // jump to return label if object not initialized
  s << LA << ACC << " "; falsebool.code_ref(s); s << endl; // load true into $a0
  emit_label_def(return_label_idx, s); // emit return label
}

void no_expr_class::code(ostream &s) {
  emit_move(ACC, ZERO, s);
}

void object_class::code(ostream &s) {
  std::string object_name(name->get_string());
  if(name==self){
    emit_move(ACC, SELF, s); // load object itself into $a0
  }
  else{
    s << LW << ACC << " " << cur_attr_posi_info.lookup(name) << endl; // load attribute into $a0
  }
}


