
//          Copyright Oliver Kowalke 2009.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CONTEXT_DETAIL_FCONTEXT_H
#define BOOST_CONTEXT_DETAIL_FCONTEXT_H

#include <hoost/config.hpp>
#include <hoost/cstdint.hpp>

#include <hoost/context/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace context {
namespace detail {

typedef void*   fcontext_t;

struct transfer_t {
    fcontext_t  fctx;
    void    *   data;
};

extern "C" BOOST_CONTEXT_DECL
transfer_t BOOST_CONTEXT_CALLDECL hoost_jump_fcontext( fcontext_t const to, void * vp);
extern "C" BOOST_CONTEXT_DECL
fcontext_t BOOST_CONTEXT_CALLDECL hoost_make_fcontext( void * sp, std::size_t size, void (* fn)( transfer_t) );

// based on an idea of Giovanni Derreta
extern "C" BOOST_CONTEXT_DECL
transfer_t BOOST_CONTEXT_CALLDECL hoost_ontop_fcontext( fcontext_t const to, void * vp, transfer_t (* fn)( transfer_t) );

}}}

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_CONTEXT_DETAIL_FCONTEXT_H
