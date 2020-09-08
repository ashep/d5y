Cronus Server
=============

Development
-----------

.. sourcecode:: shell

    docker-compose -f docker-compose-dev.yml up --build


Production
----------

.. sourcecode:: shell

    DOCKER_HOST="ssh://user@host" docker-compose -f docker-compose-prod.yml up --build --detach
