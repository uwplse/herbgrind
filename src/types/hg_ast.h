#ifndef _HG_AST
#define _HG_AST

#include "hg_ast.hh"
#include "hg_opinfo.hh"
#include "hg_shadowvals.hh"

#include "pub_tool_hashtable.h"
#include "pub_tool_xarray.h"
#include "pub_tool_basics.h"

typedef struct _OpVarMapEntry {
  struct _OpVarMapEntry* next;
  OpASTNode* key;
  int varidx;
} OpVarMapEntry;

typedef struct _ValVarMapEntry {
  struct _ValVarMapEntry* next;
  ValueASTNode* key;
  int varidx;
} ValVarMapEntry;

typedef struct _ValMapEntry {
  struct _ValMapEntry* next;
  UWord key;
  int varidx;
} ValMapEntry;

struct _ValueASTNode {
  // A circular reference the the value that this AST belongs to.
  ShadowValue* val;
  // The op which turned the args into the represented shadow value.
  Op_Info* op;
  // Number of arguments (zero if this came from a constant or input).
  SizeT nargs;
  // The arguments that created this AST, or NULL if this came from an
  // input/constant.
  ValueASTNode** args;
  // The following is information for associating instances of the
  // "same" varaible with each other.

  // A map from [pointers to var op nodes] to [variable indices]. NULL
  // if this is from an input or constant.
  VgHashTable* var_map;
};

typedef enum {
  Node_Leaf,
  Node_Branch,
} NodeType;

struct _OpASTNode {
  NodeType tag;
  union {
    struct {
      // The value which this leaf node was initialized with, or NULL
      // if it has been initialized with multiple different values.
      ShadowValue* val;
      Op_Info* op;
    } Leaf;
    struct {
      // Point back to the thing that holds us, for consistencies sake
      // and so that you can pass around just the AST when you really
      // mean to be computing on the AST.
      Op_Info* op;
      // The number of arguments.
      SizeT nargs;
      // The ops that resulted in our args
      OpASTNode** args;
      // The following is information for associating instances of the
      // "same" varaible with each other.

      // A map from [variable indices] to [xarray of leaf nodes which
      // have that variable index]
      XArray* var_map;
    } Branch;
  } nd;
};

// Initialize a branch node ast for a shadow value that has just been
// produced as the result of an operation. You must set the args that
// produced this shadow value seperately after calling this
// function. Leaf nodes (shadow values that were produced from a
// constant or variable, and not as the result of a tracked operation)
// should not be initialized this way. Instead, the default null
// values will indicate that that node is a leaf node.
void initValueBranchAST(ShadowValue* val, Op_Info* opinfo,
                        SizeT nargs, ShadowValue* firstarg, ...);
// Initialize the variable map for a value ast.
void initValVarMap(ValueASTNode* valAST);
// Register a particular leaf node in a set of maps, to be used to
// merge a set of leafs into a var_map which maps them to variable
// indices which group them by double value. Uses idx_counter and
// val_to_idx for state. Not a pretty thing, not meant to be reused
// outside of a few places in hg_ast.c.
void registerLeaf(ValueASTNode* leaf, int* idx_counter,
                  VgHashTable* val_to_idx, VgHashTable* var_map);
// Initialize a leaf node ast for a shadow value that is being
// created. Takes the value itself, and the location of the slot in
// the branch operation we're creating this for which cooresponds to
// it. This part is a bit confusing, see the comment in the
// implementation of this function in hg_ast.c
void initValueLeafAST(ShadowValue* val, Op_Info** src_loc);
// Cleanup a values AST for destroying the value
void cleanupValueAST(ShadowValue* val);
// Deep copy a value AST.
void copyValueAST(ShadowValue* src, ShadowValue* dest);
// Initialize a branch node ast for an operation. You must set the
// operation nodes from which the arguments came explicitly after
// calling this function.
void initOpBranchAST(Op_Info* op, SizeT nargs);
// Initialize a leaf node ast, for a shadow value that was not
// produced as the result of an operation. This doesn't actually
// coorespond to any op, except the "initialize value" op.
void initOpLeafAST(Op_Info* out, ShadowValue* val);
// Given an operation, and the ast of a value just produced by that
// operation, update the ast of the operation so that it is the most
// specific AST which is general enough so that: every value ast that
// has been produced by this operation could be created from the
// operation ast by replacing indeterminate leaf nodes by some
// subtree. Indeterminate leaf nodes are leaf nodes for which the val
// field is null.
void updateAST(Op_Info* op, ValueASTNode* result);
// Generalize an exising AST to meet the above criteria. This function
// is nearly identical to updateAST, but does not handle the case
// where the operation doesn't already have an AST, because this is
// the first time it has been invoked.
void generalizeAST(OpASTNode* opast, ValueASTNode* valast, Op_Info* pSource);
// A recursive tree walk which produces from a value AST an equivalent
// op AST, linking together ops instead of concrete values.
OpASTNode* convertValASTtoOpAST(ValueASTNode* valAST);
// Converts a valvarmap from a value, which maps leaf values to
// variable indices, to a opvarmap for ops, which maps variable
// indices to groups of op nodes which match them.
XArray* opvarmapFromValvarmap(VgHashTable* valVarMap);
// Convert a map from var nodes to indices, to an equivalent map from
// the cooresponding op nodes to the same indices.
VgHashTable* opLookupTable(VgHashTable* valVarMap);
// Flips a var map that maps from variable indexes to leaf nodes to
// the other way around.
VgHashTable* flipOpVarMap(XArray* opVarMap);
// Generalize an op var sufficiently, but splitting some of it's
// variable groups, so that it doesn't group any leaf nodes that the
// valVarMap doesn't group.
void generalizeVarMap(XArray* opVarMap, VgHashTable* valVarMap);
// Tests whether an int is in an xarray of ints
Bool inXArray(XArray* haystack, int needle);
// Given an AST (as the node at the top of one), returns all the
// variables bound in that ast.
XArray* usedVars(OpASTNode* opAST);
// Print out an op AST as an expression, currently in the stupidest possible way.
char* opASTtoExpr(OpASTNode* opAST);
// A helper function.
char* opASTtoExprwithVarMap(OpASTNode* opAST, VgHashTable* varMap);
// Print out an op AST as an FPBench benchmark.
char* opASTtoBench(OpASTNode* opAST);
// Some debugging printer functions.

// Prints a table from pointers to indices.
void printLookupTable(VgHashTable* opLookupTable);
void printOpVarMap(XArray* opVarMap);
void printLeafList(XArray* leafList);
// Invariant checkers
void checkOpVarMapValVarMapSameLeaves(XArray* opVarMap, VgHashTable* valVarMap);
#endif
