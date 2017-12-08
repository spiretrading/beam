Beam Web Services
=================

.. http:post:: /api/service_locator/login

   Logs an account into the service.

  :param username: The account's username.
  :type username: string
  :param password: The account's password.
  :type password: string
  :statuscode 200: :json:object:`beam.service_locator.DirectoryEntry`
    The directory entry of the account that logged in.
