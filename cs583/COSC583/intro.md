% SSL and TLS Essentials
% Christopher Craig; Aliceann Talley; Mahendra Duwal Shrestha
% Wed Oct 28 2015

## Introduction
In the early days of the internet, HTTP was only a means to send and receive
web pages.  Primitive iterations of access control were implement in
[RFC2617](https://www.ietf.org/rfc/rfc2617.txt) as a way to password-protect
certain requests for web pages. Making these web pages inaccessible to anyone
without that password. While this is a successful way to prevent the sever from
servicing a web request from an unauthorized user, it does nothing to mitigate
man-in-the-middle (MITM) attacks. In addition, the client may be communicating
to an illegitimate service that is just masquerading as the original server. 
The early architects of the Internet saw a way to change this and ensure that
both client and server could prove their identities to each other. This 
method is known as the Secure Socket Layer (SSL) or Transport Layer Security (TLS).

SSL/TLS is the primary way secure connections are made for any HTTP
traffic.  It sits between the HTTP and TCP traffic to provide a secure
connection from clients to severs.  SSL uses PKI with several verified certificate authorities (CAs) 
to ensure the browser can accurately verify that you are speaking to the
correct server. The protocol uses asymmetric encryption to actually encrypt
the traffic sent over the wire. Before we can talk about the in-depth mechanics of 
SSL, it's best to discuss its history. 

