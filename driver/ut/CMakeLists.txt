find_package (Threads)

add_executable(skm-odbc-ut
    main.cpp
    escape_sequences_ut.cpp
    lexer_ut.cpp
)

target_link_libraries(skm-odbc-ut
    skm-odbc-escaping
    gtest-lib
    Threads::Threads
)

add_test(NAME "skm-odbc-ut" COMMAND "skm-odbc-ut")
