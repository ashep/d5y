Cronus ESP8266 firmware
=======================

Preparing
---------

1. Download the `ESP8266 RTOS SDK`_.
2. Point the ``ESP_IDF`` env variable to the ESP8266 RTOS SDK location.
3. Download the `ESP toolchain`_.
4. Point the ``ESP_TOOLCHAIN`` env variable to the ESP toolchain location.
5. Setup the connection between your ESP8266 and computer.
6. Switch your ESP8266 to firmware upload mode.


Building
--------

.. sourcecode:: shell

    make flash flash_storage


.. _ESP8266 RTOS SDK: https://github.com/espressif/ESP8266_RTOS_SDK
.. _ESP toolchain: https://dl.espressif.com/dl/xtensa-lx106-elf-linux64-1.22.0-100-ge567ec7-5.2.0.tar.gz
