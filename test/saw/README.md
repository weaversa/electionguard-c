# Introduction

This note documents the use of SAW (software analysis workbench) to
verify several correctness properties of an existing SHA256 hash
implementation.  The chosen implementation is sha2-openbsd.c as
shipped with Microsoft's ElectionGuard-c software development
kit. ElectionGuard is both a mathematical algorithm and and
implementation of a system designed to create end to end verifiable
elections. The algorithm is designed in such a manner that any error
or bug in the voting process can be detected after the fact through an
independent election verification process. While this reduces the need
to show that the implementation of the voting process is correct, it
is still highly desirable to do so. If the software that generates the
voting artifacts is shown to be buggy at the conclusion of the
election, public confidence in the system would be dimished. In
addition, there still remains the need to show that the independent
post-election verifier is itself correct. Ideally, it would be
desirable to have high confidence in the correctness of both pieces of
software prior to holding an election. As a step towards that goal, this paper
verifies the SHA256 hash function which is a key component of the ElectionGuard algorithm.

Traditional software development practices such as code reviews and
testing help install confidence in the correctness of the underlying
implementation. These processes however do not formally prove any
properties of the implementation, there may be untested inputs that
cause the program to fail. Ideally, one would like to prove statements
of the form: 'program X computes value Y for all input values
Z'. Formal program verification is the process of answering such
questions by applying formal methods of mathematics.

As a first step in program verification one must define what it means
for a program to be correct.  This is typically done by specifying
correctness properties in a language that can be formally reasoned
about. Given this specification, one then attempts to prove the
correctness properties given a set of starting axioms and rules of
logic. A number of semi-automated systems exist to aid in this
process. In this note we consider the use of SAW, an open source program verification system from Galois Incorporated.

* SAW overview

While SAW can be used as a standalone tool, it is often used in
conjunction with the Cryptol language. Cryptol is a domain specific
language with features to make it easy to specify cryptographic
algorithms. Cryptol is Turing complete and Galois ships an open source
interpreter. In addition to to supporting common programming concepts
such as functions and modules, Cryptol allows one to define
*properties*. For instance, one can define the properties of the form:

```
  f x == g x

or

  f x > 0

for all 32-bit integers x.
```

By default, the Cryptol interpreter ignores these properties, but by
processing the Cryptol program with the SAW tool, one can formally
prove that the properties hold (or demonstrate a counter example).

SAW is an example of what is commonly referred to as a proof
assistant. A proof assistant is a semi-automated tool for proving
logical propositions. These tools provide the analyst with an
assortment of *tactics* to go from say the definition of *f x* to
showing that *f x > 0*.  SAW implements a powerful tactic based on
SMT/SAT solvers. This tactic first converts the property to a first
order boolean expression and then passes that expression to an
SMT solver. In theory, this tactic will always succeed. In
practice, SAT is an NP complete problem. While state of the art SAT
solvers have dramatically increased the size of problem that can be
solved, they all use heuristics that will exhibit exponential behavior
on a wide range of problems.

In the event that the SMT tactic fails to complete in an acceptable
amount of time, SAW provides a number of other tactics. For the most
part, these tactics allow one to *manually* specify rewrite
rules.  SAW then uses these rules to rewrite the property to one that
can be dispatched by the SMT tactic. For this approach to be sound,
one must also use SAW to show that the rewrite rule is valid. SAW
refers to these verified rewrite rules as theorems. It falls to
the analyst to develop a set of theorems that will allow the proof to
go through.


To this point, we have descibed SAW as a means to prove properties of
Cryptol programs. But, in the case of SHA256 we want to prove
properties of a C program. SAW supports this as well. To do so, it
first symbolically executes the C program. The result is a first order
boolean expression describing the computation that the C program
performed. This expression can then be checked for equivalence to a
Cryptol first order formula in the same manner that two Cryptol
expressions are checked. In a similar manner, two different C
implementations of the same function could also be checked.  SAW can
also symbolically execute Java code.  In all three cases, Cryptol, C
and Java are converted to the same underlying expression syntax
refered to as SAWCore.


Converting C or Java code to a first order boolean expression places
limits on the types of code that can be analyzed. One of the biggest
limitations are loops. All loops must bounded and, given the high
computation cost of symbolic exectution, the bounds must be relatively
small. In practice, SAW proofs are often parameterized by a fixed loop
count. As we will see in the SHA256 case, this restricts the proof to
a set of pre-chosen message lengths.


* The SHA256 Specification

As part of the ElectionGuard effort, Microsoft ships Cryptol specificaitons for both the ElectionGuard algorithm and the SHA256 hash function. In the case of SHA256, two algorithms are specified. The first, **SHA256** attempts to be a direct transliteration of the FIPS specification. At this point one might be tempted to ask: "Is the specification correct?". For this question, refer to reference [todo].

The second algorithm, **SHA256_imp** attempts to more closely match a
common implementation of SHA256.  SHA256 is typically implemented
using three functions, one to initialize the hash state, one to update
the hash state with the hash of message bytes, and one to finalize the
hash by applying the correct padding and generating the final digest.
One would like to verify that this second algorthm generates the same
output as the 'correct' one for all message lengths and contents.

One specifies this property as:

```
import "SHA256.cry"
p = {{\(msg:[1024][8]) -> SHA256 msg == SHA256_imp msg}};
```

At which point one could use an SMT tatic to complete the proof:

```
sha256_proof <- prove_print z3 p;
```

A couple of things to note. In the definition of **p**, one must
specify the length of the message (in this case 1024). While one can
recursively define functions in Cryptol, there is no direct support
for proving properties using such functions. Instead, Cryptol
functions are typically defined in terms of fixed length sequences.

The **z3** in the **prove_print** specifies the SMT solver to use. SAW
currently supports a number of SMT solvers: abc, boolector, cvc4,
mathsat, yices and z3. As we will see, these solvers have wildly
different performance characteristics which can vary from problem to
problem.

Using a length of 256 bytes, the **z3** solver did not finish the
proof within a 10 minute timeout. We then tried a tange of other
solvers. The results are given in Table 1.

Note that there is a fairly wide performance difference between the
different solvers. In addition, even the two best solvers, yices and
boolector, start encountering difficulites around 4096
bytes. Given that ElectionGuard computes SHA256 on inputs up to approximately
24k bytes, a different approach to proving the property must be taken

Functions in which each output bit depends on a complex function of
input bits often cause SAT/SMT solvers to exhibit poor
performance. Unfortunately, SHA256 is *designed* to have each output
bit be a non-linear function of each input bit, thus presenting a
scaling challenge to the solver. To reduce this challenge, SAW in
conjunction with certain solvers implements support for *uninterpreted
functions*. Uninterpreted functions allows the solver to ignore the
semantics of specifed functions that appear in the property to be
proved. The only thing the solver assumes about an uninterpreted
function is that it is *consistent*, i.e., give the same inputs it
returns the same outputs.

As an exmple consider the following expression:

```
x + SHA256Block(x) == SHA256Block(y) + y
```

Normally, the solver would expand the definition of SHA256Block before
attempting to prove the expression. Treating SHA256Block as an
uninterpreted function, if the solver can prove that x == y, then the
expression can be reduced to proving x + V == x + V where V == V. This
is a much easier expression for the solver to solve.

Note that without uninterpreted functions, a solver will (given enough
resources) return either SAT or UNSAT. With uninterpreted functions,
the solver can return a third options corresponding roughly to
unknown. For instance consider the following expression:


```
f(x, y) == f(y, x)
```

If the solver can not prove that x == y, then the solver will not be
able to prove anything about this expression since it does not know
whether **f** is commutative.

For the problem at hand, we can tell SAW to treat **SHA256Block** as
an uninterpreted function. It is used in both the definition of
**SHA256** and **SHA256_imp**. It is also the source of the non-linear
mixing. Specifing one or more uninterpreted functions is
straightforward. For example:

```
sha256_proof <- prove_print (unint_z3 ["SHA256Block"]) p;
```

treats **SHA256Block** as an unintrepreted function and uses the
version of Z3 that support uninterpreted functions as the solver. SAW
supports three different uninterpreted solvers: cvc4, yices and
z3. Table 2 give the time to solve for each of these solvers across a
range of input lengths. Only uninterpreted yices is capable of proving
the property for a 24638 long input - the length needed by
ElectionGuard. It should also be noted that yices required 16G of
memory to complete the proof.



limitations of the proof
  We don't say anything about init -> update* -> finalize

  verify that init -> update -> final is correct


* connecting to the C code

   * what you would hope for

   * show whats needed to verify a C program that calls:

       init -> update -> final

   * solvers fail (of course)

* uninterpreted functions to the rescue

   * model the SHA256Transform C function in terms of SHA256Block from the spec

   * the proof then goes through

   * but is our model correct?

* verifying SHA256Transform

   * with code modifications

   * without code modifications

* putting it all together

   * total lines of code

   * timing for select input sizes and solvers

Other approaches:

   F*

   Coq/VST

Summary:

   Verification was relatively straightforward

   Lots of fiddly bits to get correct but nothing fundamentally challenging

   Would be nice to have a better method of associating uninterpreted functions
   to snippets of executable code

   For the orignal purpose of EG verification, the fixed message size
   limitation is not an issue
   