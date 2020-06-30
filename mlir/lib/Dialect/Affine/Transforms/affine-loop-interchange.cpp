//===- AffineLoopInvariantCodeMotion.cpp - Code to perform loop fusion-----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements loop invariant code motion.
//
//===----------------------------------------------------------------------===//

#include "PassDetail.h"
#include "mlir/Analysis/AffineAnalysis.h"
#include "mlir/Analysis/AffineStructures.h"
#include "mlir/Analysis/LoopAnalysis.h"
#include "mlir/Analysis/SliceAnalysis.h"
#include "mlir/Analysis/Utils.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Affine/Passes.h"
#include "mlir/IR/AffineExpr.h"
#include "mlir/IR/AffineMap.h"
#include "mlir/IR/BlockAndValueMapping.h"
#include "mlir/IR/Builders.h"
#include "mlir/Transforms/LoopUtils.h"
#include "mlir/Transforms/Utils.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <bits/stdc++.h>
#include <iostream>
#include <vector>

#define DEBUG_TYPE "Affineloopinterchange"

using namespace mlir;
using namespace std;

// vector<vector<int> > fe( 1024 , vector<int> (1024, 0));

namespace {

/// Loop invariant code motion (LICM) pass.
/// TODO(asabne) : The pass is missing zero-trip tests.
/// TODO(asabne) : Check for the presence of side effects before hoisting.
/// TODO: This code should be removed once the new LICM pass can handle its
///       uses.
struct loopinterchange : public AffineloopinterchangeBase<loopinterchange> {
  void runOnFunction() override;
  void runOnAffineForOp(AffineForOp forOp);
  SmallVector<Operation *, 4> loadsAndStores;
};
} // end anonymous namespace

// dependence-analysis--------------------------------------------------------------------
// Returns a result string which represents the direction vector (if there was
// a dependence), returns the string "false" otherwise.
static std::string
getDirectionVectorStr(bool ret, unsigned numCommonLoops, unsigned loopNestDepth,
                      ArrayRef<DependenceComponent> dependenceComponents,
                      vector<vector<int>> *depend,
                      vector<vector<char>> *depend_dir) {

  vector<int> node;
  vector<char> node1;
  vector<int> node2;
  vector<int> node3;
  if (!ret)
    return "false";
  if (dependenceComponents.empty() || loopNestDepth > numCommonLoops)
    return "true";
  std::string result;
  node.clear();
  for (unsigned i = 0, e = dependenceComponents.size(); i < e; ++i) {
    std::string lbStr = "-inf";
    if (dependenceComponents[i].lb.hasValue() &&
        dependenceComponents[i].lb.getValue() !=
            std::numeric_limits<int64_t>::min())
      lbStr = std::to_string(dependenceComponents[i].lb.getValue());

    std::string ubStr = "+inf";
    if (dependenceComponents[i].ub.hasValue() &&
        dependenceComponents[i].ub.getValue() !=
            std::numeric_limits<int64_t>::max())
      ubStr = std::to_string(dependenceComponents[i].ub.getValue());
    node.push_back(dependenceComponents[i].lb.getValue());

    // direction vector
    int low_bnd, up_bnd;
    low_bnd = dependenceComponents[i].lb.getValue();
    up_bnd = dependenceComponents[i].ub.getValue();

    if (low_bnd < 0 && up_bnd < 0) {
      node1.push_back('-');
    } else if (low_bnd > 0 && up_bnd > 0) {
      node1.push_back('+');
    } else if (low_bnd == 0 && up_bnd == 0) {
      node1.push_back('0');
    } else if (low_bnd < 0 && up_bnd > 0) {
      node1.push_back('*');
    }

    result += "[" + lbStr + ", " + ubStr + "]";
  }
  depend->push_back(node);
  depend_dir->push_back(node1);
  return result;
}

// For each access in 'loadsAndStores', runs a dependence check between this
// "source" access and all subsequent "destination" accesses in
// 'loadsAndStores'. Emits the result of the dependence check as a note with
// the source access.
static void checkDependences(ArrayRef<Operation *> loadsAndStores,
                             vector<vector<int>> *depend,
                             vector<string> *all_dependences,
                             vector<vector<char>> *depend_dir) {
  for (unsigned i = 0, e = loadsAndStores.size(); i < e; ++i) {
    auto *srcOpInst = loadsAndStores[i];

    MemRefAccess srcAccess(srcOpInst);
    for (unsigned j = 0; j < e; ++j) {
      auto *dstOpInst = loadsAndStores[j];
      MemRefAccess dstAccess(dstOpInst);

      unsigned numCommonLoops =
          getNumCommonSurroundingLoops(*srcOpInst, *dstOpInst);
      for (unsigned d = 1; d <= numCommonLoops + 1; ++d) {
        FlatAffineConstraints dependenceConstraints;
        SmallVector<DependenceComponent, 2> dependenceComponents;
        DependenceResult result = checkMemrefAccessDependence(
            srcAccess, dstAccess, d, &dependenceConstraints,
            &dependenceComponents);
        assert(result.value != DependenceResult::Failure);
        bool ret = hasDependence(result);
        // TODO(andydavis) Print dependence type (i.e. RAW, etc) and print
        // distance vectors as: ([2, 3], [0, 10]). Also, shorten distance
        // vectors from ([1, 1], [3, 3]) to (1, 3).
        // srcOpInst->emitRemark("dependence from ")
        //     << i << " to " << j << " at depth " << d << " = "
        //     << getDirectionVectorStr(ret, numCommonLoops, d,
        //                              dependenceComponents);
        all_dependences->push_back(getDirectionVectorStr(
            ret, numCommonLoops, d, dependenceComponents, depend, depend_dir));
        // start_node.push_back(i);
        // dest_node.push_back(j);
      }
    }
  }
}

// end-of-dependence
// anlaysis--------------------------------------------------------------

// finding valid permutations

static void getPerfectNest(FuncOp f,
                           std::vector<SmallVector<AffineForOp, 6>> *bands) {
  // Get maximal perfect nest of 'affine.for' insts starting from root
  // (inclusive).
  auto getMaximalPerfectLoopNest = [&](AffineForOp root) {
    SmallVector<AffineForOp, 6> band;
    getPerfectlyNestedLoops(band, root);
    bands->push_back(band);
  };

  for (auto &block : f) {

    for (auto &op : block) {

      if (auto forOp = dyn_cast<AffineForOp>(op)) {
        getMaximalPerfectLoopNest(forOp);
      }
    }
  }
}

void storePermutation(vector<int> a, int n, int k, vector<vector<int>> &fe) {
  for (int i = 0; i < n; i++) {

    fe[k][i] = a[i];
  }
}

// Function to find the permutations
void findPermutations(vector<int> a, int n, int fact, vector<vector<int>> &fe) {

  // Sort the given array
  sort(a.begin(), a.begin() + n);

  // Find all possible permutations
  cout << "Possible permutations are:\n";
  int z = 0;
  do {
    storePermutation(a, n, z, fe);
    z += 1;
  } while (next_permutation(a.begin(), a.begin() + n));
}

bool checkValidity(vector<int> a, int d) {
  for (int i = 0; i < d; i++) {
    if (a[i] < 0) {
      i--;
      while (i >= 0) {

        if (a[i] > 0) {
          return true;
        }
        i--;
      }
      return false;
    }
  }
  return true;
}

// use dependency analysis to generate a list of valid permutations
void findValidPermutations(int d, vector<vector<int>> depend,
                           vector<vector<int>> *valid_perms,
                           vector<vector<int>> &fe) {
  vector<vector<int>> A(d, vector<int>(d, 0));

  vector<int> nest(d, 0);

  for (int i = 0; i < d; i++)
    nest[i] = i;

  // factorial of a number

  int fact = 1;
  for (int k = 1; k < d + 1; k++)
    fact *= k;

  findPermutations(nest, d, fact, fe);

  for (int k = 0; k < fact; k++) {
    int validity = 1;
    for (int i = 0; i < d; i++) {
      for (int j = 0; j < d; j++)
        A[i][j] = 0;
    }

    for (int i = 0; i < d; i++) {
      A[i][fe[k][i]] = 1;
    }

    // multiplication with vector

    for (auto i = depend.begin(); i != depend.end(); ++i) {
      // int in_vec[d];
      vector<int> in_vec(d, 0);

      int ii = 0;
      for (auto jj = i->begin(); jj != i->end(); ++jj, ++ii) {

        in_vec[ii] = *jj;
      }
      vector<int> fin_vec(d, 0);
      for (int ii = 0; ii < d; ii++) {
        fin_vec[ii] = 0;
        for (int j = 0; j < d; j++) {
          fin_vec[ii] += A[ii][j] * in_vec[j];
        }
      }

      // check whether the permutation is valid or not--------------------------

      if (checkValidity(fin_vec, d) == false) {

        validity = 0;
      }
    }

    vector<int> node;

    if (validity == 1) {

      node.clear();
      for (int i = 0; i < d; i++)
        node.push_back(fe[k][i]);

      valid_perms->push_back(node);
    }
  }
}

// locality analysis------------------------------------------------------

float getSelfSpatialScore(vector<vector<int>> accesslist, vector<int> perm) {
  float score = 0;
  auto inn = perm.end();
  inn -= 1;
  int innermost = *inn;
  int d = accesslist[0].size();
  for (auto i = accesslist.begin(); i != accesslist.end(); i++) {
    auto inn_a = i->end();
    inn_a -= 1;
    int inner_acc = *inn_a;
    if (inner_acc == innermost) {
      score += 1;
    }

    inn--;
    inn_a--;

    if (*inn == *inn_a && d >= 2)
      score += .25;
    inn--;
    inn_a--;

    if (*inn == *inn_a && d >= 3)
      score += .1;
  }

  return score;
}

float getSelfTemporalScore(vector<vector<int>> accesslist, vector<int> perm) {

  float score = 0;
  auto inn = perm.end();
  inn -= 1;
  int innermost = *inn;
  int d = perm.size();
  for (auto i = accesslist.begin(); i != accesslist.end(); i++) {
    int found = 0;
    for (auto j = i->begin(); j != i->end(); j++) {
      if (*j == innermost)
        found++;
    }
    if (found == 0) {
      score += 2;
    }
  }
  if (d >= 2) {
    inn -= 1;
    innermost = *inn;

    for (auto i = accesslist.begin(); i != accesslist.end(); i++) {
      int found = 0;
      for (auto j = i->begin(); j != i->end(); j++) {
        if (*j == innermost)
          found++;
      }
      if (found == 0)
        score += 0;
    }
  }
  if (d >= 2) {
    inn -= 1;
    innermost = *inn;

    for (auto i = accesslist.begin(); i != accesslist.end(); i++) {
      int found = 0;
      for (auto j = i->begin(); j != i->end(); j++) {
        if (*j == innermost)
          found++;
      }
      if (found == 0)
        score += 0;
    }
  }
  return score;
}

float getGroupReuseScore(vector<vector<int>> accesslist, vector<int> perm) {
  return 0;
}

float getParallelScore(vector<int> perm, vector<vector<int>> depend) {

  int d = perm.size();
  vector<int> arr(d, 0);
  float score = 0;

  int valid = 1;
  for (auto i = depend.begin(); i != depend.end(); i++) {
    int j = 0;
    for (auto k = i->begin(); j < int(i->size()); j++) {
      arr[j] = k[perm[j]];
    }

    if (arr[0] != 0)
      valid = 0;
  }

  if (valid)
    score += 1;
  return score;
}
vector<int> findBestPermutation(vector<vector<int>> accesslist,
                                vector<vector<int>> depend,
                                vector<vector<int>> valid_perms) {
  vector<int> findBestPermutation;
  float max_score = 0;
  float score = 0;
  for (auto i = valid_perms.begin(); i != valid_perms.end(); i++) {
    score = getSelfSpatialScore(accesslist, *i) +
            getSelfTemporalScore(accesslist, *i) + getParallelScore(*i, depend);
    if (score >= max_score) {
      findBestPermutation = *i;
      max_score = score;
    }
  }
  return findBestPermutation;
}

// end -of -locality
// analysis------------------------------------------------------

void loopinterchange::runOnAffineForOp(AffineForOp forOp) {}

vector<int> getAccess(string s, vector<string> &b) {
  vector<int> a;
  string access = s;
  int arg = 0;
  while (access.find(',') != string::npos) {
    arg = access.find("%arg");
    arg += 4;

    a.push_back(access[arg] - '0');
    if (access[arg + 2] == '+' || access[arg + 2] == '-' ||
        access[arg + 2] == '%') {

      b.push_back(access.substr(arg + 2, 3));
    } else {
      b.push_back(" 0 ");
    }
    if (access.find(',') != string::npos) {
      int loc = access.find(',');
      access = access.substr(loc + 1, 100);
    }
  }
  arg = access.find("%arg");
  arg += 4;
  a.push_back(access[arg] - '0');
  if (arg + 2 < int(access.size())) {
    if (access[arg + 2] == '+' || access[arg + 2] == '-' ||
        access[arg + 2] == '%') {
      b.push_back(access.substr(arg + 2, 3));
    }
  } else {
    b.push_back(" 0 ");
  }

  if (access.find(',') != string::npos) {
    int loc = access.find(',');
    access = access.substr(loc + 1, 100);
  }

  return a;
}

// getting access functoions
vector<string> getAccessVar(string s, vector<char> &variable_name,
                            vector<string> &last_access) {

  vector<string> p;

  int start, stop;
  while (s.find('[') != string::npos) {
    start = s.find('[');

    variable_name.push_back(s[start - 1]);
    stop = s.find(']');

    if (s[stop - 3] == '+' || s[stop - 3] == '-' || s[stop - 3] == '+') {
      last_access.push_back(s.substr(stop - 3, 3));
    } else {
      last_access.push_back("0");
    }

    p.push_back(s.substr(start + 1, stop - start - 1));

    s = s.substr(stop + 1, 1000);
  }
  return p;
}

vector<vector<int>> getAccessInfo(string s, vector<char> &variable_name,
                                  vector<string> &last_access,
                                  vector<vector<string>> &two) {
  vector<string> z = getAccessVar(s, variable_name, last_access);

  vector<int> b;
  vector<vector<int>> c;
  vector<string> one;
  for (auto i = z.begin(); i != z.end(); i++) {
    one.clear();
    b = getAccess(*i, one);
    two.push_back(one);
    c.push_back(b);
  }

  return c;
}

// this function makes sure that duplicate access are not there
void removeDuplicateAccess(vector<char> &variable_name,
                           vector<vector<string>> &access_function,
                           vector<vector<int>> &accesslist) {

  int vec_size = accesslist.size();

  for (int i = 0; i < vec_size; i++) {
    for (int j = i + 1; j < vec_size; j++) {
      int checker = 0;
      for (auto k = 0; k < int(access_function[i].size()) - 1; k++) {
        if (access_function[i][k] != access_function[j][k])
          checker = 1;
      }

      if (variable_name[i] == variable_name[j] &&
          accesslist[i] == accesslist[j] && checker != 1) {

        accesslist.erase(accesslist.begin() + j);
        variable_name.erase(variable_name.begin() + j);
        access_function.erase(access_function.begin() + j);
        j -= 1;
        vec_size -= 1;
      }
    }
  }
}

// function to permute loops

void loopPermutation(SmallVector<unsigned, 4> permMap,
                     SmallVector<AffineForOp, 6> nest) {

  permuteLoops(nest, permMap);
}

void runOnMultiNest(vector<SmallVector<AffineForOp, 6>> nest_loop) {
  int k = 0;

  for (auto i = nest_loop.begin(); i != nest_loop.end(); i++) {

    k += 1;

    SmallVector<Operation *, 4> loadsAndStores;

    int has_if = 0;
    string ostr;
    llvm ::raw_string_ostream os(ostr);
    vector<vector<int>> valid_perms;
    vector<string> all_dependences;
    loadsAndStores.clear();
    int d = 0;
    vector<vector<int>> fe(1024, vector<int>(1024, 0));
    valid_perms.clear();
    vector<string> last_access;

    vector<char> variable_name;
    //--------------------------------------------------

    i[0][0].walk([&](Operation *op) {
      if (isa<AffineLoadOp>(op) || isa<AffineStoreOp>(op)) {
        loadsAndStores.push_back(op);
        string s;

        os << *op;
      }

      if (isa<AffineForOp>(op))
        d += 1;

      if (isa<AffineIfOp>(op)) {

        llvm::errs() << "\nI can not handle if blocks\n";
        has_if = 1;
        return;
      }
    });

    string all_info = os.str();
    vector<vector<string>> access_function;
    vector<vector<int>> access_info =
        getAccessInfo(all_info, variable_name, last_access, access_function);

    int min = 100;
    for (auto i = access_info.begin(); i != access_info.end(); i++) {

      for (auto j = i->begin(); j != i->end(); j++) {
        if (min > *j)
          min = *j;
      }
    }

    for (auto i = access_info.begin(); i != access_info.end(); i++) {

      for (auto j = i->begin(); j != i->end(); j++) {
        if (min > *j)
          min = *j;
      }
      for (auto j = i->begin(); j != i->end(); j++) {
        *j -= min;
      }

      for (auto j = i->begin(); j != i->end(); j++) {
        cout << *j;
      }
      cout << "\n";
    }

    removeDuplicateAccess(variable_name, access_function, access_info);
    vector<vector<int>> depend;
    vector<vector<char>> depend_dir;

    checkDependences(loadsAndStores, &depend, &all_dependences, &depend_dir);

    findValidPermutations(d, depend, &valid_perms, fe);

    SmallVector<unsigned, 4> permMap;
    std::vector<int> best;
    best = findBestPermutation(access_info, depend, valid_perms);

    for (auto i = best.begin(); i != best.end(); i++)
      permMap.push_back(*i);

    loopPermutation(permMap, *i);
  }
}

void imgetPerfectNest_loop(SmallVector<AffineForOp, 6> all_for_loops) {}

SmallVector<unsigned, 4> mapPermutation(SmallVector<unsigned, 4> &permMap) {

  SmallVector<unsigned, 4> p(permMap.size(), 0);

  for (auto i = 0; i != int(permMap.size()); i++) {
    p[permMap[i]] = i;
  }

  return p;
}

void cloneForLoop(AffineForOp loop) {
  OpBuilder b(loop);

  BlockAndValueMapping operandMap;
  b.clone(*loop.getOperation(), operandMap);
  return;
}

void loopinterchange::runOnFunction() {
  int has_if = 0;
  string ostr;
  llvm ::raw_string_ostream os(ostr);
  vector<vector<int>> valid_perms;
  vector<string> all_dependences;
  vector<string> last_access;
  vector<vector<string>> access_function;
  loadsAndStores.clear();
  int d = 0;
  vector<vector<int>> fe(1024, vector<int>(1024, 0));

  valid_perms.clear();
  vector<char> variable_name;
  vector<SmallVector<AffineForOp, 6>> nest_loop;
  getPerfectNest(getFunction(), &nest_loop);
  int loop_size = nest_loop.size();
  if (loop_size > 1) {
    runOnMultiNest(nest_loop);
    return;
  }

  SmallVector<AffineForOp, 6> all_for_loops;

  getFunction().walk([&](Operation *op) {
    if (isa<AffineLoadOp>(op) || isa<AffineStoreOp>(op)) {
      loadsAndStores.push_back(op);
      string s;
      os << *op;
    }

    if (isa<AffineForOp>(op)) {
      d += 1;
      all_for_loops.push_back(dyn_cast<AffineForOp>(op));
    }

    if (isa<AffineIfOp>(op)) {

      llvm::errs() << "\nI can not handle if blocks\n";
      has_if = 1;
      return;
    }
  });

  if (has_if)
    return;

  if (int(nest_loop[0].size()) < d) {
    llvm::errs() << "\nimperfect nests found\n";
    SmallVector<unsigned, 4> permMap;

    SmallVector<AffineForOp, 6> nest;
    for (auto &op : getFunction().front()) {
      if (auto forOp = dyn_cast<AffineForOp>(op)) {
        getPerfectlyNestedLoops(nest, forOp);
        break;
      }
    }
    AffineForOp op;
    cloneForLoop(nest[0]);

    SmallVector<AffineForOp, 6> comb_loops;
    getFunction().walk([&](Operation *op) {
      if (isa<AffineForOp>(op)) {

        comb_loops.push_back(dyn_cast<AffineForOp>(op));
      }
    });
    comb_loops[1].erase();
    comb_loops[3].erase();

    vector<SmallVector<AffineForOp, 6>> nest_loop;

    getPerfectNest(getFunction(), &nest_loop);
    int loop_size = nest_loop.size();
    if (loop_size > 1) {
      runOnMultiNest(nest_loop);
      return;
    }

    return;
  }

  string all_info = os.str();

  vector<vector<int>> access_info =
      getAccessInfo(all_info, variable_name, last_access, access_function);

  int min = 100;
  for (auto i = access_info.begin(); i != access_info.end(); i++) {

    for (auto j = i->begin(); j != i->end(); j++) {
      if (min > *j)
        min = *j;
    }
  }

  for (auto i = access_info.begin(); i != access_info.end(); i++) {

    for (auto j = i->begin(); j != i->end(); j++) {
      if (min > *j)
        min = *j;
    }
    for (auto j = i->begin(); j != i->end(); j++) {
      *j -= min;
    }
  }

  removeDuplicateAccess(variable_name, access_function, access_info);

  vector<vector<int>> depend;
  vector<vector<char>> depend_dir;
  checkDependences(loadsAndStores, &depend, &all_dependences, &depend_dir);

  findValidPermutations(d, depend, &valid_perms, fe);

  // permuting loops------

  SmallVector<AffineForOp, 6> nest;
  for (auto &op : getFunction().front()) {
    if (auto forOp = dyn_cast<AffineForOp>(op)) {
      getPerfectlyNestedLoops(nest, forOp);
      break;
    }
  }
  SmallVector<unsigned, 4> permMap;
  std::vector<int> best;
  best = findBestPermutation(access_info, depend, valid_perms);

  for (auto i = best.begin(); i != best.end(); i++)
    permMap.push_back(*i);

  permMap = mapPermutation(permMap);
  loopPermutation(permMap, nest);

  getFunction().walk([&](AffineForOp op) {
    LLVM_DEBUG(op.getOperation()->print(llvm::dbgs() << "\nOriginal loop\n"));
    runOnAffineForOp(op);
  });
}

std::unique_ptr<OperationPass<FuncOp>> mlir::createAffineloopinterchangePass() {
  return std::make_unique<loopinterchange>();
}
