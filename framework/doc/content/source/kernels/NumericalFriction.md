# NumericalFriction

!syntax description /Kernels/NumericalFriction

## Overview

The residual contribution is proportional to the variable increment $\Delta u = u-u_{old}$.

!equation
R = p \cdot \Delta u \cdot \psi

Where $p$ is the penalty factor and $\psi$ is the test function.

## Example Input File Syntax

!! Describe and include an example of how to use the NumericalFriction object.

!syntax parameters /Kernels/NumericalFriction

!syntax inputs /Kernels/NumericalFriction

!syntax children /Kernels/NumericalFriction
