# RigidSidesetAux

!syntax description /Bounds/RigidSidesetAux

## Overview

This aux kernel uses the transformation computed by a [RigidSideset](RigidSideset.md)
user object to calculate the disctance between the transformed undeformed sideset
and the displaced sideset. It can be use to meassure the efficacy of the `penalty` factor
chosen in the [RigidSidesetPenalty](RigidSidesetPenalty.md) boundary condition.

## Example Input File Syntax

!! Describe and include an example of how to use the RigidSidesetAux object.

!syntax parameters /Bounds/RigidSidesetAux

!syntax inputs /Bounds/RigidSidesetAux

!syntax children /Bounds/RigidSidesetAux
