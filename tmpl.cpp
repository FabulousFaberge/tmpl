/* Small template library for basic
 * logic elements & circuits compile-time
 * simulation (2020)
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

template<typename A, typename B, typename S>
struct _MUX {
  /* MUX block */
  using result = typename _OR<typename _AND<A, typename _NOT<S>::result>::result,
                              typename _AND<B, S>::result>::result;
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

  /* MUX block */
  struct MUX {
    template<typename OP1, typename OP2, typename S>
    struct OP {
      template<int N>
      using _bit = typename _MUX<std::tuple_element_t<D - N - 1, OP1>,
                                 std::tuple_element_t<D - N - 1, OP2>, S>::result;

      template<int N>
      using _result = decltype(std::tuple_cat(std::declval<std::tuple<_bit<d>>>(),
                                              std::declval<typename _DIM<D, d - 1>::MUX::
                                              template OP<OP1, OP2, S>::
                                              template _result<d - 1>>()));

      using result = _result<d>;

      template<int N>
      using bit = typename std::tuple_element_t<N, result>;
     };
   };

  /* Convert block */
  template<typename T, int M = std::tuple_size<T>() - 1>
  struct CONV {
    template<int N>
    using _bit = typename std::conditional_t<N <= M, typename 
                          std::tuple_element_t<N <= M ? M - N : (D - N - 1) % M, T>, O>;

    template<int N>
    using _result = decltype(std::tuple_cat(std::declval<std::tuple<_bit<d>>>(),
                                            std::declval<typename _DIM<D, d - 1>::
                                            template CONV<T>::template _result<d - 1>>()));
    using result = _result<d>;
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

  /* MUX block */
  struct MUX {
    template<typename OP1, typename OP2, typename S>
    struct OP {
      template<int N>
      using _bit = typename _MUX<std::tuple_element_t<D - 1, OP1>,
                                 std::tuple_element_t<D - 1, OP2>, S>::result;

      template<int N>
      using _result = std::tuple<_bit<N>>;
    };
  };

  /* Convert block  */
  template<typename T, int M = std::tuple_size<T>() - 1>
  struct CONV {
    template<int N>
    using _bit = typename std::tuple_element_t<M, T>;

    template<int N>
    using _result = std::tuple<_bit<N>>;
  };
};

/* Main unit */
template<int D>
struct DIM : public _DIM<D> {

  /* ALU block */
  template<typename F>
  struct ALU {
    template<typename OP1, typename OP2>
    struct OP {
      /* MUX and NOT for second operand */
      using _not_op2 = typename _DIM<D>::NOT::template OP<OP2>::result;
      using _mux_op2 = typename _DIM<D>::MUX::template OP<OP2, _not_op2, std::tuple_element_t<0, F>>::result;

      /* Logical operations */
      using _or_ops  = typename _DIM<D>::OR::template OP<OP1, _mux_op2>::result;
      using _and_ops = typename _DIM<D>::AND::template OP<OP1, _mux_op2>::result;
      using _mux_log = typename _DIM<D>::MUX::template OP<_or_ops, _and_ops, std::tuple_element_t<1, F>>::result;

      /* Addition and substraction */
      using _sum_ops = typename _DIM<D>::ADD::template OP<OP1, _mux_op2, std::tuple_element_t<0, F>>::sum;
      using _mux_sum = typename _DIM<D>::MUX::template OP<_sum_ops, _sum_ops, std::tuple_element_t<1, F>>::result;      

      /* Final result */
      using result = typename  _DIM<D>::MUX::template OP<_mux_log, _mux_sum, std::tuple_element_t<2, F>>::result;
    }; 
  };
};

/* Alias DIM names for convinience */
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

/* ALU instruction set */
using OR  = std::tuple< O, O, O >;
using AND = std::tuple< O, I, O >;
using SUM = std::tuple< O, O, I >;
using SUB = std::tuple< I, O, I >;

/* Some operands */
using OP1 = WORD::CONV< std::tuple< O, O, I, O, I, I, O, I > >::result;
using OP2 = WORD::CONV< std::tuple< O, I, O, I, O, O, I, I > >::result;

/* Register values */
using R0  = WORD::ALU<SUM>::OP<OP1, OP2>::result;
using R1  = WORD::ALU<AND>::OP<OP1, OP2>::result;
 
int main() {
  cout << "Operands: " << endl << "OP1: ";
  print<OP1>::out();
  cout << endl << "OP2: ";
  print<OP2>::out();
  cout << endl << endl;;

  cout << "Results: " << endl << "SUM: "; 
  print<R0>::out();
  cout << endl << "AND: ";
  print<R1>::out();
  cout << endl;
}
