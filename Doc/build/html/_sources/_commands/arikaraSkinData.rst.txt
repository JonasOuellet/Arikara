arikaraSkinData
==========================

**How to call command in python:**

.. code-block:: python

    from maya import cmds

    cmds.arikaraSkinData()

=================
Mel exemples
=================

.. topic:: To Save:

    arikaraSkinData -save -path *(str: Path)* -file *(str: FileName)* object

.. topic:: To Load:

    arikaraSkinData -load -path *(str: Path)* -file *(str: FileName)* [objects]

.. topic:: Create a clean skinCluster:

    arikaraSkinData -load -clean -loadbindMatrix object

.. topic:: Load by closest local vertex Pos:

    arikaraSkinData -load -position -bias *(double: bias)* [objects]

.. topic:: Load by closest world vertex Pos:

    arikaraSkinData -load -position -worldSpace [objects]

.. list-table:: Flags:
   :widths: 15 60 15
   :header-rows: 1

   * - Flags
     - Description
     - Argument Type
   * - save (s)
     - Save skin data for current object.
       *p* and *f* flags can be use to specify the file path. 
       Otherwise, default path will be used.
     - no args
   * - load (l)
     - Load skin data for current object.  It is possible to load data only for specified vertices.
       *p* and *f* flags can be use to specify the file path. 
       Otherwise, last saved file will be used.
     - no args
   * - path (p)
     - Path to folder where to search for skin data.
     - string
   * - file (f)
     - SkinData file name
     - string
   * - loadBindMatrix (lbm)
     - Load the bind matrix data from the file.
     - no args
   * - clean (c)
     - Create a brand new skinCluster. (Delete the skinCluster on the object if there is one)m
     - no args
   * - position (pos)
     - Instead of loading weight by matching index, load weight by closest point index.
       must be specified if the current geometry doesnt have the same vertex count of the one 
       specified in the file.
     - no args
   * - worldSpace (ws)
     - Usefull when loading weight by position. Find matching world space position instead
       of local position.
     - no args
   * - suffix (suf)
     - Suffix to add to influences loaded from file for name matching
     - string
   * - prefix (pre)
     - Prefix to add to inflences loaded from file for name matching
     - string
   * - bias (b)
     - Bias used for matching the vertex pos.  Bigger is the bias shorter the calculation could take
       but precision is impacted.
     - double

====================================================================

:ref:`optionfile-reference`

.. code-block:: JavaScript

    "arikaraSkinData": {
        "defaultPath": "C:\\Users\\gsq_jouellet\\Documents\\maya\\ArikaraSkin\\SkinData",
        "prefix": "",
        "suffix": "",
        "bias": 0.001;
    }

**defaultPath:**
    Default path where to save the skinData file (.ars)

**prefix:**
    Default prefix to use for matching influences when loading a skinData.
    Usefull for namespaces

**suffix:**
    Default suffix to use for matching influences when loading a skinData.
    Usefull for namespaces

**bias:**
    Default bias value.
