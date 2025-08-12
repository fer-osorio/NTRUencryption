// Commands the integration of gmpxx library and associated features (and maybe other libraries and feature in the future).
#ifndef NTRU_CONFIG_HPP
#define NTRU_CONFIG_HPP

// GMPXX configuration
#ifdef GMPXX_INCLUDED
    #define NTRU_HAS_GMPXX 1
    #include <gmpxx.h>
#else
    #define NTRU_HAS_GMPXX 0
#endif

// Feature availability macros
#if NTRU_HAS_GMPXX
    #define NTRU_GMPXX_METHOD(method_name) method_name
    #define NTRU_GMPXX_MEMBER(member_decl) member_decl
    #define NTRU_GMPXX_CONDITIONAL(code) code
#else
    #define NTRU_GMPXX_METHOD(method_name) method_name##_unavailable
    #define NTRU_GMPXX_MEMBER(member_decl) /* member_decl */
    #define NTRU_GMPXX_CONDITIONAL(code) /* code */
#endif

#endif // NTRU_CONFIG_HPP