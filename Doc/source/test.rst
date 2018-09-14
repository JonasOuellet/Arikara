================
test
================

:ref:`my-figure`

.. _my-reference-label:

Section to cross-reference
--------------------------

.. topic:: Topic Title

    Subsequent indented lines comprise
    the body of the topic, and are
    interpreted as body elements.


.. compound::

   The 'rm' command is very dangerous.  If you are logged
   in as root and enter ::

       cd /
       rm -rf *

   you will erase the entire contents of your file system.

.. sidebar:: Sidebar Title
   :subtitle: Optional Sidebar Subtitle

   Subsequent indented lines comprise
   the body of the sidebar, and are
   interpreted as body elements.

"To Ma Own Beloved Lassie: A Poem on her 17th Birthday", by
Ewan McTeagle (for Lassie O'Shea):

    .. line-block::

        Lend us a couple of bob till Thursday.
        I'm absolutely skint.
        But I'm expecting a postal order and I can pay you back
            as soon as it comes.
        Love, Ewan.

.. code:: python

  def my_function():
      "just a test"
      print 8/2


.. epigraph::

   No matter where you go, there you are.

   -- Buckaroo Banzai

.. container:: custom

   This paragraph might be rendered in a custom way.


.. table:: Truth table for "not"
   :widths: auto

   =====  =====
     A    not A
   =====  =====
   False  True
   True   False
   =====  =====

`Link text <http://www.sphinx-doc.org/en/stable/rest.html#internal-links>`_

This is a paragraph that contains `a link`_.

.. _a link: http://www.sphinx-doc.org/en/stable/rest.html#internal-links


.. _my-figure:

Section to cross-reference
--------------------------




``Ceci est un test de literal``