Cronus Server
=============

Local development server
------------------------

.. sourcecode:: shell

    docker-compose -f docker-compose-dev.yml up --build


Deployment
----------

.. sourcecode:: shell

    DOCKER_HOST="ssh://user@host" docker-compose -f docker-compose-prod.yml up --build --detach
