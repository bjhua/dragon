CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

PROJECT(DRAGON)

add_compile_options(-W -Wall -Werror -Wstrict-prototypes
        -Wmissing-prototypes
        -Wextra
        -Wconversion
        -Wno-sign-conversion
        -Wdouble-promotion
        -Wno-unused-parameter
        -Walloca
        -Wcast-qual
        -Wformat=2
        -Wformat-security
        -Wnull-dereference
        -Wstack-protector
        -Wvla
        -Warray-bounds
        -Warray-bounds-pointer-arithmetic
        -Wassign-enum -Wbad-function-cast
        -Wconditional-uninitialized
        -Wconversion
        -Wfloat-equal -Wformat-type-confusion
        -Widiomatic-parentheses
        -Wimplicit-fallthrough -Wloop-analysis
        -Wpointer-arith -Wshift-sign-overflow
        -Wshorten-64-to-32 -Wswitch-enum
        -Wtautological-constant-in-range-compare
        -Wunreachable-code-aggressive -Wthread-safety
        -Wthread-safety-beta -Wcomma
        -D_FORTIFY_SOURCE=3
        -fstack-protector-strong
        #-fsanitize=safe-stack
        -fPIE
        #-fstack-clash-protection
        -fsanitize=bounds -fsanitize-undefined-trap-on-error
        #-Wl,-z,relro -Wl,-z,now -Wl,-z,noexecstack -Wl,-z,separate-code
        -pedantic)

# AUX_SOURCE_DIRECTORY (../gc/ GC)
AUX_SOURCE_DIRECTORY(lib/ LIB)
AUX_SOURCE_DIRECTORY(atoms/ ATOMS)
AUX_SOURCE_DIRECTORY(control/ CONTROL)
AUX_SOURCE_DIRECTORY(main/ MAIN)
AUX_SOURCE_DIRECTORY(lexer/ LEXER)
AUX_SOURCE_DIRECTORY(parser/ PARSER)
AUX_SOURCE_DIRECTORY(ast/ AST)
AUX_SOURCE_DIRECTORY(elaborate/ ELABORATE)
AUX_SOURCE_DIRECTORY(hil/ HIL)
AUX_SOURCE_DIRECTORY(ssa/ SSA)
AUX_SOURCE_DIRECTORY(machine/ MACHINE)
AUX_SOURCE_DIRECTORY(c-codegen/ CCODEGEN)
AUX_SOURCE_DIRECTORY(x86/ X86)

SET(SRC_LIST
        main.c
        ${MAIN}
        ${GC}
        ${LIB}
        ${ATOMS}
        ${CONTROL}
        ${LEXER}
        ${PARSER}
        ${AST}
        ${ELABORATE}
        ${HIL}
        ${SSA}
        ${MACHINE}
        ${CCODEGEN}
        ${X86})

ADD_EXECUTABLE(dragon ${SRC_LIST})



