arikaraMirrorWeight
==========================

**How to call command in python:**

.. code-block:: python

    from maya import cmds

    cmds.arikaraMirrorWeight()

=================
Mel exemples
=================

.. topic:: To Mirror Weights:

    arikaraSkinData -find *(str: findSubString)* -replace *(str: replaceSubString)* -axis *(int: axis)* -bias *(double: bias)* object


.. list-table:: Flags:
   :widths: 15 60 15
   :header-rows: 1

   * - Flags
     - Description
     - Argument Type
   * - find (f)
     - substring to find to match mirror influence. Default=*From OptionFile*
     - string
   * - replace (r)
     - substring to replace to match mirror influence. Default=*From OptionFile*
     - string
   * - axis (a)
     - Mirror Axis. Default = *From OptionFile*.
       0 = X, 1 = Y, 2 = Z.
     - int
   * - bias (b)
     - Bias value for finding matching vert. Default = *From OptionFile*.
       If distance between source vertex and mirror vertex is less then bias, those vertices will be matched.
     - double


====================================================================

:ref:`optionfile-reference`

Value not specified when calling the command will be set to these default value.

.. code-block:: JavaScript

    "arikaraMirrorWeight": {
        "axis": 0,
        "bias": 0.05,
        "find": "Right",
        "replace": "Right"
    }

**axis:**
    Default axis.

**bias:**
    Default bias.

**find:**
    Default find.

**replace:**
    Default replace.
