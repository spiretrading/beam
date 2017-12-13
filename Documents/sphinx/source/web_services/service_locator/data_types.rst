Beam Web Services Data Types
============================

.. json:object:: beam.service_locator.DirectoryEntry

    Represents a directory entry.

    :property type: :json:object:`number` The type of directory entry.
      Refer to :json:object:`beam.service_locator.DirectoryEntry.Type`
    :property id: :json:object:`number` The entry's unique id.
    :property name: :json:object:`string` The entry's name.

.. json:object:: beam.service_locator.DirectoryEntry.Type

    Enumerates the types of directory entries, specifying whether an entry
    represents an account or a directory.

    :property NONE=-1: The directory entry is invalid.
    :property ACCOUNT=0: The directory entry represents an account.
    :property DIRECTORY=1: The directory entry represents a directory.
