Beam Web Services Data Types
============================

.. cpp:enum-class:: beam.service_locator.DirectoryEntry.Type

   Indicates whether a directory entry represents an account or a directory.

.. cpp:enumerator:: beam.service_locator.DirectoryEntry.Type::NONE = -1

   The directory entry is invalid.

.. cpp:enumerator:: beam.service_locator.DirectoryEntry.Type::ACCOUNT = 0

   The directory entry represents an account.

.. cpp:enumerator:: beam.service_locator.DirectoryEntry.Type::ACCOUNT = 1

   The directory entry represents a directory.

.. json:object:: beam.service_locator.DirectoryEntry

   Stores a directory entry.

  :property type: The type of directory entry.
  :proptype type: :cpp:enum-class:`beam.service_locator.DirectoryEntry.Type`
  :property id: The entry's unique id.
  :proptype id: integer
  :property name: The name of the entry.
  :proptype name: string
