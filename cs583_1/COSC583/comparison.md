## Comparison
### SSL 2.0
* SSL 2.0 is flawed in many ways.
* Identical cryptographic keys are used for message authentication and encryption. This means assurance of authenticity is weakened to same degree as privacy of data.
* SSL 2.0 has weak MAC construction. It uses MD5 hash function that is vulnerable in length extension attacks.
* SSL 2.0 does not have any protection for the handshake, meaning a man-in-the-middle downgrade attack can go undetected.
* SSL 2.0 uses the TCP connection close to indicate the end of data. This makes it susceptible to truncation attacks. Attacker can form a TCP FIN without aware of recipient from illegitimate data message.
* SSL 2.0 uses a fixed domain certificate with a single service, which will not match with the latest features of virtual hosting for web servers. Therefore, most websites are left weakened from applying SSL.
* SSL 2.0 does not follow chain certificate and non-RSA algorithm. It uses only RSA key exchange.
SSL 2.0 is disabled by default, befinning with _Internet Explorer 7_, _Mozilla Firefox 2_, _Opera 9.5_ and _Safari_. 

### SSL 3.0
* SSL 3.0 improved a lot upon SSL 2.0 by adding SHA-1-based ciphers and support for certificate authenticaiton. However, it is weaker than TLS 1.0 from security stand point.
* It can protect against man-in-the-middle attack. The finish messages are authenticated  and keep a hash for all previous handshake messages including finish messages. If there is any difference between what each party thought happened and what actually happened, the whole transaction will fail.
* Closure alerts are used in SSL 3.0 to close the connection properly (either party can initiate closure). So truncation attacks are not possible in SSL 3.0.
* SSL 3.0 uses SHA-1 hashing algorithm which is more secure that MD5. It supports extra cipher suites.
* SSL 3.0 use general key exchange protocol like Diffie-Hellman, RSA and Fortezza.
* SSL 3.0 uses adhoc use of MAC. There are MAC constructions that have not been subject to analysis like earlier version of HMAC. It is better to use something that has withstood fair amount of analysis.

### TLS 1.0
* TLS 1.0 is very similar to SSL 3.0. But there are few improvements made in it.
* TLS defined almost twice as many alert descriptions than SSL 3.0. TLS improved in its procedures for notification of potential and actual security alerts.
* TLS used standard HMAC instead of adhoc MAC.
* CertificateVerify is simple and simply the handshake messages previously exchanged during the session unlike in SSL 3.0 where it consists of complex, two level has of handshake messages, master secrets and padding.
* For interoperability, an approach is well defined for fallback from TLS 1.0 to SSL 3.0.
