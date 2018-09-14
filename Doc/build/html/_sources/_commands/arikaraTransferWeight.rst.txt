arikaraTransferWeight
==========================

**How to call command in python:**

.. code-block:: python

    from maya import cmds

    cmds.arikaraTransferWeight()

=================
Mel exemples
=================

.. topic:: To Transfer Weight:

    arikaraSkinData -source *(str: sourceName)* -target *(str: targetName)* -value *(double: value)* [objects]

**Info:**

Transfer Weight can be done on specified vertex components or on the whole object.

.. list-table:: Flags:
   :widths: 15 60 15
   :header-rows: 1

   * - Flags
     - Description
     - Argument Type
   * - source (s)
     - Source influence
     - string
   * - target (t)
     - Target influence
     - string
   * - value (v)
     - Value of the weight to transfer. **Default:** 1.0
     - double
