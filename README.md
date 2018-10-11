# WinGadgetlib
Windows version for *libsnark's gadgetlib*
___

[Libsnark](https://github.com/scipr-lab/libsnark) is a framework for building **zk-SNARKS** - novel zero-knowledge proof protocols for general circuits, which are nothing else but deterministic functions with known in advance number of loops in any inner cycle. In order for libsnark to operate, such a circuit must be represented in a special form called **Rank1 Constraint System (R1CS)**, which requires all operations made by the program to be _"snarked"_ to a system of quadratic equations over finite Galois fields. 

The problem, which naturally arises on this way is the following: how one can transform arbitrary circuits into R1CS form. Libsnark itself is shipped with supportive libraries (called *gadgetlib1* and *gadgetlib2*) which provide users with all the instruments required for such a transformation. However these libraries are extremely low-level and anyone wishing to construct *R1CS* system even for simple circuits such as a bunch of addtions and multiplications has to struggle throught dozens of messy bit-level gadgets. Such handcrafting is tedious, exausting and error-prone. The approach we suggest in *WinGadgetlib* is to shift all the hard bit-manipulating work in library internals and give end users the possibility to represent their intensions in a simple readable form.
___

*WinGadgetlib* is shipped with an *embedded C++ DSL* for writing circuits. This is, for example, how one can write a gadget for [MIMC-Style](https://eprint.iacr.org/2016/492.pdf) hash function:

```C++
gadget MimcHash(const gadget& a_, const gadget& b_)
{
	static constexpr unsigned MIMC_ROUNDS = 57;
	
	//take at random;
	size_t const_elems[] = {
			69903, 40881, 76085, 19806, 59389, 72154, 8071, 71432, 86763, 68279, 9954, 20005,
			03373, 56459, 56376, 72855, 93480, 65167, 18166, 48738, 07064, 25708, 57661,
			91900, 17643, 98782, 49011, 11135, 5081, 26045, 23498, 43851, 63402, 6672, 39843,
			45133, 33604, 98922, 79523, 1803, 61469, 46699, 67078, 71485, 80378, 31110,
			15431, 46665, 19120, 47035, 96195, 43755, 34710, 4687, 34984, 17157, 70194 };
	gadget temp1, temp2, a = a_, b = b_;

	for (unsigned i = 0; i < MIMC_ROUNDS; i++)
	{
		temp2 = a;
		a = a + gadget(const_elems[i]);
		temp1 = a * a;
		a = a * temp1;
		a = a + b;
		b = temp2;
	}
	return a;	
}
```


Simple, right? This gadget will be automatically transformed to corresponding R1CS system by the library kernel. Apart from MIMC hashing, our testsuit contains other interesting examples, among which are:

1. SHA256 hash function gadget.
2. Battleship game initialization gadget. This one was submitted for ETH-Berlin hackathon ([Check here!](https://devpost.com/software/gameofsnarks_contracts)) and even won one of prizes.
3. Plasma single transaction gadget. This is a part of the logic gadget for our zk-SNARK governed plasma implementation. For further details see our [reference paper](https://github.com/matterinc/research/blob/master/zkSNARKs/zkSNARKgovernedPlasma/zkSNARKs%20governed%20Plasma%20by%20Matter%20Inc%2C%20informal%20spec.md).
4. Blackjack gadget - main gadget for zk-SNARK driven classical blackjack game.

All gadgets for afforementioned circuits are located in [*basic_gadgets.hpp*](../master/include/basic_gadgets.hpp)

___

Current version of *WinGadgetLib* is not production ready, it is more like a POC implementation of compact *EDSL* for circuit description.  There is a wide room for various optimizations, starting from simple ones such as delayed reduction in modulus operations and ending with more sophisticated and advanced such as multivariative polynomial elimination. Many concrete optimization techniques can be found in [xJsnark paper](http://www.cs.umd.edu/~akosba/papers/xjsnark.pdf)
___

## Installation remarks.

*WinGadgetLib* depends on *Boost* (for variant) and [*NTL*](https://www.shoup.net/ntl/) (for finite field arithmetic). CMake variables `NTL_LIBRARY` and `NTL_INCLUDE_DIR` should be set to directories containing *ntl.lib* and NTL *include* folder correspondenly.

