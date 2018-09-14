arikaraInfluence
==========================

**How to call command in python:**

.. code-block:: python

    from maya import cmds

    cmds.arikaraInfluence()

=================
Mel exemples
=================

.. topic:: To Add Influences:

    arikaraInfluence -add [objects]

.. topic:: To Remove Influences:

    arikaraInfluence -remove [objects]

.. topic:: To Remove Unused Influences:

    arikaraInfluence -removeUnused *(double: minWeight)* object

.. topic:: To Set Max Influences:

    arikaraInfluence -maxInfluence *(int: maxInfl)* object

.. topic:: To Reset Bind Pose:

    arikaraInfluence -resetBindPose object

**Info:**

When adding or removing influences, the last object in the selection must be the
skinCluster object.

You can specify the skinCluster itself or the object that has the skinCluster.

.. list-table:: Flags:
   :widths: 15 60 15
   :header-rows: 1

   * - Flags
     - Description
     - Argument Type
   * - addInfluences (add)
     - Add specified influences to the specified skinCluster.
       (influences are specified by the selection or in the commands.)
     - no args 
   * - remove (rem)
     - Remove specified influences to the specified skinCluster.
       (influences are specified by the selection or in the commands.)
     - no args
   * - removeUnused (ru)
     - Remove the unused influences in the specified skincluster.
       The value specified is the minimum value an influences must have 
       on a vertex to be kept in the skinCluster.
     - double
   * - maxInfluence (mi)
     - Set maximum influence count on each vertices.
     - int
   * - resetBindPose (rbp)
     - Reset the bindPose given the current frame influence transform.
     - no args
