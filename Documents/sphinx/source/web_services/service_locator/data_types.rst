Beam Web Services Data Types
============================

.. py:currentmodule:: beam.service_locator

.. py:class:: DirectoryEntry

    Represents a directory entry.

  .. py:attribute:: type

    :py:class:`DirectoryEntry.Type`
    The type of directory entry.

  .. py:attribute:: id

    `number`
    The entry's unique id.

  .. py:attribute:: name

    `string`
    The entry's name.

  .. py:class:: Type

      Enumerates the types of directory entries, specifying whether an entry
      represents an account or a directory.

    .. py:data:: NONE=-1

      The directory entry is invalid.

    .. py:data:: ACCOUNT=0

      The directory entry represents an account.

    .. py:data:: DIRECTORY=1

      The directory entry represents a directory.
