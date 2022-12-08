#pragma once
/*!
 * \file
 * \brief Atomic operations on 32-bit integers.
 */

//! \brief Increment a number atomically.
//! \returns Old value of the number.
//! \remark Performs the following operation atomically:\n int old_value = *address; (*address)++; return old_value;
int atomic_increment(int* address);

//! \brief Decrement a number atomically.
//! \returns Old value of the number.
//! \remarks Performs the following operation atomically:\n int old_value = *address; (*address)--; return old_value;
int atomic_decrement(int* address);

//! \brief Compare two numbers atomically and assign if equal.
//! \returns Old value of the number.
//! \remarks Performs the following operation atomically:\n int old_value = *address; if (*address == compare) *address = exchange; return old_value;
int atomic_compare_and_exchange(int* dest, int compare, int exchange);

//! \brief Reads an integer from an address.
//! \remarks All writes that occurred before the last atomic_store to this address are flushed.
int atomic_load(int* address);

//! \brief Writes an integer.
//! \remarks Paired with an atomic_load, can guarantee ordering and visibility.
void atomic_store(int* address, int value);
