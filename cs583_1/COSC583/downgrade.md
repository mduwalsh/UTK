### Downgrade Attacks

By design, SSL/TLS wanted newer versions of the protocol to be backwards
compatible for older systems. As a result, an attacker can force a client to
use a weaker encryption standard than requested from the client. Unlike the
padding oracle attack discussed in the previous section, the attacker must be
able to intercept communication. This can be accomplished several ways but the
most common is to take advantage of the underlying protocol that is responsible
for address resolution.  This can be accomplished by poisoning the cache of the
victim's Domain Name Service (DNS) server so that it routes to the attacker's
server.  Alternatively, an attacker can poison the Address Resolution Protocol
table on the victim. Both prerequisite attacks allow the attacker to
to surreptitiously impersonate the target server.  

Protocol downgrade attacks are made possible by the assumption that an error or
terminated connection is a failed SSL/TLS failure. When the client sends
a `ClientHello` message to the server to initiate a secure connection, the
attacker terminates the connection while impersonating the server. The client,
viewing a failed SSL/TLS connection, will assume the legitimate server can not
support the original version of SSL and will send another `ClientHello` message
with using an older version of SSL/TLS. This attack is a security vulnerability
since older versions of SSL continue to supports weak cryptographic cyphers. 

