/* ---------------------------------------------------------------------
 *
 * Copyright (C) 2021 by the deal.II authors
 *
 * This file is part of the deal.II library.
 *
 * The deal.II library is free software; you can use it, redistribute
 * it, and/or modify it under the terms of the GNU Lesser General
 * Public License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * The full text of the license can be found in the file LICENSE.md at
 * the top level directory of deal.II.
 *
 * ---------------------------------------------------------------------

 *
 * Author: Wolfgang Bangerth, Colorado State University, 2021.
 * Based on step-15 by Sven Wetterauer, University of Heidelberg, 2012.
 */


// @sect3{Include files}

// This program starts out like most others with well known include
// files. Compared to the step-15 program from which most of what we
// do here is copied, the only difference is the include of the header
// files from which we import the SparseDirectUMFPACK class and the actual
// interface to KINSOL:

#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/function.h>
#include <deal.II/base/timer.h>
#include <deal.II/base/utilities.h>

// Included from step-40
#include <deal.II/lac/generic_linear_algebra.h>

#define FORCE_USE_OF_TRILINOS

namespace LA
{
#if defined(DEAL_II_WITH_PETSC) && !defined(DEAL_II_PETSC_WITH_COMPLEX) && \
  !(defined(DEAL_II_WITH_TRILINOS) && defined(FORCE_USE_OF_TRILINOS))
  using namespace dealii::LinearAlgebraPETSc;
#  define USE_PETSC_LA
#elif defined(DEAL_II_WITH_TRILINOS)
  using namespace dealii::LinearAlgebraTrilinos;
#else
#  error DEAL_II_WITH_PETSC or DEAL_II_WITH_TRILINOS required
#endif
} // namespace LA
// End of stuff included from step-40

#include <deal.II/lac/vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/sparse_matrix.h>
#include <deal.II/lac/dynamic_sparsity_pattern.h>
#include <deal.II/lac/affine_constraints.h>
#include <deal.II/lac/sparse_direct.h>

// Only used for one function
#include <deal.II/lac/sparsity_tools.h>

#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_refinement.h>

// MPI libraries
#include <deal.II/distributed/tria.h>
#include <deal.II/distributed/grid_refinement.h>

#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_accessor.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_values.h>
#include <deal.II/fe/fe_q.h>

#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/error_estimator.h>
#include <deal.II/numerics/solution_transfer.h>

#include <deal.II/base/conditional_ostream.h>

#include <deal.II/sundials/kinsol.h>

#include <fstream>
#include <iostream>

// remember this

namespace Step77
{
  using namespace dealii;

  using kinsol = SUNDIALS::KINSOL<LA::MPI::Vector>;

  using NOX = TrilinosWrappers::NOXSolver<LA::MPI::Vector>;

  // @sect3{The <code>MinimalSurfaceProblem</code> class template}

  // Similarly, the main class of this program is essentially a copy of the one
  // in step-15. The class does, however, split the computation of the Jacobian
  // (system) matrix (and its factorization using a direct solver) and residual
  // into separate functions for the reasons outlined in the introduction. For
  // the same reason, the class also has a pointer to a factorization of the
  // Jacobian matrix that is reset every time we update the Jacobian matrix.
  //
  // (If you are wondering why the program uses a direct object for the Jacobian
  // matrix but a pointer for the factorization: Every time KINSOL requests that
  // the Jacobian be updated, we can simply write `jacobian_matrix=0;` to reset
  // it to an empty matrix that we can then fill again. On the other hand, the
  // SparseDirectUMFPACK class does not have any way to throw away its content
  // or to replace it with a new factorization, and so we use a pointer: We just
  // throw away the whole object and create a new one whenever we have a new
  // Jacobian matrix to factor.)
  //
  // Finally, the class has a timer variable that we will use to assess how long
  // the different parts of the program take so that we can assess whether
  // KINSOL's tendency to not rebuild the matrix and its factorization makes
  // sense. We will discuss this in the "Results" section below.
  template <int dim>
  class MinimalSurfaceProblem
  {
  public:
    MinimalSurfaceProblem();
    void run();

  private:
    void setup_system(const bool initial_step);
    void solve(const LA::MPI::Vector &rhs,
               LA::MPI::Vector &      solution,
               const double           tolerance);
    void refine_mesh();
    void output_results(const unsigned int refinement_cycle);
    void set_boundary_values();
    void compute_and_factorize_jacobian(const LA::MPI::Vector &evaluation_point);
    void compute_residual(const LA::MPI::Vector &evaluation_point,
                          LA::MPI::Vector &      residual);

    MPI_Comm mpi_communicator;

    parallel::distributed::Triangulation<dim> triangulation;

    DoFHandler<dim> dof_handler;
    FE_Q<dim>       fe; 


    // Added MPI stuff
    IndexSet locally_owned_dofs;
    IndexSet locally_relevant_dofs;

    ConditionalOStream pcout;
    TimerOutput computing_timer;
    // End of MPI added stuff

    AffineConstraints<double> nonzero_constraints;
    AffineConstraints<double> zero_constraints;

    LA::MPI::SparseMatrix     jacobian_matrix;
    //std::unique_ptr<SparseDirectUMFPACK> jacobian_matrix_factorization;
    //std::unique_ptr<LA::MPI::SparseMatrix> jacobian_matrix_factorization;

    LA::MPI::Vector current_solution;

    int solver_type = 2;    // 0 - linesearch
                            // 1 - Newton
                            // 2 - Picard

    int AA_size = 10;       // Only use when using Picard solver

    int NL_Solve_type = 0;  // 0 - KINSol
                            // 1 - NOX
  };



  // @sect3{Boundary condition}

  // The classes implementing boundary values are a copy from step-15:
  template <int dim>
  class BoundaryValues : public Function<dim>
  {
  public:
    virtual double value(const Point<dim> & p,
                         const unsigned int component = 0) const override;
  };


  template <int dim>
  double BoundaryValues<dim>::value(const Point<dim> &p,
                                    const unsigned int /*component*/) const
  {
    return std::sin(2 * numbers::PI * (p[0] + p[1]));
  }


  // @sect3{The <code>MinimalSurfaceProblem</code> class implementation}

  // @sect4{Constructor and set up functions}

  // The following few functions are also essentially copies of what
  // step-15 already does, and so there is little to discuss.
  template <int dim>
  MinimalSurfaceProblem<dim>::MinimalSurfaceProblem()
    : mpi_communicator(MPI_COMM_WORLD)
    , triangulation(mpi_communicator,
                    typename Triangulation<dim>::MeshSmoothing(
                    Triangulation<dim>::smoothing_on_refinement |
                    Triangulation<dim>::smoothing_on_coarsening))
    , dof_handler(triangulation)
    , fe(1)
    , pcout(std::cout,
            (Utilities::MPI::this_mpi_process(mpi_communicator) == 0))
    , computing_timer(mpi_communicator,
                      pcout,
                      TimerOutput::never,
                      TimerOutput::wall_times)
  {}



  template <int dim>
  void MinimalSurfaceProblem<dim>::setup_system(const bool initial_step)
  {
    TimerOutput::Scope t(computing_timer, "set up");

    if (initial_step)
      {
        dof_handler.distribute_dofs(fe);

        // Added because MPI
        locally_owned_dofs = dof_handler.locally_owned_dofs();
        locally_relevant_dofs =
          DoFTools::extract_locally_relevant_dofs(dof_handler);


        current_solution.reinit(locally_owned_dofs,
                                mpi_communicator);

        // nonzero constraint section
        {
          nonzero_constraints.clear();
          nonzero_constraints.reinit(locally_relevant_dofs);
          DoFTools::make_hanging_node_constraints(dof_handler,
                                                  nonzero_constraints);

          nonzero_constraints.close();

          nonzero_constraints.distribute(current_solution);

          // Problem right here 
          std::map<types::global_dof_index, double> boundary_values;
          VectorTools::interpolate_boundary_values(dof_handler,
                                                   0,
                                                   BoundaryValues<dim>(),
                                                   boundary_values);

          for (const auto &boundary_value : boundary_values)
            current_solution(boundary_value.first) = boundary_value.second;
        }

        // zero constraint section
        
        {
          zero_constraints.clear();
          zero_constraints.reinit(locally_relevant_dofs);
          DoFTools::make_hanging_node_constraints(dof_handler, zero_constraints);
          VectorTools::interpolate_boundary_values(
                      dof_handler,
                      0,
                      Functions::ZeroFunction<dim>(),
                      zero_constraints);
        }
        zero_constraints.close();
        
      }

    //DynamicSparsityPattern dsp(dof_handler.n_dofs());
    DynamicSparsityPattern dsp(locally_relevant_dofs);

    //DoFTools::make_sparsity_pattern(dof_handler, dsp);
    DoFTools::make_sparsity_pattern(dof_handler, dsp,
                                    zero_constraints, false);

    // Included for MPI
    SparsityTools::distribute_sparsity_pattern(dsp,
                                               dof_handler.locally_owned_dofs(),
                                               mpi_communicator,
                                               locally_relevant_dofs);

    //nonzero_constraints.condense(dsp);


    jacobian_matrix.reinit(locally_owned_dofs,
                           locally_owned_dofs,
                           dsp,
                           mpi_communicator);
    //jacobian_matrix_factorization.reset();
  }



  // @sect4{Assembling and factorizing the Jacobian matrix}

  // The following function is then responsible for assembling and factorizing
  // the Jacobian matrix. The first half of the function is in essence the
  // `assemble_system()` function of step-15, except that it does not deal with
  // also forming a right hand side vector (i.e., the residual) since we do not
  // always have to do these operations at the same time.
  //
  // We put the whole assembly functionality into a code block enclosed by curly
  // braces so that we can use a TimerOutput::Scope variable to measure how much
  // time is spent in this code block, excluding everything that happens in this
  // function after the matching closing brace `}`.
  template <int dim>
  void MinimalSurfaceProblem<dim>::compute_and_factorize_jacobian(
    const LA::MPI::Vector &evaluation_point)
  {
    // TIMO : evaluation point with ghost values
    LA::MPI::Vector evaluation_point_1;
    evaluation_point_1.reinit(locally_owned_dofs,
                              locally_relevant_dofs,
                              mpi_communicator);
    evaluation_point_1 = evaluation_point;

    {
      TimerOutput::Scope t(computing_timer, "assembling the Jacobian");

      pcout << "  Computing Jacobian matrix" << std::endl;

      const QGauss<dim> quadrature_formula(fe.degree + 1);

      jacobian_matrix = 0;

      FEValues<dim> fe_values(fe,
                              quadrature_formula,
                              update_gradients | update_quadrature_points |
                                update_JxW_values);

      const unsigned int dofs_per_cell = fe.n_dofs_per_cell();
      const unsigned int n_q_points    = quadrature_formula.size();

      FullMatrix<double> cell_matrix(dofs_per_cell, dofs_per_cell);

      std::vector<Tensor<1, dim>> evaluation_point_gradients(n_q_points);

      std::vector<types::global_dof_index> local_dof_indices(dofs_per_cell);

      for (const auto &cell : dof_handler.active_cell_iterators())
        {
          if (cell->is_locally_owned())
          {
            cell_matrix = 0.;

            fe_values.reinit(cell);

            fe_values.get_function_gradients(evaluation_point_1,
                                             evaluation_point_gradients);

            for (unsigned int q = 0; q < n_q_points; ++q)
              {
                const double coeff =
                  1.0 / std::sqrt(1 + evaluation_point_gradients[q] *
                                        evaluation_point_gradients[q]);

                for (unsigned int i = 0; i < dofs_per_cell; ++i)
                  {
                    for (unsigned int j = 0; j < dofs_per_cell; ++j)
                    if (solver_type == 0 || solver_type == 1)
                      {
                        // Newton iteration (or Newton w/ linesearch)
                        cell_matrix(i, j) +=
                          (((fe_values.shape_grad(i, q)    // ((\nabla \phi_i
                            * coeff                       //   * a_n
                            * fe_values.shape_grad(j, q)) //   * \nabla \phi_j)
                            -                              //  -
                            (fe_values.shape_grad(i, q)    //  (\nabla \phi_i
                            * coeff * coeff * coeff       //   * a_n^3
                            *
                            (fe_values.shape_grad(j, q)       //   * (\nabla \phi_j
                              * evaluation_point_gradients[q]) //      * \nabla u_n)
                            * evaluation_point_gradients[q])) //   * \nabla u_n)))
                          * fe_values.JxW(q));                // * dx
                        }
                      else if (solver_type == 2)
                        {
                          // Picard iteration
                          cell_matrix(i, j) +=
                            ((fe_values.shape_grad(i, q)    // ((\nabla \phi_i
                              * coeff                       //   * a_n
                              * fe_values.shape_grad(j, q)) //   * \nabla u_n)))
                            * fe_values.JxW(q));                // * dx
                        }
                  }
              }

            cell->get_dof_indices(local_dof_indices);

            // const AffineConstraints<double> &constraints_used =
            //   initial_step ? nonzero_constraints : zero_constraints;

            zero_constraints.distribute_local_to_global(cell_matrix,
                                                        local_dof_indices,
                                                        jacobian_matrix);

          }
        }

        // TIMO  need to go into AffineConstraints, then remove all this
        // SEAN: I think I did this...
        // Update 1/26: Timo explained ghost vectors, maybe try using a place
        // holder variable to counteract the ghost value complaints.
    }

    jacobian_matrix.compress(VectorOperation::add);

    // The second half of the function then deals with factorizing the
    // so-computed matrix. To do this, we first create a new SparseDirectUMFPACK
    // object and by assigning it to the member variable
    // `jacobian_matrix_factorization`, we also destroy whatever object that
    // pointer previously pointed to (if any). Then we tell the object to
    // factorize the Jacobian.
    //
    // As above, we enclose this block of code into curly braces and use a timer
    // to assess how long this part of the program takes.
    //
    // (Strictly speaking, we don't actually need the matrix any more after we
    // are done here, and could throw the matrix object away. A code intended to
    // be memory efficient would do this, and only create the matrix object in
    // this function, rather than as a member variable of the surrounding class.
    // We omit this step here because using the same coding style as in previous
    // tutorial programs breeds familiarity with the common style and helps make
    // these tutorial programs easier to read.)
    {
      TimerOutput::Scope t(computing_timer, "factorizing the Jacobian");

      pcout << "  Factorizing Jacobian matrix" << std::endl;

// TIMO later
      //jacobian_matrix_factorization = std::make_unique<SparseDirectUMFPACK>();
      //jacobian_matrix_factorization->factorize(jacobian_matrix);
      //jacobian_matrix_factorization = std::make_unique<LA::MPI::SparseMatrix>();
      //jacobian_matrix_factorization->factorize(jacobian_matrix);
    }
  }



  // @sect4{Computing the residual vector}

  // The second part of what `assemble_system()` used to do in step-15 is
  // computing the residual vector, i.e., the right hand side vector of the
  // Newton linear systems. We have broken this out of the previous function,
  // but the following function will be easy to understand if you understood
  // what `assemble_system()` in step-15 did. Importantly, however, we need to
  // compute the residual not linearized around the current solution vector, but
  // whatever we get from KINSOL. This is necessary for operations such as line
  // search where we want to know what the residual $F(U^k + \alpha_k \delta
  // U^K)$ is for different values of $\alpha_k$; KINSOL in those cases simply
  // gives us the argument to the function $F$ and we then compute the residual
  // $F(\cdot)$ at this point.
  //
  // The function prints the norm of the so-computed residual at the end as a
  // way for us to follow along the progress of the program.
  template <int dim>
  void MinimalSurfaceProblem<dim>::compute_residual(
    const LA::MPI::Vector &evaluation_point,
    LA::MPI::Vector &      residual)
  {
    TimerOutput::Scope t(computing_timer, "assembling the residual");

    pcout << "  Computing residual vector..." << std::flush;

    // Adding in ghost vector because it can't perform stuff on a
    // distributed vector.
    LA::MPI::Vector evaluation_point_1;
    evaluation_point_1.reinit(locally_owned_dofs,
                              locally_relevant_dofs,
                              mpi_communicator);
    evaluation_point_1 = evaluation_point;

    const QGauss<dim> quadrature_formula(fe.degree + 1);
    FEValues<dim>     fe_values(fe,
                            quadrature_formula,
                            update_gradients | update_quadrature_points |
                              update_JxW_values);

    const unsigned int dofs_per_cell = fe.n_dofs_per_cell();
    const unsigned int n_q_points    = quadrature_formula.size();

    Vector<double>              cell_residual(dofs_per_cell);
    std::vector<Tensor<1, dim>> evaluation_point_gradients(n_q_points);

    std::vector<types::global_dof_index> local_dof_indices(dofs_per_cell);

    for (const auto &cell : dof_handler.active_cell_iterators())
      {
        if (cell->is_locally_owned())
        {
          cell_residual = 0.;
          fe_values.reinit(cell);

          fe_values.get_function_gradients(evaluation_point_1,
                                           evaluation_point_gradients);


          for (unsigned int q = 0; q < n_q_points; ++q)
            {
              const double coeff =
                1.0 / std::sqrt(1 + evaluation_point_gradients[q] *
                                      evaluation_point_gradients[q]);

              for (unsigned int i = 0; i < dofs_per_cell; ++i)
                cell_residual(i) =
                  (fe_values.shape_grad(i, q)      // \nabla \phi_i
                   * coeff                         // * a_n
                   * evaluation_point_gradients[q] // * \nabla u_n
                   * fe_values.JxW(q));            // * dx
            }

          cell->get_dof_indices(local_dof_indices);
          // for (unsigned int i = 0; i < dofs_per_cell; ++i)
          //   residual(local_dof_indices[i]) += cell_residual(i);

          // const AffineConstraints<double> &constraints_used =
          // initial_step ? nonzero_constraints : zero_constraints;

          zero_constraints.distribute_local_to_global(cell_residual,
                                                      local_dof_indices,
                                                      residual);
        }
      }

    //zero_constraints.condense(residual);

    // TIMO maybe?
    zero_constraints.set_zero(residual);
    residual.compress(VectorOperation::add);

    // for (const types::global_dof_index i :
    //      DoFTools::extract_boundary_dofs(dof_handler))
    //   residual(i) = 0;
    
    // for (const types::global_dof_index i :
    //      DoFTools::extract_hanging_node_dofs(dof_handler))
    //   residual(i) = 0;

    pcout << " norm=" << residual.l2_norm() << std::endl;
  }



  // @sect4{Solving linear systems with the Jacobian matrix}

  // Next up is the function that implements the solution of a linear system
  // with the Jacobian matrix. Since we have already factored the matrix when we
  // built the matrix, solving a linear system comes down to applying the
  // inverse matrix to the given right hand side vector: This is what the
  // SparseDirectUMFPACK::vmult() function does that we use here. Following
  // this, we have to make sure that we also address the values of hanging nodes
  // in the solution vector, and this is done using
  // AffineConstraints::distribute().
  //
  // The function takes an additional, but unused, argument `tolerance` that
  // indicates how accurately we have to solve the linear system. The meaning of
  // this argument is discussed in the introduction in the context of the
  // "Eisenstat Walker trick", but since we are using a direct rather than an
  // iterative solver, we are not using this opportunity to solve linear systems
  // only inexactly.
  template <int dim>
  void MinimalSurfaceProblem<dim>::solve(const LA::MPI::Vector &rhs,
                                         LA::MPI::Vector &      solution,
                                         const double /*tolerance*/)
  {
    TimerOutput::Scope t(computing_timer, "linear system solve");

    // const AffineConstraints<double> &constraints_used =
    //   initial_step ? nonzero_constraints : zero_constraints;

    pcout << "  Solving linear system" << std::endl;

    SolverControl solver_control(dof_handler.n_dofs(), 1e-12);

    #ifdef USE_PETSC_LA
        LA::SolverCG solver(solver_control, mpi_communicator);
    #else
        LA::SolverCG solver(solver_control);
    #endif

        LA::MPI::PreconditionAMG preconditioner;

        LA::MPI::PreconditionAMG::AdditionalData data;

    #ifdef USE_PETSC_LA
        data.symmetric_operator = true;
    #else
        /* Trilinos defaults are good */
    #endif
        preconditioner.initialize(jacobian_matrix, data);

    solver.solve(jacobian_matrix,
                 solution,
                 rhs,
                 preconditioner);

    pcout << "   Solved in " << solver_control.last_step() << " iterations."
          << std::endl;


    //jacobian_matrix_factorization->vmult(solution, rhs);

    zero_constraints.distribute(solution);
  }



  // @sect4{Refining the mesh, setting boundary values, and generating graphical output}

  // The following three functions are again simply copies of the ones in
  // step-15:
  template <int dim>
  void MinimalSurfaceProblem<dim>::refine_mesh()
  {
    /*
    Vector<float> estimated_error_per_cell(triangulation.n_active_cells());

    KellyErrorEstimator<dim>::estimate(
      dof_handler,
      QGauss<dim - 1>(fe.degree + 1),
      std::map<types::boundary_id, const Function<dim> *>(),
      current_solution,
      estimated_error_per_cell);

    GridRefinement::refine_and_coarsen_fixed_number(triangulation,
                                                    estimated_error_per_cell,
                                                    0.3,
                                                    0.03);

    triangulation.prepare_coarsening_and_refinement();

    SolutionTransfer<dim> solution_transfer(dof_handler);
    solution_transfer.prepare_for_coarsening_and_refinement(current_solution);

    triangulation.execute_coarsening_and_refinement();

    dof_handler.distribute_dofs(fe);

    LA::MPI::Vector tmp(dof_handler.n_dofs());
    solution_transfer.interpolate(current_solution, tmp);
    current_solution = std::move(tmp);

    constraints.clear();

    DoFTools::make_constraints(dof_handler,
                                            constraints);
    constraints.close();

    constraints.distribute(current_solution);

    set_boundary_values();

    setup_system(initial_step=false);
    */
  }



  template <int dim>
  void MinimalSurfaceProblem<dim>::set_boundary_values()
  {
    // TIMO  need a zero constraints and nonzero constraints object/
    // like in step-57
    // SEAN: I think this is ok
    // std::map<types::global_dof_index, double> boundary_values;
    // VectorTools::interpolate_boundary_values(dof_handler,
    //                                          0,
    //                                          BoundaryValues<dim>(),
    //                                          boundary_values);
    // for (const auto &boundary_value : boundary_values)
    //   current_solution(boundary_value.first) = boundary_value.second;
    //
    // constraints.distribute(current_solution);
  }



  template <int dim>
  void MinimalSurfaceProblem<dim>::output_results(
    const unsigned int refinement_cycle)
  {
    TimerOutput::Scope t(computing_timer, "graphical output");

    DataOut<dim> data_out;

    data_out.attach_dof_handler(dof_handler);
    data_out.add_data_vector(current_solution, "solution");

    // copied from step-40
    Vector<float> subdomain(triangulation.n_active_cells());
    for (unsigned int i = 0; i < subdomain.size(); ++i)
      subdomain(i) = triangulation.locally_owned_subdomain();
    data_out.add_data_vector(subdomain, "subdomain");

    data_out.build_patches();

    // const std::string filename =
    //   "solution-" + Utilities::int_to_string(refinement_cycle, 2) + ".vtu";
    // std::ofstream output(filename);
    // data_out.write_vtu(output);

    data_out.write_vtu_with_pvtu_record(
      "./", "solution", refinement_cycle, mpi_communicator, 2, 8);
  }



  // @sect4{The run() function and the overall logic of the program}

  // The only function that *really* is interesting in this program is the one
  // that drives the overall algorithm of starting on a coarse mesh, doing some
  // mesh refinement cycles, and on each mesh using KINSOL to find the solution
  // of the nonlinear algebraic equation we obtain from discretization on this
  // mesh. The `refine_mesh()` function above makes sure that the solution on
  // one mesh is used as the starting guess on the next mesh. We also use a
  // TimerOutput object to measure how much time every operation on each mesh
  // costs, and reset the timer at the beginning of each cycle.
  //
  // As discussed in the introduction, it is not necessary to solve problems on
  // coarse meshes particularly accurately since these will only solve as
  // starting guesses for the next mesh. As a consequence, we will use a target
  // tolerance of
  // $\tau=10^{-3} \frac{1}{10^k}$ for the $k$th mesh refinement cycle.
  //
  // All of this is encoded in the first part of this function:
  template <int dim>
  void MinimalSurfaceProblem<dim>::run()
  {
    pcout << "Running with "
#ifdef USE_PETSC_LA
          << "PETSc"
#else
          << "Trilinos"
#endif
          << " on " << Utilities::MPI::n_mpi_processes(mpi_communicator)
          << " MPI rank(s)..." << std::endl;

    GridGenerator::hyper_ball(triangulation);
    triangulation.refine_global(4);

    const bool initial_step = true;

    setup_system(initial_step);
    //set_boundary_values();

    for (unsigned int refinement_cycle = 0; refinement_cycle < 1;
         ++refinement_cycle)
      {
        computing_timer.reset();
        pcout << "Mesh refinement step " << refinement_cycle << std::endl;

        if (refinement_cycle != 0)
          refine_mesh();

        const double target_tolerance = 1e-3 * std::pow(0.1, refinement_cycle);
        pcout << "  Target_tolerance: " << target_tolerance << std::endl
                  << std::endl;

        // This is where the fun starts. At the top we create the KINSOL solver
        // object and feed it with an object that encodes a number of additional
        // specifics (of which we only change the nonlinear tolerance we want to
        // reach; but you might want to look into what other members the
        // SUNDIALS::KINSOL::AdditionalData class has and play with them).
        if (NL_Solve_type == 0)
        {
          typename kinsol::AdditionalData
            additional_data;
          additional_data.function_tolerance = target_tolerance;

          // Important to specify strategy for the nonlinear solver
          kinsol::AdditionalData::SolutionStrategy strategy;
          if (solver_type == 0)
          {
            // This is the default but we put it here anyway for fun
            strategy = kinsol::AdditionalData::linesearch;
          }
          else if (solver_type == 1)
          {
            // Newton
            strategy = kinsol::AdditionalData::newton;
          }
          else if (solver_type == 2)
          {
            // Picard
            strategy = kinsol::AdditionalData::picard;

            additional_data.anderson_subspace_size = AA_size;
          }
          additional_data.strategy = strategy;
          

          kinsol nonlinear_solver(additional_data, mpi_communicator);

          
          nonlinear_solver.reinit_vector = [&](LA::MPI::Vector &x) {
            //x.reinit(dof_handler.n_dofs(), mpi_communicator);
            x.reinit(locally_owned_dofs,
                     mpi_communicator);
          };

          nonlinear_solver.residual =
            [&](const LA::MPI::Vector &evaluation_point,
                LA::MPI::Vector &      residual) {
              compute_residual(evaluation_point, residual);

              return 0;
            };

          nonlinear_solver.setup_jacobian =
            [&](const LA::MPI::Vector &current_u,
                const LA::MPI::Vector & /*current_f*/) {
              compute_and_factorize_jacobian(current_u);

              return 0;
            };

          nonlinear_solver.solve_with_jacobian = [&](const LA::MPI::Vector &rhs,
                                                     LA::MPI::Vector &      dst,
                                                     const double tolerance) {
            this->solve(rhs, dst, tolerance);

            return 0;
          };

          // something happening in here
          nonlinear_solver.solve(current_solution);
        }
        else if (NL_Solve_type)
        {
          // Just like with KINSol, we have an additional_data type
          // Doesn't give us the amount of control that we want it seems.
          // It only wants to give us things like max iterations and tolerance
          // It DOES NOT give us control of the type of nonlinear solve itself.
          // More of that stuff can be foud in parameterlist
          NOX::AdditionalData additional_data;

          // We define the ParameterList class here. We can add specifics to the
          // solver with this object.
          // Help with this object type can be found here:
          // https://github.com/dealii/dealii/blob/46b63523561e3c7a6859d2fcdeb242dcc980dec2/tests/trilinos/nox_solver_01.cc#L107
          const Teuchos::RCP<Teuchos::ParameterList> &parameters =
                  Teuchos::rcp(new Teuchos::ParameterList);

          // Here we set the type of nonlinear solver
          // The default is "Line Search Based"
          parameters->set("Nonlinear Solver","Line Search Based");

          // To get specifics for the solver, we create a "sublist"
          // We start with the "Line Search" specs
          parameters->sublist("Line Search").set("Method","Full Step");;

          // Constructor, nearly identical to KINSol
          NOX nonlinear_solver(additional_data,params);

          nonlinear_solver.residual =
            [&](const LA::MPI::Vector &evaluation_point,
                LA::MPI::Vector &      residual) {
              compute_residual(evaluation_point, residual);

              return 0;
            };

          nonlinear_solver.setup_jacobian =
            [&](const LA::MPI::Vector &current_u) {
              compute_and_factorize_jacobian(current_u);

              return 0;
            };

          nonlinear_solver.solve_with_jacobian = [&](const LA::MPI::Vector &rhs,
                                                     LA::MPI::Vector &      dst,
                                                     const double tolerance) {
            this->solve(rhs, dst, tolerance);

            return 0;
          };

          // something happening in here
          nonlinear_solver.solve(current_solution);
        }

        // The rest is then just house-keeping: Writing data to a file for
        // visualizing, and showing a summary of the timing collected so that we
        // can interpret how long each operation has taken, how often it was
        // executed, etc:
        output_results(refinement_cycle);

        computing_timer.print_summary();

        pcout << std::endl;
      }
  }
} // namespace Step77


int main(int argc, char *argv[])
{
  try
    {
      using namespace Step77;

      Utilities::MPI::MPI_InitFinalize mpi_initialization(argc, argv, 1);

      MinimalSurfaceProblem<2> laplace_problem_2d;
      laplace_problem_2d.run();
    }
  catch (std::exception &exc)
    {
      std::cerr << std::endl
                << std::endl
                << "----------------------------------------------------"
                << std::endl;
      std::cerr << "Exception on processing: " << std::endl
                << exc.what() << std::endl
                << "Aborting!" << std::endl
                << "----------------------------------------------------"
                << std::endl;

      return 1;
    }
  catch (...)
    {
      std::cerr << std::endl
                << std::endl
                << "----------------------------------------------------"
                << std::endl;
      std::cerr << "Unknown exception!" << std::endl
                << "Aborting!" << std::endl
                << "----------------------------------------------------"
                << std::endl;
      return 1;
    }
  return 0;
}
