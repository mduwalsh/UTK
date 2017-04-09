### Poodle Attack
POODLE (CVE-2014-3566) is a novel attack against TLS and SSLv3. The acronym
stands for Padding Oracle On Downgraded Legacy Encryption and it combines two
techniques we've seen before against SSL/TLS: Padding Oracle Attacks and
Downgrade attacks. The attack is as follows: The attacker must first be able to
intercept the client to impersonate the server. The eavesdropper must force the
client to downgrade their connection. As with typical downgrade attacks, this
can be accomplished by resetting any SSL/TLS connection sent from the client.
Doing so forces the client to attempt to reconnect using an older version.
After the client successfully downgrades the SSL connection , the attacker can
then capture ciphertexts and use repeated SSL connections to perform a Padding
Oracle Attack on the weaker cipher. 

Discovered in 2014, this was comparable to Heartbleed and Shellshock in the
number of systems affected. The major cause for concern was the number of 
servers actively using SSlv3 for secure communication and had not
not yet upgraded. In addition, it was discovered that implementation errors
made specific versions of TLS vulnerable to this same style of attack
(CVE-2014-8730).  Now, the majority of up-to-date browsers are not vulnerable
to this attack. 
