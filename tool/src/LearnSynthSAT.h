// ----------------------------------------------------------------------------
// Copyright (c) 2013-2014 by Graz University of Technology and
//                            Johannes Kepler University Linz
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, see
// <http://www.gnu.org/licenses/>.
//
// For more information about this software see
//   <http://www.iaik.tugraz.at/content/research/design_verification/demiurge/>
// or email the authors directly.
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file LearnSynthSAT.h
/// @brief Contains the declaration of the class LearnSynthSAT.
// -------------------------------------------------------------------------------------------

#ifndef LearnSynthSAT_H__
#define LearnSynthSAT_H__

#include "defines.h"
#include "CNF.h"
#include "BackEnd.h"
#include "LearnStatisticsSAT.h"

class SatSolver;

// -------------------------------------------------------------------------------------------
///
/// @class LearnSynthSAT
/// @brief Implements a learning-based synthesis using two SAT solvers.
///
/// The working principle of the algorithm implemented in this class is very similar to
/// LearnSynthQBF: We start by setting the winning region F to the set of all safe states P.
/// In a loop, we first compute a counterexample to the correctness of this guess in form of a
/// state from which the antagonist (controlling the uncontrollable inputs i) can enforce to
/// leave F. The state is represented as a cube over all state-variables. Next, this
/// state-cube is generalized into a larger region of states by dropping literals as long as
/// the cube contains only states that are winning for the antagonist (and hence must be
/// removed or have already been removed from F). Finally, the generalized cube is removed
/// from F. The next iteration checks for counterexample-state on the refined F. This is
/// repeated until no more counterexample-states exist (then the specification is realizable
/// and F is a winning region) or the initial state has been removed from F (in which case
/// the specification is unrealizable).
///
/// The difference to LearnSynthQBF is that this class uses two competing SAT-solvers to
/// compute a counterexample. First, solver #solver_i_ finds some input values i that could
/// be chosen by the antagonist in order to leave the #winning_region_. Next, #solver_ctrl_
/// checks if there exists a response of the protagonist to chose control values c such that
/// the winning region is not left. If no such response exists, then we have found a
/// counterexample. Otherwise, we exclude the state-input combination found by #solver_i_
/// (after generalizing it into a larger set of state-input pairs) and try again.
/// There is also an optimization which allows to use #solver_i_ incrementally: The next
/// state-copy of the winning region is updated only lazily.
/// Also the generalization of counterexamples is implemented with a QBF-solver instead of a
/// SAT solver. Details and background can be found in the VMCAI'14 paper "SAT-Based
/// Synthesis Methods for Safety Specs".
///
/// The procedure outlined above is implemented in #computeWinningRegionPlain(). Some
/// variants of this procedure are implemented in other methods of this class. The method
/// #computeWinningRegionRG() also uses inductive reachability reasoning in the
/// generalization of counterexamples, just like LearnSynthQBFInd.  The method
/// #computeWinningRegionRGRC() uses inductive reachability reasoning also during the
/// computation or counterexamples. These extensions are described as optimization RG and
/// optimization RC in the VMCAI'14 paper. Command-line parameters (--mode) decide which of
/// these methods to use.
///
/// Finally the QBFCertImplExtractor is used to extract a circuit.
///
/// @author Robert Koenighofer (robert.koenighofer@iaik.tugraz.at)
/// @version 1.0.0
class LearnSynthSAT : public BackEnd
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
  LearnSynthSAT();

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~LearnSynthSAT();

// -------------------------------------------------------------------------------------------
///
/// @brief Executes this back-end.
///
/// The working principle of this method is very similar to LearnSynthQBF#run(): We start by
/// setting the winning region F to the set of all safe states P.
/// In a loop, we first compute a counterexample to the correctness of this guess in form of a
/// state from which the antagonist (controlling the uncontrollable inputs i) can enforce to
/// leave F. The state is represented as a cube over all state-variables. Next, this
/// state-cube is generalized into a larger region of states by dropping literals as long as
/// the cube contains only states that are winning for the antagonist (and hence must be
/// removed or have already been removed from F). Finally, the generalized cube is removed
/// from F. The next iteration checks for counterexample-state on the refined F. This is
/// repeated until no more counterexample-states exist (then the specification is realizable
/// and F is a winning region) or the initial state has been removed from F (in which case
/// the specification is unrealizable).
/// The difference to LearnSynthQBF is that this method uses two competing SAT-solvers to
/// compute a counterexample. First, solver #solver_i_ finds some input values i that could
/// be chosen by the antagonist in order to leave the #winning_region_. Next, #solver_ctrl_
/// checks if there exists a response of the protagonist to chose control values c such that
/// the winning region is not left. If no such response exists, then we have found a
/// counterexample. Otherwise, we exclude the state-input combination found by #solver_i_
/// (after generalizing it into a larger set of state-input pairs) and try again.
/// There is also an optimization which allows to use #solver_i_ incrementally: The next
/// state-copy of the winning region is updated only lazily.
/// Also the generalization of counterexamples is implemented with a QBF-solver instead of a
/// SAT solver. Details and background can be found in the VMCAI'14 paper "SAT-Based
/// Synthesis Methods for Safety Specs".
///
/// Command-line parameters (--mode) are used to select different variants of this method.
/// There is
/// <ul>
///  <li> the standard method as described above,
///  <li> a variant which uses inductive reachability reasoning in the generalization of
///       counterexamples, just like LearnSynthQBFInd (optimization 'RG')
///  <li> a variant which uses inductive reachability reasoning also in the computation of
///       counterexamples, just like LearnSynthQBFInd (optimization 'RG' and 'RC')
/// </ul>
///
/// @return True if the specification was realizable, false otherwise.
  virtual bool run();

protected:

// -------------------------------------------------------------------------------------------
///
/// @brief Computes the winning region and stores the result in #winning_region_.
///
/// Depending on the command-line parameters (--mode) this method calls one of
/// <ul>
///  <li> #computeWinningRegionPlain()
///  <li> #computeWinningRegionRG()
///  <li> #computeWinningRegionRGRC()
/// </ul>
///
/// @return True if the specification was realizable, false otherwise.
  bool computeWinningRegion();

// -------------------------------------------------------------------------------------------
///
/// @brief Uses the standard method to compute a winning region.
///
/// The standard method works as described in the class description.
///
/// @return True if the specification was realizable, false otherwise.
  bool computeWinningRegionPlain();

// -------------------------------------------------------------------------------------------
///
/// @brief Compute a winning region with optimization RG when generalizing counterexamples.
///
/// Optimization RG is described in LearnSynthQBFInd already. In a nutshell, the
/// generalization query is modified so a state that prevents generalization must be either
/// initial or have a predecessor in F. Otherwise it is unreachable and we do not care.
/// This means that we can remove states that are winning for the protagonist from the
/// winning region of they are unreachalbe in the final implementation.
///
/// @return True if the specification was realizable, false otherwise.
  bool computeWinningRegionRG();

// -------------------------------------------------------------------------------------------
///
/// @brief Compute a winning region with optimization RG and RC enabled.
///
/// In addition to optimization RG (see #computeWinningRegionRG()), this method also applies
/// optimization RC as described in LearnSynthQBFInd. In a nutshell, the queries to compute
/// counterexamples are modified in such a way that the counterexample-state is either initial
/// or has a predecessor in the current over-approximation of the winning region. Otherwise,
/// the state is unreachable and can be ignored.
///
/// @return True if the specification was realizable, false otherwise.
  bool computeWinningRegionRGRC();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the previous-state copy of a literal.
///
/// @param literal The literal to transform.
/// @return The previous-state copy of a literal.
  int presentToPrevious(int literal) const;

// -------------------------------------------------------------------------------------------
///
/// @brief Computes the previous-state copy of a cube or clause.
///
/// @param cube_or_clause A cube or clause (in form of a vector of literals) over the
///        present state variables. This vector is overwritten by the corresponding cube of
///        clause over the previous-state literals (i.e., all literals are replaced by their
///        previous-state copy).
  void presentToPrevious(vector<int> &cube_or_clause) const;

// -------------------------------------------------------------------------------------------
///
/// @brief Computes the previous-state copy of a CNF.
///
/// @param cnf A CNF formula over the present state variables. This CNF is overwritten by the
///        corresponding CNF over the previous-state literals (i.e., all literals are
///        replaced by their previous-state copy).
  void presentToPrevious(CNF &cnf) const;

// -------------------------------------------------------------------------------------------
///
/// @brief Checks a winning region computed with optimization RC for correctness.
///
/// We usually check the correctness of the winning region in debug-mode by calling
/// Utils#debugCheckWinReg(). However, this method of Utils does not work for winning regions
/// that have been computed with optimization RC enabled. In such cases we this method.
  void debugCheckWinReg();

// -------------------------------------------------------------------------------------------
///
/// @brief Stores and maintains statistics and performance measures.
  LearnStatisticsSAT statistics_;

// -------------------------------------------------------------------------------------------
///
/// @brief The current over-approximation of the winning region.
///
/// Only when #computeWinningRegion() is done, this field will store the correct winning
/// region.
  CNF winning_region_;

// -------------------------------------------------------------------------------------------
///
/// @brief The current over-approximation of the winning region in an uncompressed form.
///
/// This CNF is logically equivalent to #winning_region_. However, #winning_region_ is
/// 'compressed' from time to time by removing redundant clauses (clauses that are implied
/// by other clauses in the winning region). This field stores the uncompressed version of
/// the winning region. Having the uncompressed winning region can be good because throwing
/// away redundant clauses is not always beneficial. The CNF get smaller, but the solver may
/// have to re-discover the removed clauses.
  CNF winning_region_large_;

// -------------------------------------------------------------------------------------------
///
/// @brief Says: the current state is initial or the previous transition relation holds.
///
/// This CNF expresses that the current state is initial or the previous-state copy of the
/// transition relation holds. This is an important building block for the CNF to compute
/// or generalize counterexamples if optimization RG or RC is enabled.
  CNF prev_trans_or_initial_;

// -------------------------------------------------------------------------------------------
///
/// @brief A map from present-state variables to their previous-state copy.
  vector<int> current_to_previous_map_;

// -------------------------------------------------------------------------------------------
///
/// @brief A literal that is true if the current state is initial and false otherwise.
///
/// The clauses assigning the literal are part of #prev_trans_or_initial_.
  int current_state_is_initial_;

// -------------------------------------------------------------------------------------------
///
/// @brief Says: the current state is different from the previous one.
///
/// This CNF expresses that the current state is different from the previous state. This is
/// an important building block for the CNF to compute counterexamples if optimization RC is
/// enabled.
  CNF different_from_prev_or_initial_;

// -------------------------------------------------------------------------------------------
///
/// @brief The solver for computing counterexample-candidates if optimization RC is disabled.
  SatSolver *solver_i_;

// -------------------------------------------------------------------------------------------
///
/// @brief The solver for checking counterexample-candidates.
///
/// It is also used to generalize counterexamples if optimization RG is disabled.
  SatSolver *solver_ctrl_;

// -------------------------------------------------------------------------------------------
///
/// @brief The solver for computing counterexample-candidates if optimization RC is enabled.
  SatSolver *solver_i_ind_;

// -------------------------------------------------------------------------------------------
///
/// @brief The solver for generalizing counterexamples if optimization RG is endabled.
  SatSolver *solver_ctrl_ind_;


private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  LearnSynthSAT(const LearnSynthSAT &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  LearnSynthSAT& operator=(const LearnSynthSAT &other);

};

#endif // LearnSynthSAT_H__
