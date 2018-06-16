add_library(skm-odbc-escaping
    escape_sequences.cpp
    lexer.cpp
)

target_include_directories(skm-odbc-escaping PUBLIC ..)
