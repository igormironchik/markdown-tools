http://docutils.sourceforge.net/docs/ref/rst/restructuredtext.html

A heading
#########

:field:     A field
    On another line, *with inline* **markup**.
:py:field:  A field with a colon (not handled correctly, currently)
:and space:

.. comment

text

.. multi-line
    comment

text again

..
    also multiline comment
..

.. directive:: argument another
    :option: value

    Directive content.

.. py:function:: a directive with a colon
    :py:option: an option with a colon (not handled correctly, currently)
    :option a: an option with a space

.. code-block::

    A code block

.. code::

    But this is a code block, too.

Inline ``code``, :ref:`interpreted role`, `trailing`:ref:,
:py:ref:`role with a colon`. The `default interpreted role` and regular text
after.

A reference_, `link <https://kde.org>`_ Plain link: https://kde.org. `An anonymous link <https://kde.org>`__. A footnote
reference [1]_.

.. [1] Footnote content

A text block.

    Block quote

    -- attribution

-   A list
-   Another item

    -   Sub item
    -   More

1.  Ordered
2.  list

Definition title
    Definition description
Definition title
    Definition description

(``code between parentheses``)
