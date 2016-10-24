# Encrypted Communication with OCB-AES and X.1035

Encrypted communication code example using OCB-AES for data encryption and X.1035 symmetric password-authenticated key exchange (PAK) protocol.

The summary of the implementation is given below, and a more detailed explanation of the code can be found in the PDF file (not yet here, needs some updates).

* Master initiates AES symmetric key generation through X.1035 (Authenticated Diffie-Hellman) algorithm. Slave responds back, and after several transactions key agreement is completed.

* Master transmits data blocks during one session using OCB mode which provides both authentication and encryption. Slave can check both integrity and decode accordingly.

* When time-lapse for session is reached and there is new data to be transmitted, master re-initiates key-agreement procedure, and new session is started.

## What you can find here?

This repository can be used to find example implementations for:

* OCB3 Mode of operation using AES1024 Symmetric Key Block Cipher
* X.1035 Password-authenticated key exchange (PAK) protocol
* Mersenne Twister pseudorandom number generator
* Montgomery Multiplication, Exponentiation, Inversion and Division
* Big Integer Arithmetics
