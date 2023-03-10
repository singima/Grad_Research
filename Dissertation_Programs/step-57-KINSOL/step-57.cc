#include <deal.II/base/quadrature_lib.h>
#include <deal.II/base/function.h>
#include <deal.II/base/utilities.h>
#include <deal.II/base/tensor.h>

#include <deal.II/lac/block_vector.h>
#include <deal.II/lac/full_matrix.h>
#include <deal.II/lac/block_sparse_matrix.h>
#include <deal.II/lac/dynamic_sparsity_pattern.h>
#include <deal.II/lac/solver_cg.h>
#include <deal.II/lac/solver_gmres.h>
#include <deal.II/lac/precondition.h>
#include <deal.II/lac/affine_constraints.h>

#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/grid/grid_refinement.h>
#include <deal.II/grid/grid_tools.h>

#include <deal.II/dofs/dof_handler.h>
#include <deal.II/dofs/dof_renumbering.h>
#include <deal.II/dofs/dof_tools.h>

#include <deal.II/fe/fe_q.h>
#include <deal.II/fe/fe_system.h>
#include <deal.II/fe/fe_values.h>

#include <deal.II/numerics/vector_tools.h>
#include <deal.II/numerics/matrix_tools.h>
#include <deal.II/numerics/data_out.h>
#include <deal.II/numerics/error_estimator.h>

#include <deal.II/numerics/solution_transfer.h>
#include <deal.II/lac/sparse_direct.h>
#include <deal.II/lac/sparse_ilu.h>
#include <deal.II/sundials/kinsol.h>
#include <chrono>
using namespace std::chrono;

#include <fstream>
#include <iostream>

namespace Step57
{
  using namespace dealii;

  using kinsol = SUNDIALS::KINSOL<BlockVector<double>>;

  template <int dim>
  class StationaryNavierStokes
  {
  public:
    StationaryNavierStokes(const unsigned int degree);
    void run(const unsigned int refinement);
    void run_kinsol();

  private:
    void setup_dofs();

    void initialize_system();

    void refine_mesh();

    void process_solution(unsigned int refinement);

    void output_results(const unsigned int refinement_cycle) const;

    void compute_residual(const BlockVector<double> &kinsol_eval_point,
                          BlockVector<double> &residual);

    void compute_jacobian(const BlockVector<double> &kinsol_eval_point);

    void solve_kinsol(const BlockVector<double> &rhs,
                      BlockVector<double> &solution,
                      const double /*tolerance*/);

    double                               viscosity;
    double                               gamma;
    const unsigned int                   degree;
    std::vector<types::global_dof_index> dofs_per_block;

    Triangulation<dim> triangulation;
    FESystem<dim>      fe;
    DoFHandler<dim>    dof_handler;

    AffineConstraints<double> zero_constraints;
    AffineConstraints<double> nonzero_constraints;

    BlockSparsityPattern      sparsity_pattern;
    BlockSparseMatrix<double> system_matrix;
    SparseMatrix<double>      pressure_mass_matrix;

    BlockVector<double> present_solution;
    BlockVector<double> newton_update;
    BlockVector<double> system_rhs;
    BlockVector<double> evaluation_point;

    BlockSparseMatrix<double> jacobian_matrix;
    BlockVector<double> kinsol_solution;
    BlockVector<double> residual;

    std::unique_ptr<SparseDirectUMFPACK> jacobian_matrix_factorization;
 
  };

  template <int dim>
  class BoundaryValues : public Function<dim>
  {
  public:
    BoundaryValues()
      : Function<dim>(dim + 1)
    {}
    virtual double value(const Point<dim> & p,
                         const unsigned int component) const override;
  };

  template <int dim>
  double BoundaryValues<dim>::value(const Point<dim> & p,
                                    const unsigned int component) const
  {
    Assert(component < this->n_components,
           ExcIndexRange(component, 0, this->n_components));
    if (component == 0 && std::abs(p[dim - 1] - 1.0) < 1e-10)
      return 1.0;

    return 0;
  }

  template <class PreconditionerMp>
  class BlockSchurPreconditioner : public Subscriptor
  {
  public:
    BlockSchurPreconditioner(double                           gamma,
                             double                           viscosity,
                             const BlockSparseMatrix<double> &S,
                             const SparseMatrix<double> &     P,
                             const PreconditionerMp &         Mppreconditioner);

    void vmult(BlockVector<double> &dst, const BlockVector<double> &src) const;

  private:
    const double                     gamma;
    const double                     viscosity;
    const BlockSparseMatrix<double> &stokes_matrix;
    const SparseMatrix<double> &     pressure_mass_matrix;
    const PreconditionerMp &         mp_preconditioner;
    SparseDirectUMFPACK              A_inverse;
  };

  template <class PreconditionerMp>
  BlockSchurPreconditioner<PreconditionerMp>::BlockSchurPreconditioner(
    double                           gamma,
    double                           viscosity,
    const BlockSparseMatrix<double> &S,
    const SparseMatrix<double> &     P,
    const PreconditionerMp &         Mppreconditioner)
    : gamma(gamma)
    , viscosity(viscosity)
    , stokes_matrix(S)
    , pressure_mass_matrix(P)
    , mp_preconditioner(Mppreconditioner)
  {
    A_inverse.initialize(stokes_matrix.block(0, 0));
  }

  template <class PreconditionerMp>
  void BlockSchurPreconditioner<PreconditionerMp>::vmult(
    BlockVector<double> &      dst,
    const BlockVector<double> &src) const
  {
    Vector<double> utmp(src.block(0));

    {
      SolverControl solver_control(1000, 1e-6 * src.block(1).l2_norm());
      SolverCG<Vector<double>> cg(solver_control);

      dst.block(1) = 0.0;
      cg.solve(pressure_mass_matrix,
               dst.block(1),
               src.block(1),
               mp_preconditioner);
      dst.block(1) *= -(viscosity + gamma);
    }

    {
      stokes_matrix.block(0, 1).vmult(utmp, dst.block(1));
      utmp *= -1.0;
      utmp += src.block(0);
    }

    A_inverse.vmult(dst.block(0), utmp);
  }

  template <int dim>
  StationaryNavierStokes<dim>::StationaryNavierStokes(const unsigned int degree)
    : viscosity(1.0 / 2500.0)
    , gamma(1.0)
    , degree(degree)
    , triangulation(Triangulation<dim>::maximum_smoothing)
    , fe(FE_Q<dim>(degree + 1), dim, FE_Q<dim>(degree), 1)
    , dof_handler(triangulation)
  {}

  template <int dim>
  void StationaryNavierStokes<dim>::setup_dofs()
  {
    system_matrix.clear();
    pressure_mass_matrix.clear();

    jacobian_matrix.clear();

    dof_handler.distribute_dofs(fe);

    std::vector<unsigned int> block_component(dim + 1, 0);
    block_component[dim] = 1;
    DoFRenumbering::component_wise(dof_handler, block_component);

    dofs_per_block =
      DoFTools::count_dofs_per_fe_block(dof_handler, block_component);
    unsigned int dof_u = dofs_per_block[0];
    unsigned int dof_p = dofs_per_block[1];

    const FEValuesExtractors::Vector velocities(0);
    {
      nonzero_constraints.clear();

      DoFTools::make_hanging_node_constraints(dof_handler, nonzero_constraints);
      VectorTools::interpolate_boundary_values(dof_handler,
                                               0,
                                               BoundaryValues<dim>(),
                                               nonzero_constraints,
                                               fe.component_mask(velocities));
    }
    nonzero_constraints.close();

    {
      zero_constraints.clear();

      DoFTools::make_hanging_node_constraints(dof_handler, zero_constraints);
      VectorTools::interpolate_boundary_values(dof_handler,
                                               0,
                                               Functions::ZeroFunction<dim>(
                                                 dim + 1),
                                               zero_constraints,
                                               fe.component_mask(velocities));
    }
    zero_constraints.close();

    std::cout << "Number of active cells: " << triangulation.n_active_cells()
              << std::endl
              << "Number of degrees of freedom: " << dof_handler.n_dofs()
              << " (" << dof_u << " + " << dof_p << ')' << std::endl;
  }

  template <int dim>
  void StationaryNavierStokes<dim>::initialize_system()
  {
    {
      BlockDynamicSparsityPattern dsp(dofs_per_block, dofs_per_block);
      DoFTools::make_sparsity_pattern(dof_handler, dsp, nonzero_constraints);
      sparsity_pattern.copy_from(dsp);
    }

    system_matrix.reinit(sparsity_pattern);

    present_solution.reinit(dofs_per_block);
    newton_update.reinit(dofs_per_block);
    system_rhs.reinit(dofs_per_block);

    jacobian_matrix.reinit(sparsity_pattern);
    kinsol_solution.reinit(dofs_per_block);
    jacobian_matrix_factorization.reset();
  }

  template <int dim>
  void StationaryNavierStokes<dim>::compute_residual(
    const BlockVector<double> &kinsol_eval_point,
    BlockVector<double> &residual)
  {
    residual = 0;

    std::cout << "Computing residual vector..." << std::flush;

    const QGauss<dim> quadrature_formula(degree + 2);

    FEValues<dim> fe_values(fe,
                            quadrature_formula,
                            update_values | update_quadrature_points |
                              update_JxW_values | update_gradients);

    const unsigned int dofs_per_cell = fe.n_dofs_per_cell();
    const unsigned int n_q_points    = quadrature_formula.size();

    const FEValuesExtractors::Vector velocities(0);
    const FEValuesExtractors::Scalar pressure(dim);

    Vector<double>     local_residual(dofs_per_cell);

    std::vector<types::global_dof_index> local_dof_indices(dofs_per_cell);

    std::vector<Tensor<1, dim>> present_velocity_values(n_q_points);
    std::vector<Tensor<2, dim>> present_velocity_gradients(n_q_points);
    std::vector<double>         present_pressure_values(n_q_points);

    std::vector<double>         div_phi_u(dofs_per_cell);
    std::vector<Tensor<1, dim>> phi_u(dofs_per_cell);
    std::vector<Tensor<2, dim>> grad_phi_u(dofs_per_cell);
    std::vector<double>         phi_p(dofs_per_cell);

    for (const auto &cell : dof_handler.active_cell_iterators())
      {
        fe_values.reinit(cell);

        local_residual    = 0;

        fe_values[velocities].get_function_values(kinsol_eval_point,
                                                  present_velocity_values);

        fe_values[velocities].get_function_gradients(
          kinsol_eval_point, present_velocity_gradients);

        fe_values[pressure].get_function_values(kinsol_eval_point,
                                                present_pressure_values);

        for (unsigned int q = 0; q < n_q_points; ++q)
          {
            for (unsigned int k = 0; k < dofs_per_cell; ++k)
              {
                div_phi_u[k]  = fe_values[velocities].divergence(k, q);
                grad_phi_u[k] = fe_values[velocities].gradient(k, q);
                phi_u[k]      = fe_values[velocities].value(k, q);
                phi_p[k]      = fe_values[pressure].value(k, q);
              }

            for (unsigned int i = 0; i < dofs_per_cell; ++i)
              {
                double present_velocity_divergence =
                  trace(present_velocity_gradients[q]);
                local_residual(i) -=
                  (-viscosity * scalar_product(present_velocity_gradients[q],
                                               grad_phi_u[i]) -
                   present_velocity_gradients[q] * present_velocity_values[q] *
                     phi_u[i] +
                   present_pressure_values[q] * div_phi_u[i] +
                   present_velocity_divergence * phi_p[i] -
                   gamma * present_velocity_divergence * div_phi_u[i]) *
                  fe_values.JxW(q);
              }
          }

        cell->get_dof_indices(local_dof_indices);
        zero_constraints.distribute_local_to_global(local_residual,
                                                    local_dof_indices,
                                                    residual);
      }

    std::cout << " norm=" << residual.l2_norm() << std::endl;
  }

  template <int dim>
  void StationaryNavierStokes<dim>::compute_jacobian(
    const BlockVector<double> &kinsol_eval_point)
  {
    jacobian_matrix = 0;

    std::cout << "Computing jacobian matrix..." << std::endl;

    QGauss<dim> quadrature_formula(degree + 2);

    FEValues<dim> fe_values(fe,
                            quadrature_formula,
                            update_values | update_quadrature_points |
                              update_JxW_values | update_gradients);

    const unsigned int dofs_per_cell = fe.n_dofs_per_cell();
    const unsigned int n_q_points    = quadrature_formula.size();

    const FEValuesExtractors::Vector velocities(0);
    const FEValuesExtractors::Scalar pressure(dim);

    FullMatrix<double> cell_matrix(dofs_per_cell, dofs_per_cell);

    std::vector<types::global_dof_index> local_dof_indices(dofs_per_cell);

    std::vector<Tensor<1, dim>> present_velocity_values(n_q_points);
    std::vector<Tensor<2, dim>> present_velocity_gradients(n_q_points);
    std::vector<double>         present_pressure_values(n_q_points);

    std::vector<double>         div_phi_u(dofs_per_cell);
    std::vector<Tensor<1, dim>> phi_u(dofs_per_cell);
    std::vector<Tensor<2, dim>> grad_phi_u(dofs_per_cell);
    std::vector<double>         phi_p(dofs_per_cell);

    for (const auto &cell : dof_handler.active_cell_iterators())
      {
        fe_values.reinit(cell);

        cell_matrix = 0;

        fe_values[velocities].get_function_values(kinsol_eval_point,
                                                  present_velocity_values);

        fe_values[velocities].get_function_gradients(
          kinsol_eval_point, present_velocity_gradients);

        fe_values[pressure].get_function_values(kinsol_eval_point,
                                                present_pressure_values);

        for (unsigned int q = 0; q < n_q_points; ++q)
          {
            for (unsigned int k = 0; k < dofs_per_cell; ++k)
              {
                div_phi_u[k]  = fe_values[velocities].divergence(k, q);
                grad_phi_u[k] = fe_values[velocities].gradient(k, q);
                phi_u[k]      = fe_values[velocities].value(k, q);
                phi_p[k]      = fe_values[pressure].value(k, q);
              }

            for (unsigned int i = 0; i < dofs_per_cell; ++i)
              {
                for (unsigned int j = 0; j < dofs_per_cell; ++j)
                  {
                    // Picard iteration
                    cell_matrix(i, j) +=
                      (viscosity *
                          scalar_product(grad_phi_u[j], grad_phi_u[i]) +
                          grad_phi_u[j] * present_velocity_values[q] *
                            phi_u[i] -
                        div_phi_u[i] * phi_p[j] - phi_p[i] * div_phi_u[j] +
                        gamma * div_phi_u[j] * div_phi_u[i] +
                        phi_p[i] * phi_p[j]) *
                      fe_values.JxW(q);
                  }
              }
          }

        cell->get_dof_indices(local_dof_indices);

        zero_constraints.distribute_local_to_global(cell_matrix,
                                                    local_dof_indices,
                                                    jacobian_matrix);
      }

    pressure_mass_matrix.reinit(sparsity_pattern.block(1, 1));
    pressure_mass_matrix.copy_from(jacobian_matrix.block(1, 1));

    jacobian_matrix.block(1, 1) = 0;
  }

  template <int dim>
  void StationaryNavierStokes<dim>::solve_kinsol(
    const BlockVector<double> &rhs,
    BlockVector<double> &solution,
    const double /*tolerance*/)
  {
    std::cout << "GRMES solve..." << std::endl;

    SolverControl solver_control(jacobian_matrix.m(),
                                 1e-4 * rhs.l2_norm(),
                                 true);

    SolverFGMRES<BlockVector<double>> gmres(solver_control);
    SparseILU<double>                 pmass_preconditioner;
    pmass_preconditioner.initialize(pressure_mass_matrix,
                                    SparseILU<double>::AdditionalData());

    const BlockSchurPreconditioner<SparseILU<double>> preconditioner(
      gamma,
      viscosity,
      jacobian_matrix,
      pressure_mass_matrix,
      pmass_preconditioner);

    gmres.solve(jacobian_matrix, solution, rhs, preconditioner);
    std::cout << "FGMRES steps: " << solver_control.last_step() << std::endl;

    zero_constraints.distribute(solution);
  }

  template <int dim>
  void StationaryNavierStokes<dim>::refine_mesh()
  {
    Vector<float> estimated_error_per_cell(triangulation.n_active_cells());
    const FEValuesExtractors::Vector velocity(0);
    KellyErrorEstimator<dim>::estimate(
      dof_handler,
      QGauss<dim - 1>(degree + 1),
      std::map<types::boundary_id, const Function<dim> *>(),
      present_solution,
      estimated_error_per_cell,
      fe.component_mask(velocity));

    GridRefinement::refine_and_coarsen_fixed_number(triangulation,
                                                    estimated_error_per_cell,
                                                    0.3,
                                                    0.0);

    triangulation.prepare_coarsening_and_refinement();
    SolutionTransfer<dim, BlockVector<double>> solution_transfer(dof_handler);
    solution_transfer.prepare_for_coarsening_and_refinement(present_solution);
    triangulation.execute_coarsening_and_refinement();

    setup_dofs();

    BlockVector<double> tmp(dofs_per_block);

    solution_transfer.interpolate(present_solution, tmp);
    nonzero_constraints.distribute(tmp);

    initialize_system();
    present_solution = tmp;
  }

  template <int dim>
  void StationaryNavierStokes<dim>::output_results(
    const unsigned int output_index) const
  {
    std::vector<std::string> solution_names(dim, "velocity");
    solution_names.emplace_back("pressure");

    std::vector<DataComponentInterpretation::DataComponentInterpretation>
      data_component_interpretation(
        dim, DataComponentInterpretation::component_is_part_of_vector);
    data_component_interpretation.push_back(
      DataComponentInterpretation::component_is_scalar);
    DataOut<dim> data_out;
    data_out.attach_dof_handler(dof_handler);
    data_out.add_data_vector(present_solution,
                             solution_names,
                             DataOut<dim>::type_dof_data,
                             data_component_interpretation);
    data_out.build_patches();

    std::ofstream output(std::to_string(1.0 / viscosity) + "-solution-" +
                         Utilities::int_to_string(output_index, 4) + ".vtk");
    data_out.write_vtk(output);
  }

  template <int dim>
  void StationaryNavierStokes<dim>::process_solution(unsigned int refinement)
  {
    std::ofstream f(std::to_string(1.0 / viscosity) + "-line-" +
                    std::to_string(refinement) + ".txt");
    f << "# y u_x u_y" << std::endl;

    Point<dim> p;
    p(0) = 0.5;
    p(1) = 0.5;

    f << std::scientific;

    for (unsigned int i = 0; i <= 100; ++i)
      {
        p(dim - 1) = i / 100.0;

        Vector<double> tmp_vector(dim + 1);
        VectorTools::point_value(dof_handler, present_solution, p, tmp_vector);
        f << p(dim - 1);

        for (int j = 0; j < dim; ++j)
          f << ' ' << tmp_vector(j);
        f << std::endl;
      }
  }
  
  template <int dim>
  void StationaryNavierStokes<dim>::run_kinsol()
  {
    GridGenerator::hyper_cube(triangulation);
    triangulation.refine_global(6);

    std::cout << "viscosity = " << viscosity << std::endl;

    setup_dofs();
    initialize_system();

    nonzero_constraints.distribute(kinsol_solution);

    int refinement_cycle = 0;

    const double target_tolerance = 1e-12;
    std::cout << "  Target_tolerance: " << target_tolerance << std::endl
              << std::endl;

    {
      typename kinsol::AdditionalData additional_data;
      additional_data.function_tolerance = target_tolerance;

      kinsol::AdditionalData::SolutionStrategy strategy;
      strategy = kinsol::AdditionalData::picard;
      additional_data.strategy = strategy;
      additional_data.anderson_subspace_size = 10;

      kinsol nonlinear_solver(additional_data);

      nonlinear_solver.reinit_vector = 
      [&](BlockVector<double> &x) {
        x.reinit(dofs_per_block);
      };

      nonlinear_solver.residual =
      [&](const BlockVector<double> &kinsol_eval_point,
          BlockVector<double> &      residual) {
        compute_residual(kinsol_eval_point, residual);
  
        return 0;
      };

      nonlinear_solver.setup_jacobian =
      [&](const BlockVector<double> &current_u,
          const BlockVector<double> & /*current_f*/) {
        compute_jacobian(current_u);
  
        return 0;
      };

      nonlinear_solver.solve_with_jacobian = 
      [&](const BlockVector<double> &rhs,
          BlockVector<double> &      dst,
          const double tolerance) {
        this->solve_kinsol(rhs, dst, tolerance);
 
        return 0;
      };

      if (false)
      {
        // start with Stokes solution:
        kinsol_solution = present_solution;
      }
      else {
        // start with 0 (but correct boundary values)
      kinsol_solution=0;
      nonzero_constraints.distribute(kinsol_solution);
      }

      nonlinear_solver.solve(kinsol_solution);
      nonzero_constraints.distribute(kinsol_solution);
      output_results(0);
    }
  }


} // namespace Step57

int main()
{
  try
    {
      using namespace Step57;

      // Timer start
      auto start = high_resolution_clock::now();

      StationaryNavierStokes<2> flow(/* degree = */ 1);
      flow.run_kinsol();

      // Timer end
      auto end = high_resolution_clock::now();

      // Computational time
      auto duration = duration_cast<microseconds>(end - start);

      // output stuff
      std::cout << "elapsed time: " << duration.count() * 1e-6
                << " seconds" << std::endl;
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