# RigidSideset

!syntax description /UserObjects/RigidSideset

## Overview

This side userobject iterates over all quadrature points in a sideset and

1. Computes the average translation of the sideset
2. The average axis of rotation of the sideset
3. The average angle of rotation around this axis

The userobject then provides a translation vector and a rotation matrix. That
represents the rigid body displacement of the undeformed sideset closest to the
deformed sideset. This information is required by the [RigidSidesetPenalty](RigidSidesetPenalty.md)
boundary condition.

The distance between the translated/rotated undeformed sideset can be visualized
with the [RigidSidesetAux](RigidSidesetAux.md) AuxKernel.

## Example Input File Syntax

!! Describe and include an example of how to use the RigidSideset object.

!syntax parameters /UserObjects/RigidSideset

!syntax inputs /UserObjects/RigidSideset

!syntax children /UserObjects/RigidSideset
