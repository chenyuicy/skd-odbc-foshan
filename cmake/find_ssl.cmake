option (USE_INTERNAL_SSL_LIBRARY "Set to FALSE to use system *ssl library instead of bundled" ${MSVC})

set (OPENSSL_USE_STATIC_LIBS ${USE_STATIC_LIBRARIES})

if (NOT USE_INTERNAL_SSL_LIBRARY)
    if (APPLE)
        set (OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
        # https://rt.openssl.org/Ticket/Display.html?user=guest&pass=guest&id=2232
        if (USE_STATIC_LIBRARIES)
            message(WARNING "Disable USE_STATIC_LIBRARIES if you have linking problems with OpenSSL on MacOS")
        endif ()
    endif ()

    set(_save ${CMAKE_FIND_LIBRARY_SUFFIXES})

    # If you got error
    # /usr/bin/ld: /usr/lib/gcc/x86_64-linux-gnu/7/../../../x86_64-linux-gnu/libssl.a(s23_srvr.o): relocation R_X86_64_PC32 against symbol `ssl23_get_client_hello' can not be used when making a shared object; recompile with -fPIC
    # ues this option:
    if (OPENSSL_USE_SHARED_LIBS)
        list (REVERSE CMAKE_FIND_LIBRARY_SUFFIXES)
    endif ()

    find_package (OpenSSL)

    if (NOT OPENSSL_FOUND)
        # Try to find manually.
        set (OPENSSL_INCLUDE_PATHS "/usr/local/opt/openssl/include")
        set (OPENSSL_PATHS "/usr/local/opt/openssl/lib")
        find_path (OPENSSL_INCLUDE_DIR NAMES openssl/ssl.h PATHS ${OPENSSL_INCLUDE_PATHS})
        find_library (OPENSSL_SSL_LIBRARY ssl PATHS ${OPENSSL_PATHS})
        find_library (OPENSSL_CRYPTO_LIBRARY crypto PATHS ${OPENSSL_PATHS})
        if (OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY AND OPENSSL_INCLUDE_DIR)
            set (OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
            set (OPENSSL_FOUND 1)
        endif ()
    endif ()

    set(CMAKE_FIND_LIBRARY_SUFFIXES ${_save})

endif ()

if (NOT OPENSSL_FOUND)
    set (USE_INTERNAL_SSL_LIBRARY 1)
    set (OPENSSL_ROOT_DIR "${Skm_SOURCE_DIR}/contrib/ssl")
    set (OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
    if (NOT USE_STATIC_LIBRARIES AND TARGET crypto-shared AND TARGET ssl-shared)
        set (OPENSSL_CRYPTO_LIBRARY crypto-shared)
        set (OPENSSL_SSL_LIBRARY ssl-shared)
    else ()
        set (OPENSSL_CRYPTO_LIBRARY crypto)
        set (OPENSSL_SSL_LIBRARY ssl)
    endif ()
    set (OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
    set (OPENSSL_FOUND 1)
endif ()

message (STATUS "Using ssl=${OPENSSL_FOUND}: ${OPENSSL_INCLUDE_DIR} : ${OPENSSL_LIBRARIES}")
