# RigidSidesetPenalty

!syntax description /BCs/RigidSidesetPenalty

## Overview

This integrated boundary condition  uses the transformation computed by a
[RigidSideset](RigidSideset.md) user object to calculate the disctance between
the transformed undeformed sideset (reference state) and the deformed sideset
(current state).  A force (proportional to the given `penalty` factor and the
distance between the two states is applied  to pull the sideset into the rigidly
transformed reference state.

In effect this boundary condition is a penalty enforcement of a rigid sideset.
Larger penalty factors result in stricter enforcement of the sideset rigidity, but
will eventually impact simulation convergence negatively.

## Example Input File Syntax

!! Describe and include an example of how to use the RigidSidesetPenalty object.

!syntax parameters /BCs/RigidSidesetPenalty

!syntax inputs /BCs/RigidSidesetPenalty

!syntax children /BCs/RigidSidesetPenalty
