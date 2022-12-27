#include "StdAfx.hpp"

#include "FGHStencil.hpp"

#include "Definitions.hpp"
#include "StencilFunctions.hpp"
#include "TurbulentFlowFieldKE.hpp"

Stencils::FGHStencil::FGHStencil(const Parameters& parameters):
  FieldStencil<FlowField>(parameters) {}

void Stencils::FGHStencil::apply(FlowField& flowField, int i, int j) {
  // Load local velocities into the center layer of the local array
  loadLocalVelocity2D(flowField, localVelocity_, i, j);
  loadLocalMeshsize2D(parameters_, localMeshsize_, i, j);

  RealType* const values = flowField.getFGH().getVector(i, j);

  // Now the localVelocity array should contain lexicographically ordered elements around the given index
  values[0] = computeF2D(localVelocity_, localMeshsize_, parameters_, parameters_.timestep.dt);
  values[1] = computeG2D(localVelocity_, localMeshsize_, parameters_, parameters_.timestep.dt);
}

void Stencils::FGHStencil::apply(FlowField& flowField, int i, int j, int k) {
  // The same as in 2D, with slight modifications.

  const int       obstacle = flowField.getFlags().getValue(i, j, k);
  RealType* const values   = flowField.getFGH().getVector(i, j, k);

  if ((obstacle & OBSTACLE_SELF) == 0) { // If the cell is fluid
    loadLocalVelocity3D(flowField, localVelocity_, i, j, k);
    loadLocalMeshsize3D(parameters_, localMeshsize_, i, j, k);

    if ((obstacle & OBSTACLE_RIGHT) == 0) { // If the right cell is fluid
      values[0] = computeF3D(localVelocity_, localMeshsize_, parameters_, parameters_.timestep.dt);
    }
    if ((obstacle & OBSTACLE_TOP) == 0) {
      values[1] = computeG3D(localVelocity_, localMeshsize_, parameters_, parameters_.timestep.dt);
    }
    if ((obstacle & OBSTACLE_BACK) == 0) {
      values[2] = computeH3D(localVelocity_, localMeshsize_, parameters_, parameters_.timestep.dt);
    }
  }
}

Stencils::TurbulentFGHStencil::TurbulentFGHStencil(const Parameters& parameters):
  FieldStencil<TurbulentFlowField>(parameters) {}

void Stencils::TurbulentFGHStencil::apply(TurbulentFlowField& flowField, int i, int j) {
  // Load local velocities into the center layer of the local array
  loadLocalVelocity2D(flowField, localVelocity_, i, j);
  loadLocalViscosity2D(flowField, localViscosity_, i, j);
  loadLocalMeshsize2D(parameters_, localMeshsize_, i, j);

  RealType* const values = flowField.getFGH().getVector(i, j);

  // Now the localVelocity array should contain lexicographically ordered elements around the given index
  values[0] = computeF2D_turbulent(
    localVelocity_, localViscosity_, localMeshsize_, parameters_, parameters_.timestep.dt
  );
  values[1] = computeG2D_turbulent(
    localVelocity_, localViscosity_, localMeshsize_, parameters_, parameters_.timestep.dt
  );
  // std::cout << "Hello from turbulent apply\n";
}

void Stencils::TurbulentFGHStencil::apply(TurbulentFlowField& flowField, int i, int j, int k) {
  // The same as in 2D, with slight modifications.

  const int       obstacle = flowField.getFlags().getValue(i, j, k);
  RealType* const values   = flowField.getFGH().getVector(i, j, k);

  if ((obstacle & OBSTACLE_SELF) == 0) { // If the cell is fluid
    loadLocalVelocity3D(flowField, localVelocity_, i, j, k);
    loadLocalMeshsize3D(parameters_, localMeshsize_, i, j, k);

    if ((obstacle & OBSTACLE_RIGHT) == 0) { // If the right cell is fluid
      values[0] = computeF3D_turbulent(
        localVelocity_, localViscosity_, localMeshsize_, parameters_, parameters_.timestep.dt
      );
    }
    if ((obstacle & OBSTACLE_TOP) == 0) {
      values[1] = computeG3D_turbulent(
        localVelocity_, localViscosity_, localMeshsize_, parameters_, parameters_.timestep.dt
      );
    }
    if ((obstacle & OBSTACLE_BACK) == 0) {
      values[2] = computeH3D_turbulent(
        localVelocity_, localViscosity_, localMeshsize_, parameters_, parameters_.timestep.dt
      );
    }
  }
}


Stencils::TurbulentFGHStencilKE::TurbulentFGHStencilKE(const Parameters& parameters):
  FieldStencil<TurbulentFlowFieldKE>(parameters) {}

void Stencils::TurbulentFGHStencilKE::apply(TurbulentFlowFieldKE& flowField, int i, int j) {
  // Load local velocities into the center layer of the local array
  loadLocalVelocity2D(flowField, localVelocity_, i, j);
  loadLocalViscosity2D(flowField, localViscosity_, i, j);
  loadLocalMeshsize2D(parameters_, localMeshsize_, i, j);

  RealType* const values = flowField.getFGH().getVector(i, j);

  // Now the localVelocity array should contain lexicographically ordered elements around the given index
  values[0] = computeF2D_turbulent(
    localVelocity_, localViscosity_, localMeshsize_, parameters_, parameters_.timestep.dt
  );
  values[1] = computeG2D_turbulent(
    localVelocity_, localViscosity_, localMeshsize_, parameters_, parameters_.timestep.dt
  );
  // std::cout << "Hello from turbulent apply\n";
}

