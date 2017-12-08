Beam Web Services Data Types
============================

.. js:class:: beam.service_locator.DirectoryEntry.Type

    Enumerates the types of directory entries, specifying whether an entry
    represents an account or a directory.

  .. js:attribute:: NONE = -1

    The directory entry is invalid.

  .. js:attribute:: ACCOUNT = 0

    The directory entry represents an account.

  .. js:attribute:: DIRECTORY = 1

    The directory entry represents a directory.

.. js:class:: beam.service_locator.DirectoryEntry

    Represents a directory entry.

  .. js:attribute:: type
    The type of directory entry.
    :js:class:`beam.service_locator.DirectoryEntry.Type`
