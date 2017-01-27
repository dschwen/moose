# CALPHAD Thermodynamic Database free energy extration

The ```free_energy.py``` tool allows users to extract free energy expressions from ```*.tdb```
thermodynamic database files (ThermoCalc format). The tool exports a list of MOOSE Material blocks
for each phase (or a user specified subset of phases). The CALPHAD functional expressions are
implemented using the ```DerivativeParsedMaterial``` class.

It is up to the user to construct a full MOOSE input file around these material blocks and to rename
the variables used in the exported form to more suitable names.

## Installation

The [pycalphad](https://github.com/richardotis/pycalphad) Python module is required to perform the
parsing of the ```*.tdb``` files. [SymPy](https://github.com/sympy/sympy) is required to build the
functional forms of the calphad expressions.

To install and/or upgrade these prerequisites use conda (requires the `miniconda`
module to be loaded):

```
conda create -n calphad --clone=/opt/moose/miniconda
conda install -n calphad sympy
conda install -n calphad --channel https://conda.anaconda.org/conda-forge pycalphad
```

The new environment needs to be activated using:

```
source activate jupyter
```


## Thermodynamic databases

Database files can be obtained online at

* [Computational Phase Diagram Database](http://cpddb.nims.go.jp/index_en.html/) (CPDDB)
