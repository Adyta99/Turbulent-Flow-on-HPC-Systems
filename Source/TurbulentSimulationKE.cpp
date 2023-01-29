#include "TurbulentSimulationKE.hpp"

TurbulentSimulationKE::TurbulentSimulationKE(Parameters& parameters, TurbulentFlowFieldKE& flowField):
  Simulation(parameters, flowField),
  turbflowFieldKE_(flowField),
  TurbulentFGHStencilKE_(parameters),
  TurbulentFGHIteratorKE_(turbflowFieldKE_, parameters, TurbulentFGHStencilKE_),
  nuTStencilKE_(parameters),
  nuTIteratorKE_(turbflowFieldKE_, parameters, nuTStencilKE_, 1, 0),
  globalTurbulentBoundaryFactory_(parameters),
  wallkIterator_(globalTurbulentBoundaryFactory_.getGlobalBoundaryKIterator(turbflowFieldKE_)),
  wallEpsilonIterator_(globalTurbulentBoundaryFactory_.getGlobalBoundaryEpsilonIterator(turbflowFieldKE_)),
  wallnuTIterator_(globalTurbulentBoundaryFactory_.getGlobalBoundarynuTIterator(turbflowFieldKE_)),
  hStencil_(parameters),
  hIterator_(turbflowFieldKE_, parameters, hStencil_, 0, 0),
  dtStencil_(parameters),
  dtIterator_(turbflowFieldKE_, parameters, dtStencil_),
  kStencil_(parameters),
  kIterator_(flowField, parameters, kStencil_, 1, 0),
  epsilonStencil_(parameters),
  epsilonIterator_(flowField, parameters, epsilonStencil_, 1, 0),
  ppmTurbulentKE_(parameters, turbflowFieldKE_),
  MaxKStencil_(parameters),
  MaxKIterator_(flowField, parameters, MaxKStencil_, 1, 0),
  MaxEpsStencil_(parameters),
  MaxEpsIterator_(flowField, parameters, MaxEpsStencil_, 1, 0) {}

void TurbulentSimulationKE::initializeFlowField() {
  Simulation::initializeFlowField();
  hUpdate();
  if (parameters_.simulation.scenario == "channel") {
    // Currently, a particular initialisation is only required for the taylor-green vortex.
    Stencils::InitkFlowFieldStencil     kInitStencil(parameters_);
    FieldIterator<TurbulentFlowFieldKE> kInitIterator(turbflowFieldKE_, parameters_, kInitStencil, 0, 0);
    kInitIterator.iterate();
    wallkIterator_.iterate();

    Stencils::InitEpsilonFlowFieldStencil epsInitStencil(parameters_);
    FieldIterator<TurbulentFlowFieldKE>   epsInitIterator(turbflowFieldKE_, parameters_, epsInitStencil, 0, 0);
    epsInitIterator.iterate();
    wallEpsilonIterator_.iterate();
  }
  nuTUpdate();

}

void TurbulentSimulationKE::solveTimestep() {

  ppmTurbulentKE_.communicateViscosity();
  // Determine and set max. timestep which is allowed in this simulation. For Channel case run it with timestep set as 1e-4.
  setTimeStep();
  // parameters_.timestep.dt = 1e-4; 

  //Set boundary conditions for Velocity, FGH, k and epsilon
  wallVelocityIterator_.iterate();
  wallFGHIterator_.iterate();
  wallkIterator_.iterate();
  wallEpsilonIterator_.iterate();

  //Save k values from previous time step for calculation
  turbflowFieldKE_.updatekold();
  kIterator_.iterate();

//Save epsilion values from previous time step for calculation
  turbflowFieldKE_.updateepsold();
  epsilonIterator_.iterate();

//Update viscosity
  nuTUpdate();

//Calculate flux and RHS and solve Pressure Poisson
  TurbulentFGHIteratorKE_.iterate();

  rhsIterator_.iterate();

  solver_->solve();

  ppmTurbulentKE_.communicatePressure();

  velocityIterator_.iterate();
  obstacleIterator_.iterate();

  ppmTurbulentKE_.communicateVelocities();

}

void TurbulentSimulationKE::setTimeStep() {
  RealType localMin, globalMin;
  ASSERTION(parameters_.geometry.dim == 2 || parameters_.geometry.dim == 3);

  // Determine maximum velocity
  maxUStencil_.reset();
  MaxKStencil_.reset();
  MaxEpsStencil_.reset();

  maxUFieldIterator_.iterate();
  maxUBoundaryIterator_.iterate();

  dtStencil_.reset();
  dtIterator_.iterate();

  localMin = dtStencil_.getDt();

  MaxKIterator_.iterate();
  MaxEpsIterator_.iterate();

  if (parameters_.geometry.dim == 3) {
    parameters_.timestep.dt = 1.0 / (maxUStencil_.getMaxValues()[2] + std::numeric_limits<double>::min());
  } else {
    parameters_.timestep.dt = 1.0 / (maxUStencil_.getMaxValues()[0] + std::numeric_limits<double>::min());
  }

  localMin = std::min(
    localMin,
    std::min(
      parameters_.timestep.dt,
      std::min(
        1 / (maxUStencil_.getMaxValues()[1] + std::numeric_limits<double>::min()),
        std::min(
          1 / (2 * MaxKStencil_.getMaxValues() + std::numeric_limits<double>::min()),
          1 / (2 * MaxEpsStencil_.getMaxValues() + std::numeric_limits<double>::min())
        )
      )
    )
  );

  // Here, we select the type of operation before compiling. This allows to use the correct
  // data type for MPI. Not a concern for small simulations, but useful if using heterogeneous
  // machines.

  globalMin = MY_FLOAT_MAX;
  MPI_Allreduce(&localMin, &globalMin, 1, MY_MPI_FLOAT, MPI_MIN, PETSC_COMM_WORLD);

  parameters_.timestep.dt = globalMin;
  parameters_.timestep.dt *= parameters_.timestep.tau;
}

void TurbulentSimulationKE::hUpdate() { hIterator_.iterate(); }

void TurbulentSimulationKE::nuTUpdate() { nuTIteratorKE_.iterate(); }

void TurbulentSimulationKE::plotVTK(int timeStep, RealType simulationTime) {
  Stencils::TurbulentVTKStencilKE     TurbulentvtkStencilKE(parameters_);
  FieldIterator<TurbulentFlowFieldKE> TurbulentvtkIteratorKE(
    turbflowFieldKE_, parameters_, TurbulentvtkStencilKE, 1, 0
  );

  TurbulentvtkIteratorKE.iterate();
  TurbulentvtkStencilKE.write(turbflowFieldKE_, timeStep, simulationTime);
}
