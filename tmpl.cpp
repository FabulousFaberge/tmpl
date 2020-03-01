/* Small template library for basic
 * logic elements & circuits compile-time
 * simulation (2020)
 * Author: fabulous.faberge@yandex.ru
 */

#include <iostream>
#include <type_traits>
#include <tuple>

using std::cout;
using std::endl;

struct I {
  /* HIGH, TRUE, ONE */
};

struct O {
  /* LOW, FALSE, ZERO */
};

template<typename A, typename B>
struct _NAND {
  /* NAND gate */
  using result = std::conditional_t<(std::is_same<A, B>::value && 
                                     std::is_same<A, I>::value), O, I>;  
};

template<typename A, typename B = A>
struct _NOT {
  /* NOT gate */
  using result = typename _NAND<A, A>::result;
};

template<typename A, typename B>
struct _AND {
  /* AND gate */
  using result = typename _NOT<typename _NAND<A, B>::result>::result;
};

template<typename A, typename B>
struct _OR {
  /* OR gate */
  using result = typename _NAND<typename _NOT<A>::result, typename _NOT<B>::result>::result;
};

template<typename A, typename B>
struct _NOR {
  /* NOR gate */
  using result = typename _NOT<typename _OR<A, B>::result>::result;
};

template<typename A, typename B>
struct _XOR {
  /* XOR gate */
  using result = typename _NAND<typename _NAND<typename _NAND<A, B>::result, A>::result,
                                typename _NAND<typename _NAND<A, B>::result, B>::result>::result;
};

template<typename A, typename B>
struct _HALFADDER {
  /* HALFADDER block */
  using sum = typename _XOR<A, B>::result;
  using carry = typename _AND<A, B>::result;
};

template<typename A, typename B, typename C = O>
struct _FULLADDER {
  /* FULLADDER block */
  using sum = typename _HALFADDER<typename _HALFADDER<A, B>::sum, C>::sum;
  using carry = typename _OR<typename _HALFADDER<A,B>::carry, typename _HALFADDER<
                             typename _HALFADDER<A, B>::sum, C>::carry>::result;
};

/* Implementation class */
template<int D, int d = D - 1>
struct _DIM {
  
  /* Basic logic gates */
  template<template <typename, typename> class T0>
  struct EXP {
    template<typename OP1, typename OP2 = OP1>
    struct OP {
      template<int N>
      using _bit = typename T0<std::tuple_element_t<D - N - 1, OP1>, 
                               std::tuple_element_t<D - N - 1, OP2>>::result;

      template<int N>
      using _result = decltype(std::tuple_cat(std::declval<std::tuple<_bit<d>>>(), 
                                              std::declval<typename _DIM<D, d - 1>::
                                                           template EXP<T0>::
                                                           template OP<OP1, OP2>::
                                                           template _result<d - 1>>()));

      using result = _result<d>;

      template<int N>
      using bit = typename std::tuple_element_t<N, result>;
    };
  };
  
  /* Adder block */
  struct ADD {
    template<typename OP1, typename OP2, typename C = O>
    struct OP {
      template<int N>
      using _bit_sum = typename _FULLADDER<std::tuple_element_t<D - N - 1, OP1>,
                                           std::tuple_element_t<D - N - 1, OP2>,
                                           typename _DIM<D, N - 1>::ADD::template 
                                           OP<OP1, OP2, C>::template _bit_carry<N - 1>>::sum;

      template<int N>
      using _bit_carry = typename _FULLADDER<std::tuple_element_t<D - N - 1, OP1>,
                                             std::tuple_element_t<D - N - 1, OP2>,
                                             typename _DIM<D, N - 1>::ADD::template
                                             OP<OP1, OP2, C>::template _bit_carry<N - 1>>::carry;

      template<int N>
      using _sum = decltype(std::tuple_cat(std::declval<std::tuple<_bit_sum<d>>>(),
                                           std::declval<typename _DIM<D, d - 1>::ADD::
                                           template OP<OP1, OP2, C>::template _sum<d - 1>>()));

      using sum = _sum<d>;
      using carry = _bit_carry<d>;

      template<int N>
      using bit = typename std::tuple_element_t<N, sum>;
    };
  };
  
  /* Alias names for convinience */
  using NAND = EXP<_NAND>;
  using NOT  = EXP<_NOT>;
  using AND  = EXP<_AND>;
  using OR   = EXP<_OR>;
  using XOR  = EXP<_XOR>;
};

/* Exit condition for recursion */
template<int D>
struct _DIM<D, 0> {
  
  /* Basic logic gates */
  template<template <typename, typename> class T0>
  struct EXP {
    template<typename OP1, typename OP2 = OP1>
    struct OP {
      template<int N>
      using _bit = typename T0<std::tuple_element_t<D - N - 1, OP1>, 
                               std::tuple_element_t<D - N - 1, OP2>>::result;

      template<int N>
      using _result = std::tuple<_bit<N>>;

    };
  };

  /* Adder block */
  struct ADD {
    template<typename OP1, typename OP2, typename C = O>
    struct OP {
      template<int N>
      using _bit_sum = typename _FULLADDER<std::tuple_element_t<D - 1, OP1>,
                                           std::tuple_element_t<D - 1, OP2>, C>::sum;
      template<int N>
      using _bit_carry = typename _FULLADDER<std::tuple_element_t<D - 1, OP1>,
                                             std::tuple_element_t<D - 1, OP2>, C>::carry;

      template<int N>
      using _sum = std::tuple<_bit_sum<N>>;

      template<int N>
      using _carry = std::tuple<_bit_carry<N>>;
    };
  };
};

/* Main class */
template<int D>
struct DIM : public _DIM<D> {

};

/* Alias names for convinience */
using BIT   = DIM<1>;
using BYTE  = DIM<8>;
using WORD  = DIM<16>;
using DWORD = DIM<32>;
using QWORD = DIM<64>;

/* Utility 'print' function */
template<typename T>
void _print();

template<>
void _print<I>() {
  cout << "I ";
}

template<>
void _print<O>() {
  cout << "O ";
}

template<typename T, int N = std::tuple_size<T>() - 1>
struct print {
  static void out() {
    print<T, N - 1>::out();
    _print<std::tuple_element_t<N, T>>();
  }
};

template<typename T>
struct print<T, 0> {
  static void out() {
    _print<std::tuple_element_t<0, T>>();
  }
};

using OP1 = std::tuple< O, O, I, O, I, I, O, I >;
using OP2 = std::tuple< O, I, O, I, O, O, I, I >;
using OP3 = std::tuple< O, O, O, O, O, O, O, I >;

using R1  = BYTE::ADD::OP<OP1, OP2>::sum;
using R2  = BYTE::AND::OP<OP1, OP3>::result;

int main() {
  print<R1>::out();
  cout << endl;
  print<R2>::out();
  cout << endl;
}
