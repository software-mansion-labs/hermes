
//          Copyright Oliver Kowalke 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_CONTEXT_FIBER_H
#define BOOST_CONTEXT_FIBER_H

#include <hoost/context/detail/config.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <functional>
#include <memory>
#include <ostream>
#include <tuple>
#include <utility>

#include <hoost/assert.hpp>
#include <hoost/config.hpp>

#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
#include <hoost/context/detail/exchange.hpp>
#endif
#if defined(BOOST_NO_CXX17_STD_INVOKE)
#include <hoost/context/detail/invoke.hpp>
#endif
#include <hoost/context/detail/disable_overload.hpp>
#include <hoost/context/detail/exception.hpp>
#include <hoost/context/detail/fcontext.hpp>
#include <hoost/context/detail/tuple.hpp>
#include <hoost/context/fixedsize_stack.hpp>
#include <hoost/context/flags.hpp>
#include <hoost/context/preallocated.hpp>
#include <hoost/context/segmented_stack.hpp>
#include <hoost/context/stack_context.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_PREFIX
#endif

#if defined(__CET__) && defined(__unix__)
#  include <cet.h>
#  include <sys/mman.h>
#  define SHSTK_ENABLED (__CET__ & 0x2)
#  define BOOST_CONTEXT_SHADOW_STACK (SHSTK_ENABLED && SHADOW_STACK_SYSCALL)
#  define __NR_map_shadow_stack 451
#ifndef SHADOW_STACK_SET_TOKEN
#  define SHADOW_STACK_SET_TOKEN 0x1
#endif
#endif

#if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable: 4702)
#endif

namespace boost {
namespace context {
namespace detail {

inline
transfer_t fiber_unwind( transfer_t t) {
#ifdef HOOST_EXCEPTIONS_DISABLED
    abort();
#else
    throw forced_unwind( t.fctx);
    return { nullptr, nullptr };
#endif
}

template< typename Rec >
transfer_t fiber_exit( transfer_t t) noexcept {
    Rec * rec = static_cast< Rec * >( t.data);
#if BOOST_CONTEXT_SHADOW_STACK
    // destory shadow stack
    std::size_t ss_size = *((unsigned long*)(reinterpret_cast< uintptr_t >( rec)- 16));
    long unsigned int ss_base = *((unsigned long*)(reinterpret_cast< uintptr_t >( rec)- 8));
    munmap((void *)ss_base, ss_size);
#endif
    // destroy context stack
    rec->deallocate();
    return { nullptr, nullptr };
}

template< typename Rec >
void fiber_entry( transfer_t t) noexcept {
    // transfer control structure to the context-stack
    Rec * rec = static_cast< Rec * >( t.data);
    BOOST_ASSERT( nullptr != t.fctx);
    BOOST_ASSERT( nullptr != rec);
#ifndef HOOST_EXCEPTIONS_DISABLED
    try {
#endif
        // jump back to `create_context()`
        t = hoost_jump_fcontext( t.fctx, nullptr);
        // start executing
        t.fctx = rec->run( t.fctx);
#ifndef HOOST_EXCEPTIONS_DISABLED
    } catch ( forced_unwind const& ex) {
        t = { ex.fctx, nullptr };
    }
#endif
    BOOST_ASSERT( nullptr != t.fctx);
    // destroy context-stack of `this`context on next context
    hoost_ontop_fcontext( t.fctx, rec, fiber_exit< Rec >);
    BOOST_ASSERT_MSG( false, "context already terminated");
}

template< typename Ctx, typename Fn >
transfer_t fiber_ontop( transfer_t t) {
    BOOST_ASSERT( nullptr != t.data);
    auto p = *static_cast< Fn * >( t.data);
    t.data = nullptr;
    // execute function, pass fiber via reference
    Ctx c = p( Ctx{ t.fctx } );
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
    return { exchange( c.fctx_, nullptr), nullptr };
#else
    return { std::exchange( c.fctx_, nullptr), nullptr };
#endif
}

template< typename Ctx, typename StackAlloc, typename Fn >
class fiber_record {
private:
    stack_context                                       sctx_;
    typename std::decay< StackAlloc >::type             salloc_;
    typename std::decay< Fn >::type                     fn_;

    static void destroy( fiber_record * p) noexcept {
        typename std::decay< StackAlloc >::type salloc = std::move( p->salloc_);
        stack_context sctx = p->sctx_;
        // deallocate fiber_record
        p->~fiber_record();
        // destroy stack with stack allocator
        salloc.deallocate( sctx);
    }

public:
    fiber_record( stack_context sctx, StackAlloc && salloc,
            Fn && fn) noexcept :
        sctx_( sctx),
        salloc_( std::forward< StackAlloc >( salloc)),
        fn_( std::forward< Fn >( fn) ) {
    }

    fiber_record( fiber_record const&) = delete;
    fiber_record & operator=( fiber_record const&) = delete;

    void deallocate() noexcept {
        destroy( this);
    }

    fcontext_t run( fcontext_t fctx) {
        // invoke context-function
#if defined(BOOST_NO_CXX17_STD_INVOKE)
        Ctx c = boost::context::detail::invoke( fn_, Ctx{ fctx } );
#else
        Ctx c = std::invoke( fn_, Ctx{ fctx } );
#endif
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
        return exchange( c.fctx_, nullptr);
#else
        return std::exchange( c.fctx_, nullptr);
#endif
    }
};

template< typename Record, typename StackAlloc, typename Fn >
fcontext_t create_fiber1( StackAlloc && salloc, Fn && fn) {
    auto sctx = salloc.allocate();
    // reserve space for control structure
	void * storage = reinterpret_cast< void * >(
			( reinterpret_cast< uintptr_t >( sctx.sp) - static_cast< uintptr_t >( sizeof( Record) ) )
            & ~static_cast< uintptr_t >( 0xff) );
    // placment new for control structure on context stack
    Record * record = new ( storage) Record{
            sctx, std::forward< StackAlloc >( salloc), std::forward< Fn >( fn) };
    // 64byte gab between control structure and stack top
    // should be 16byte aligned
    void * stack_top = reinterpret_cast< void * >(
            reinterpret_cast< uintptr_t >( storage) - static_cast< uintptr_t >( 64) );
    void * stack_bottom = reinterpret_cast< void * >(
            reinterpret_cast< uintptr_t >( sctx.sp) - static_cast< uintptr_t >( sctx.size) );
    // create fast-context
    const std::size_t size = reinterpret_cast< uintptr_t >( stack_top) - reinterpret_cast< uintptr_t >( stack_bottom);

#if BOOST_CONTEXT_SHADOW_STACK
    std::size_t ss_size = size >> 5;
    // align shadow stack to 8 bytes.
	ss_size = (ss_size + 7) & ~7;
    // Todo: shadow stack occupies at least 4KB
    ss_size = (ss_size > 4096) ? size : 4096;
    // create shadow stack
    void *ss_base = (void *)syscall(__NR_map_shadow_stack, 0, ss_size, SHADOW_STACK_SET_TOKEN);
    BOOST_ASSERT(ss_base != -1);
    unsigned long ss_sp = (unsigned long)ss_base + ss_size;
    /* pass the shadow stack pointer to hoost_make_fcontext
	 i.e., link the new shadow stack with the new fcontext
	 TODO should be a better way? */
    *((unsigned long*)(reinterpret_cast< uintptr_t >( stack_top)- 8)) = ss_sp;
    /* Todo: place shadow stack info in 64byte gap */
    *((unsigned long*)(reinterpret_cast< uintptr_t >( storage)- 8)) = (unsigned long) ss_base;
    *((unsigned long*)(reinterpret_cast< uintptr_t >( storage)- 16)) = ss_size;
#endif
    const fcontext_t fctx = hoost_make_fcontext( stack_top, size, & fiber_entry< Record >);
    BOOST_ASSERT( nullptr != fctx);
    // transfer control structure to context-stack
    return hoost_jump_fcontext( fctx, record).fctx;
}

template< typename Record, typename StackAlloc, typename Fn >
fcontext_t create_fiber2( preallocated palloc, StackAlloc && salloc, Fn && fn) {
    // reserve space for control structure
    void * storage = reinterpret_cast< void * >(
            ( reinterpret_cast< uintptr_t >( palloc.sp) - static_cast< uintptr_t >( sizeof( Record) ) )
            & ~ static_cast< uintptr_t >( 0xff) );
    // placment new for control structure on context-stack
    Record * record = new ( storage) Record{
            palloc.sctx, std::forward< StackAlloc >( salloc), std::forward< Fn >( fn) };
    // 64byte gab between control structure and stack top
    void * stack_top = reinterpret_cast< void * >(
            reinterpret_cast< uintptr_t >( storage) - static_cast< uintptr_t >( 64) );
    void * stack_bottom = reinterpret_cast< void * >(
            reinterpret_cast< uintptr_t >( palloc.sctx.sp) - static_cast< uintptr_t >( palloc.sctx.size) );
    // create fast-context
    const std::size_t size = reinterpret_cast< uintptr_t >( stack_top) - reinterpret_cast< uintptr_t >( stack_bottom);

#if BOOST_CONTEXT_SHADOW_STACK
    std::size_t ss_size = size >> 5;
    // align shadow stack to 8 bytes.
	ss_size = (ss_size + 7) & ~7;
    // Todo: shadow stack occupies at least 4KB
    ss_size = (ss_size > 4096) ? size : 4096;
    // create shadow stack
    void *ss_base = (void *)syscall(__NR_map_shadow_stack, 0, ss_size, SHADOW_STACK_SET_TOKEN);
    BOOST_ASSERT(ss_base != -1);
    unsigned long ss_sp = (unsigned long)ss_base + ss_size;
    /* pass the shadow stack pointer to hoost_make_fcontext
	 i.e., link the new shadow stack with the new fcontext
	 TODO should be a better way? */
    *((unsigned long*)(reinterpret_cast< uintptr_t >( stack_top)- 8)) = ss_sp;
    /* Todo: place shadow stack info in 64byte gap */
    *((unsigned long*)(reinterpret_cast< uintptr_t >( storage)- 8)) = (unsigned long) ss_base;
    *((unsigned long*)(reinterpret_cast< uintptr_t >( storage)- 16)) = ss_size;
#endif
    const fcontext_t fctx = hoost_make_fcontext( stack_top, size, & fiber_entry< Record >);
    BOOST_ASSERT( nullptr != fctx);
    // transfer control structure to context-stack
    return hoost_jump_fcontext( fctx, record).fctx;
}

}

class fiber {
private:
    template< typename Ctx, typename StackAlloc, typename Fn >
    friend class detail::fiber_record;

    template< typename Ctx, typename Fn >
    friend detail::transfer_t
    detail::fiber_ontop( detail::transfer_t);

    detail::fcontext_t  fctx_{ nullptr };

    fiber( detail::fcontext_t fctx) noexcept :
        fctx_{ fctx } {
    }

public:
    fiber() noexcept = default;

    template< typename Fn, typename = detail::disable_overload< fiber, Fn > >
    fiber( Fn && fn) :
        fiber{ std::allocator_arg, fixedsize_stack(), std::forward< Fn >( fn) } {
    }

    template< typename StackAlloc, typename Fn >
    fiber( std::allocator_arg_t, StackAlloc && salloc, Fn && fn) :
        fctx_{ detail::create_fiber1< detail::fiber_record< fiber, StackAlloc, Fn > >(
                std::forward< StackAlloc >( salloc), std::forward< Fn >( fn) ) } {
    }

    template< typename StackAlloc, typename Fn >
    fiber( std::allocator_arg_t, preallocated palloc, StackAlloc && salloc, Fn && fn) :
        fctx_{ detail::create_fiber2< detail::fiber_record< fiber, StackAlloc, Fn > >(
                palloc, std::forward< StackAlloc >( salloc), std::forward< Fn >( fn) ) } {
    }

#if defined(BOOST_USE_SEGMENTED_STACKS)
    template< typename Fn >
    fiber( std::allocator_arg_t, segmented_stack, Fn &&);

    template< typename StackAlloc, typename Fn >
    fiber( std::allocator_arg_t, preallocated, segmented_stack, Fn &&);
#endif

    ~fiber() {
        if ( BOOST_UNLIKELY( nullptr != fctx_) ) {
            detail::hoost_ontop_fcontext(
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
                    detail::exchange( fctx_, nullptr),
#else
                    std::exchange( fctx_, nullptr),
#endif
                   nullptr,
                   detail::fiber_unwind);
        }
    }

    fiber( fiber && other) noexcept {
        swap( other);
    }

    fiber & operator=( fiber && other) noexcept {
        if ( BOOST_LIKELY( this != & other) ) {
            fiber tmp = std::move( other);
            swap( tmp);
        }
        return * this;
    }

    fiber( fiber const& other) noexcept = delete;
    fiber & operator=( fiber const& other) noexcept = delete;

    fiber resume() && {
        BOOST_ASSERT( nullptr != fctx_);
        return { detail::hoost_jump_fcontext(
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
                    detail::exchange( fctx_, nullptr),
#else
                    std::exchange( fctx_, nullptr),
#endif
                    nullptr).fctx };
    }

    template< typename Fn >
    fiber resume_with( Fn && fn) && {
        BOOST_ASSERT( nullptr != fctx_);
        auto p = std::forward< Fn >( fn);
        return { detail::hoost_ontop_fcontext(
#if defined(BOOST_NO_CXX14_STD_EXCHANGE)
                    detail::exchange( fctx_, nullptr),
#else
                    std::exchange( fctx_, nullptr),
#endif
                    & p,
                    detail::fiber_ontop< fiber, decltype(p) >).fctx };
    }

    explicit operator bool() const noexcept {
        return nullptr != fctx_;
    }

    bool operator!() const noexcept {
        return nullptr == fctx_;
    }

    bool operator<( fiber const& other) const noexcept {
        return fctx_ < other.fctx_;
    }

    #if !defined(BOOST_EMBTC)

    template< typename charT, class traitsT >
    friend std::basic_ostream< charT, traitsT > &
    operator<<( std::basic_ostream< charT, traitsT > & os, fiber const& other) {
        if ( nullptr != other.fctx_) {
            return os << other.fctx_;
        } else {
            return os << "{not-a-context}";
        }
    }

    #else

    template< typename charT, class traitsT >
    friend std::basic_ostream< charT, traitsT > &
    operator<<( std::basic_ostream< charT, traitsT > & os, fiber const& other);

    #endif

    void swap( fiber & other) noexcept {
        std::swap( fctx_, other.fctx_);
    }
};

#if defined(BOOST_EMBTC)

    template< typename charT, class traitsT >
    inline std::basic_ostream< charT, traitsT > &
    operator<<( std::basic_ostream< charT, traitsT > & os, fiber const& other) {
        if ( nullptr != other.fctx_) {
            return os << other.fctx_;
        } else {
            return os << "{not-a-context}";
        }
    }

#endif

inline
void swap( fiber & l, fiber & r) noexcept {
    l.swap( r);
}

typedef fiber fiber_context;

}}

#if defined(BOOST_MSVC)
# pragma warning(pop)
#endif

#ifdef BOOST_HAS_ABI_HEADERS
# include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_CONTEXT_FIBER_H
