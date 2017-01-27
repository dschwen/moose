!!! info
    This is a work in progress.

A script to export MOOSE readable free energy expressions from ```*.tdb``` thermodynamic ]
database files is located at

```bash
moose/python/calphad/free_energy.py
```

The `free_energy.py` tool allows users to extract free energy expressions from `*.tdb`
thermodynamic database files (ThermoCalc format). The tool exports a list of MOOSE Material blocks
for each phase (or a user specified subset of phases). The CALPHAD functional expressions are
implemented using the [`DerivativeParsedMaterial`](/DerivativeParsedMaterial.md) class.

It is up to the user to construct a full MOOSE input file around these material
blocks and to rename the variables used in the exported form to more suitable names.

## Installation

The [pycalphad](https://github.com/richardotis/pycalphad) Python module is required to perform the
parsing of the `*.tdb` files. [SymPy](https://github.com/sympy/sympy) is required to build the
functional forms of the calphad expressions.

To install and/or upgrade these prerequisites use conda (requires the `miniconda`
module to be loaded):

```bash
conda create -n calphad --clone=/opt/moose/miniconda
conda install -n calphad sympy
conda install -n calphad --channel https://conda.anaconda.org/conda-forge pycalphad
```

The new environment needs to be activated using:

```bash
source activate jupyter
```

## Thermodynamic databases

Database files can be obtained online at

* [Computational Phase Diagram Database](http://cpddb.nims.go.jp/index_en.html) (CPDDB)

## Example

Fetch a TDB file like the FeCr database [crfe_her.TDB](http://cpddb.nims.go.jp/cpddb/cr-elem/crfe/crfe_her.TDB)
from CPDDB (free account registration required).

In the `moose/python/calphad` directory with the `calphad` conda environment active
run

```bash
./free_energy.py crfe_her.TDB.txt
```

The output wil be four [`DerivativeParsedMaterial`](/DerivativeParsedMaterial.md)
block to be inserted in the `[Materials]` section of a MOOSE input file.
You shoudl ses blocks for the sigma, fcc A1, bcc A2, and liquid phases. Each
phase will be parameterized by a temperature `T` and at least two concentration
variables for iron (`FE`) and chromium (`CR`), named with phase name as a prefix.
Some phases have multiple species concentrations, which refer to sublattices and/or
interstitial sites.

If we ignore the `F_SIGMA` block with two iron and two chromium concentrations we
are left with the fcc, bcc, and liquid phases. In those we can replace the iron
concentration with one minus teh chromium concentration. In th fcc block we would
add `FCC_A10FE := 1 - FCC_A10CR;` at the start of the function expression.

Depending on the multiphase model being used we can simplify further. For [KKS](Multi Phase/KKS.md)
we would retain the phase concentrations, and for [WBM](Multi Phase/WBM.md)] we'd
Reduce all phase chromium concentrations to a single global `c_Cr` concentration
by prepending `FCC_A10CR := c_Cr;`, `BCC_A20CR := c_Cr;`, and `LIQUID0CR := c_Cr;`
respectively.
