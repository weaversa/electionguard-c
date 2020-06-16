all:	obj/bignum.bc obj/sha256.bc obj/crypto.bc

obj/bignum.bc:	../../src/electionguard/bignum.c
	clang -emit-llvm ../../src/electionguard/bignum.c -I../../include/electionguard -I/usr/local/include -I../../src/electionguard -o obj/bignum.bc -c

obj/crypto.bc:	../../src/electionguard/crypto.c
	clang -emit-llvm ../../src/electionguard/crypto.c -I../../include/electionguard -I../../include -I/usr/local/include -I../../src/electionguard -o obj/crypto.bc -c

obj/sha256.bc:	../../src/electionguard/sha2-openbsd.c
	clang -g -emit-llvm ../../src/electionguard/sha2-openbsd.c -I../../include/electionguard -I/usr/local/include -I../../src/electionguard -o obj/sha256.bc -c

obj/eg.bc: obj/bignum.bc obj/sha256.bc obj/crypto.bc
	llvm-link obj/bignum.bc obj/sha256.bc obj/crypto.bc -o eg.bc

obj/test.bc:	test.c
	clang test.c -emit-llvm  -I/usr/local/include -o obj/test.bc -c

obj/ptr.bc:	ptr.c
	clang ptr.c -emit-llvm  -I/usr/local/include -o obj/ptr.bc -c

obj/cube.bc:	cube.c
	clang cube.c -emit-llvm  -I/usr/local/include -o obj/cube.bc -c

obj/sha256:	main.c ../../src/electionguard/sha2-openbsd.c
	clang main.c ../../src/electionguard/sha2-openbsd.c -I../../include/electionguard -I/usr/local/include -I../../src/electionguard -o obj/sha256

