#include "hg_ast.h"
#include "hg_shadowvals.h"
#include "hg_opinfo.h"
#include "../include/hg_macros.h"
#include "../include/hg_options.h"

#include "pub_tool_libcprint.h"
#include "pub_tool_mallocfree.h"

#include <stdarg.h>

#define MAX_AST_STR_LEN 256

void initValueBranchAST(ShadowValue* val, Op_Info* opinfo,
                        SizeT nargs, ShadowValue* firstarg, ...){
  val->ast->val = val;
  val->ast->op = opinfo;

  val->ast->nargs = nargs;
  ALLOC(val->ast->args, "hg.val_ast_args", nargs, sizeof(ShadowValue*));

  // I guess this is how you handle variable arity in c. 
  va_list args;

  va_start(args, firstarg);
  val->ast->args[0] = firstarg->ast;
  for(int i = 1; i < nargs; ++i){
    val->ast->args[i] = va_arg(args, ShadowValue*)->ast;
  }
  va_end(args);

  val->ast->var_map = VG_(HT_construct)("val_var_map");
}

void initValueLeafAST(ShadowValue* val){
  val->ast->val = val;
  val->ast->op = NULL;
  val->ast->nargs = 0;
  val->ast->args = NULL;
  val->ast->var_map = NULL;
}

void cleanupValueAST(ShadowValue* val){
  VG_(HT_destruct)(val->ast->var_map, VG_(free));
  VG_(free)(val->ast->args);
  VG_(free)(val->ast);
}

void copyValueAST(ShadowValue* src, ShadowValue* dest){
  ALLOC(dest->ast, "hg.val_ast", 1, sizeof(ValueASTNode));
  dest->ast->val = dest;
  dest->ast->op = src->ast->op;
  dest->ast->nargs = src->ast->nargs;
  dest->ast->var_map = src->ast->var_map;
  if (src->ast->nargs != 0){
    ALLOC(dest->ast->args, "hg.val_ast_args", src->ast->nargs, sizeof(ShadowValue*));
    for (int i = 0; i < src->ast->nargs; ++i){
      dest->ast->args[i] = src->ast->args[i];
    }
  }
}

void initOpBranchAST(OpASTNode* out, Op_Info* op, SizeT nargs){
  out->tag = Node_Branch;
  out->nd.Branch.op = op;
  out->nd.Branch.nargs = nargs;
  ALLOC(out->nd.Branch.args, "hg.val_ast_args", nargs, sizeof(OpASTNode*));
  out->nd.Branch.var_map = VG_(newXA)(VG_(malloc), "var_map",
                                      VG_(free), sizeof(XArray*));
}

void initOpLeafAST(OpASTNode* out, ShadowValue* val){
  out->tag = Node_Leaf;
  out->nd.Leaf.val = val;
}

void updateAST(Op_Info* op, ValueASTNode* trace_ast){
  if (op->ast == NULL){
    // The first time we see a value, the most specific AST that fits it
    // is exactly it's ast.
    op->ast = convertValASTtoOpAST(trace_ast);
  } else {
    // If we've already seen values, we'll want to generalize the AST
    // we have sufficiently to match the new value.
    generalizeAST(op->ast, trace_ast);
  }
  // This doesn't (shouldn't) affect the functionality of this
  // function, but allows us to print AST's on update, mostly for
  // debugging purposes. Or like, you might just be into that, in
  // which case more power to you.
  if (print_expr_updates){
    char* opASTString = opASTtoString(op->ast);
    VG_(printf)("Updating op ast to: %s\n", opASTString);
    VG_(free)(opASTString);
  }
}

void generalizeAST(OpASTNode* opast, ValueASTNode* valast){
  if (opast->tag == Node_Leaf){
    // If we hit a value leaf, and it matches the one we've already
    // seen, then our best guess right now is that that is a constant
    // which doesn't change in this expression, so leave it in our AST.
    if (valast->val == NULL ||
        opast->nd.Leaf.val == NULL ||
        mpfr_get_d(valast->val->value, MPFR_RNDN)
        == mpfr_get_d(opast->nd.Leaf.val->value, MPFR_RNDN))
      return;
    // Otherwise, it's some sort of input that changes, so abstract it
    // into a variable by setting it's val field to NULL.
    else {
      opast->nd.Leaf.val = NULL;
      return;
    }
  } else {
    // We're at a branch node.
    if (valast->op == NULL || valast->op != opast->nd.Branch.op){
      // If the valast is a leaf node, or it continues but it doesn't
      // match the opast, cut off the opast here, with a variable leaf
      // node (one where the shadow value is NULL, because we've seen
      // different values here).

      // Once we free the args array we malloc'ed, we can just
      // overwrite everything with new initial values, and things will
      // probably turn out fine.
      VG_(free)(opast->nd.Branch.args);
      initOpLeafAST(opast, NULL);
    } else if (opast->nd.Branch.op != NULL) {
      // Otherwise, if they both continue and match, generalize the
      // variable map appropriately, and recurse on children,
      for(int i = 0; i < valast->nargs; ++i){
        generalizeVarMap(opast->nd.Branch.var_map, valast->var_map);
        generalizeAST(opast->nd.Branch.args[i], valast->args[i]);
      }
    }
  }
}

OpASTNode* convertValASTtoOpAST(ValueASTNode* valAST){
  // First, check if we've already made an AST for the op that this
  // value came from. If so, just share that.
  if (valAST->op != NULL && valAST->op->ast != NULL) return valAST->op->ast;

  // Otherwise, we need to create a new AST node.
  OpASTNode* result;
  ALLOC(result, "hg.op_ast", 1, sizeof(OpASTNode));
  // If the value we're copying is a leaf...
  if (valAST->op == NULL){
    // Make a leaf node for the op ast.
    initOpLeafAST(result, valAST->val);
  } else {
    // Otherwise, start a branch node.
    initOpBranchAST(result, valAST->op, valAST->nargs);
    // Convert all of the children recursively. Usually they won't hit
    // this branch again, since we generally build subexpression AST's
    // before their parents, with the exception of leaf nodes since
    // they don't get build until a branch needs them. So, our
    // children should either already have an op ast somewhere, or
    // they are leaf nodes. This isn't necessarily an invariant I'm
    // commited to, so we should operate fine if it's not true, but
    // for clarity's sake that is what I currently expect.
    for (int i = 0; i < valAST->nargs; ++i){
      result->nd.Branch.args[i] = convertValASTtoOpAST(valAST->args[i]);
    }

    // Now, build up a map from double values to variable indices, so
    // that we can identify when variable leaf nodes are the "same"
    // variable. At the same time, build up the var_map for our shadow
    // value.
    VgHashTable* val_to_idx = VG_(HT_construct)("val_to_idx");
    int next_idx = 0;
    for (int i = 0; i < valAST->nargs; ++i){
      ValueASTNode* arg = valAST->args[i];
      if (arg->op == NULL){
        registerLeaf(arg, &next_idx, val_to_idx, valAST->var_map);
      } else {
        VG_(HT_ResetIter)(arg->var_map);
        for (ValVarMapEntry* entry = VG_(HT_Next)(arg->var_map);
             entry != NULL; entry = VG_(HT_Next)(arg->var_map)){
          registerLeaf(entry->key, &next_idx, val_to_idx,
                       valAST->var_map);
        }
      }
    }
    VG_(HT_destruct)(val_to_idx, VG_(free));
    // Finally, since this is the first value map this op has seen, it
    // copies it as it's own this means that any values that were the
    // same this time are currently assumed to be the same variable
    // (or constant), and anything that isn't the same this time can
    // never be the same variable.
    result->nd.Branch.var_map = opvarmapFromValvarmap(valAST->var_map);
  }
  return result;
}

void registerLeaf(ValueASTNode* leaf, int* idx_counter,
                  VgHashTable* val_to_idx, VgHashTable* var_map){
  double val = mpfr_get_d(leaf->val->value, MPFR_RNDN);
  ValMapEntry* val_entry = VG_(HT_lookup)(val_to_idx, val);
  int cur_idx;
  // If there isn't an already existing entry with the same
  // value, then this is a value we haven't seen before, so
  // create a fresh variable index for it.
  if (val_entry == NULL){
    // Add a new entry to our local map from values to
    // indices, so that next time we get a value that matches
    // this we'll map it to the same index.
    ALLOC(val_entry, "val_to_idx entry", 1, sizeof(ValMapEntry));
    val_entry->key = val;
    val_entry->varidx = (*idx_counter)++;
    VG_(HT_add_node)(val_to_idx, val_entry);

    // Use our new idx.
    cur_idx = val_entry->varidx;
  } else {
    // Use the old idx.
    cur_idx = val_entry->varidx;
  }
  // Here we'll update our var_map for this op node, to map the
  // leaf node we're currently processing to a index which is
  // unique to it's value. Now, this may not actually be a
  // variable, it might still be a constant at this point, but
  // we don't know yet.
  ValVarMapEntry* valvar_entry;
  ALLOC(valvar_entry, "leaf_to_idx entry", 1, sizeof(ValVarMapEntry));
  valvar_entry->key = leaf;
  valvar_entry->varidx = cur_idx;
  VG_(HT_add_node)(var_map, valvar_entry);
}

XArray* opvarmapFromValvarmap(VgHashTable* valVarMap){
  XArray* opVarMap = VG_(newXA)(VG_(malloc), "opVarMap",
                                VG_(free), sizeof(XArray*));
  VG_(HT_ResetIter)(valVarMap);
  for (ValVarMapEntry* valEntry = VG_(HT_Next)(valVarMap);
       valEntry != NULL; valEntry = VG_(HT_Next)(valVarMap)){
    while(VG_(sizeXA)(opVarMap) <= valEntry->varidx){
      XArray* newVarGroup = VG_(newXA)(VG_(malloc), "opVarMapRow",
                                        VG_(free), sizeof(OpASTNode*));
      VG_(addToXA)(opVarMap, &newVarGroup);
    }
    XArray** entry = VG_(indexXA)(opVarMap, valEntry->varidx);
    XArray* varGroup = *entry;
    OpASTNode* opNode = convertValASTtoOpAST(valEntry->key);
    VG_(addToXA)(varGroup, &opNode);
  }
  return opVarMap;
}

VgHashTable* opLookupTable(VgHashTable* valVarMap){
  VgHashTable* lookupTable = VG_(HT_construct)("lookupTable");
  VG_(HT_ResetIter)(valVarMap);
  for (ValVarMapEntry* valEntry = VG_(HT_Next)(valVarMap);
       valEntry != NULL; valEntry = VG_(HT_Next)(valVarMap)){
    OpVarMapEntry* opEntry;
    ALLOC(opEntry, "opEntry", 1, sizeof(OpVarMapEntry));
    opEntry->key = convertValASTtoOpAST(valEntry->key);
    opEntry->varidx = valEntry->varidx;
    VG_(HT_add_node)(lookupTable, opEntry);
  }
  return lookupTable;
}

typedef struct _IdxMapEntry {
  struct _IdxMapEntry* next;
  int key;
  int val;
} IdxMapEntry;

void generalizeVarMap(XArray* opVarMap, VgHashTable* valVarMap){
  VgHashTable* valueLookupTable = opLookupTable(valVarMap);
  for (int i = 0; i < VG_(sizeXA)(opVarMap); ++i){
    // Construct a map from indices found in the valVarMap to indices
    // in the opVarMap. For all leaves that match the first leaf in
    // this varGroup, the index they map to in valVarMap will map to
    // the var group itself. But when we need to split something away
    // from the group, we'll add a new group to the end of the array,
    // put the split element in it, and map it there. Then, future
    // leaves in this group that match it's index in the valVarMap
    // will be mapped to the same new group.
    VgHashTable* splitMap = VG_(HT_construct)("splitMap");
    XArray* varGroup = *(XArray**)VG_(indexXA)(opVarMap, i);
    IdxMapEntry* splitEntry;
    ALLOC(splitEntry, "splitEntry", 1, sizeof(IdxMapEntry));
    OpVarMapEntry* valOpEntry =
      VG_(HT_lookup)(valueLookupTable, *(UWord*)VG_(indexXA)(varGroup, 0));
    splitEntry->key = valOpEntry->varidx;
    splitEntry->val = i;
    VG_(HT_add_node)(splitMap, splitEntry);
    for(int j = 1; j < VG_(sizeXA)(varGroup); ++j){
      OpASTNode* curNode = *(OpASTNode**)VG_(indexXA)(varGroup, j);
      valOpEntry = VG_(HT_lookup)(valueLookupTable, (UWord)curNode);
      splitEntry = VG_(HT_lookup)(splitMap, valOpEntry->varidx);
      if (splitEntry == NULL){
        // This means this leaf doesn't match earlier ones in this
        // varGroup, so we need to create a new group for it, and
        // remove it from the old one.
        XArray* newGroup = (VG_(newXA)(VG_(malloc), "varGroup",
                                       VG_(free), sizeof(XArray*)));
        VG_(addToXA)(newGroup, curNode);
        Word newIndex = VG_(addToXA)(opVarMap, &newGroup);

        // Also, we need to set up a new splitmap entry so that future
        // matching leaves get redirected to this var group.
        ALLOC(splitEntry, "splitEntry", 1, sizeof(IdxMapEntry));
        splitEntry->key = valOpEntry->varidx;
        splitEntry->val = newIndex;
        VG_(HT_add_node)(splitMap, splitEntry);

        // Remove the old group entry
        VG_(removeIndexXA)(varGroup, j--);
      } else if (splitEntry->val != i) {
        // If we already have an entry in the split map for it, but it
        // doesn't map to the current group, move it to the correct
        // group.
        VG_(addToXA)(VG_(indexXA)(opVarMap, splitEntry->val), curNode);
        VG_(removeIndexXA)(varGroup, j--);
      }
    }
    VG_(HT_destruct)(splitMap, VG_(free));
  }
  VG_(HT_destruct)(valueLookupTable, VG_(free));
}

// This is a crude and wasteful function, but hopefully no one will
// notice.
char* opASTtoString(OpASTNode* opAST){
  char* buf;
  // This is our "cursor" in the output string.
  SizeT bufpos = 0;
  ALLOC(buf, "hg.ast_string", 1, sizeof(char)*MAX_AST_STR_LEN);

  // If this is a leaf node...
  if (opAST->tag == Node_Leaf){
    // It could either be a constant or a variable. 
    if (opAST->nd.Leaf.val == NULL)
      // Variables we'll print as "x" for now, but soon we should give
      // them distinct names when we don't have a reason to believe
      // they come from the same place.
      VG_(snprintf)(buf, 2, "x");
    else
      // For constants we'll get the double value associated with the
      // constant, and print it.
      VG_(snprintf)(buf, MAX_AST_STR_LEN, "%f",
                    mpfr_get_d(opAST->nd.Leaf.val->value, MPFR_RNDN));
  } else {
    // Print the opening parenthesis and operator.
    bufpos += VG_(snprintf)(buf + bufpos, MAX_AST_STR_LEN - bufpos, "(%s",
                            opAST->nd.Branch.op->debuginfo.symbol);
    // Recursively get the subexpression strings, and print them
    // preceded by a space.
    for (SizeT i = 0; i < opAST->nd.Branch.nargs; ++i){
      char* subexpr = opASTtoString(opAST->nd.Branch.args[i]);
      bufpos += VG_(snprintf)(buf + bufpos, MAX_AST_STR_LEN - bufpos, " %s",
                              subexpr);
      VG_(free)(subexpr);
    }
    // Finally, print the closing parenthesis.
    bufpos += VG_(snprintf)(buf + bufpos, MAX_AST_STR_LEN - bufpos, ")");
  }
  return buf;
}
