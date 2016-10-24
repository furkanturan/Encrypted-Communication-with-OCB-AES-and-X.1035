
hellomake: main.c
	gcc -o exp main.c Montgomery.c MGF.c MersenneTwister.c sha.c sha3.c x1035.c BigInteger.c OCB.c aes.c

clean:
	rm -f $(OBJ) exp