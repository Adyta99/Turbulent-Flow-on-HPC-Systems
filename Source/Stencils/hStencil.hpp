#pragma once

#include "FieldStencil.hpp"
#include "FlowField.hpp"
#include "Parameters.hpp"

namespace Stencils {
  class hStencil: public FieldStencil<FlowField> {
  public:
    hStencil(const Parameters& parameters);
    ~hStencil() override = default;

    void apply(FlowField& flowField, int i, int j) override;
    void apply(FlowField& flowField, int i, int j, int k) override;
  };
} // namespace Stencils
