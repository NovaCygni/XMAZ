# XMAZ

XMAZ is a simple, quick, robust, p2p, recursive cryptosystem, which is Unix 32/64-bit open-source.

Some proprieties :
* simplicity/minimalism
* compatibility/portability
* independence/decentralisation
* rapidity
* non-malleabilty
* deniable encryption/authentication
* perfect/forward secrecy

The cryptosystem uses :
* **OTP** - for symmetric block encryption, which is *pratically* unbreakable
* **Whirplool** - it provides a recursive mechanism for synchronous key generation, using some additional algorithmics with XOR, to prevent chosen ciphertext attacks
* **RIPEMD-160** - *ensures* data integrity and forging protection
* **Flags** - transmission control and anomaly detection/prevention
* **Nonce** - to prevent known-plaintext attacks and their derivatives
* **ECDHE** - a *proposition* for key exchange

Nevertheless, there are some inconveniences like :
* 32 characters transfer at once
* if both IPv4 addresses are public, at least one has to be accessible directly (not behind NAT or with port forwarding)
* no identification on protocol/algorithmic level (but automatic due to key bound)
* trust for received public key
* communication database is in clear (should be encrypted)

On the other hand, the cryptosystem relies on : 
* UDP port 1307 - transport
* IPv4 WAN/LAN addresses - anonymous identification
* C libraries :
	- OpenSSL - crypto, hashes and generation
	- ncurses - interface

##Install & Launch

```bash
apt-get install libssl-dev libncurses-dev
cd $XMAZ_SOURCE_DIRECTORY
make
./XMAZ
```
Enter the IP, share the keys and communicate.

###Notes

In case of popularity, some more technical details are to be revealed.
If you are behind NAT/PAT, you can try [pwnat](http://samy.pl/pwnat).
This is all experimental and further modifications/improvements are desirable.

> "One must acknowledge with cryptography no amount of violence will ever solve a math problem."
Jacob Appelbaum, *Cypherpunks: Freedom and the Future of the Internet*

