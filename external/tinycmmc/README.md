tinycmmc - Tiny CMake Module Collection
=======================================

Usage
-----

The `modules/` path can either be added directly to `CMAKE_MODULE_PATH`
without installation via:

    list(APPEND CMAKE_MODULE_PATH tinycmmc/modules/)

Or when installed it can by obtained via and added to `CMAKE_MODULE_PATH`:

    find_package(tinycmmc REQUIRED CONFIG)
    list(APPEND CMAKE_MODULE_PATH ${TINYCMMC_MODULE_PATH})
