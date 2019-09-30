// EPOS ARM Cortex-A AES Mediator Declarations

#ifndef __cortex_a_aes_h
#define __cortex_a_aes_h

#include <machine/aes.h>
#include <utility/aes.h>

__BEGIN_SYS

// TODO: this is just a place holder. Replace with Cortex-A AES!
template<unsigned int KEY_SIZE>
class _AES: private AES_Common, public _UTIL::_AES<KEY_SIZE> {};

__END_SYS

#endif
