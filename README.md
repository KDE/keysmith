# OTP client

It uses the [oath-toolkit](https://www.nongnu.org/oath-toolkit/) provided library liboath to generate the 2FA codes, both TOTP and HOTP based. Currently it is largely untested. From initial rough testing it seems that auto-refreshing of code is not working. Also button to refresh token for HOTP is also dummy at moment.

Some todo items include,

 - Verify the generated oath code is correct
 - Make refreshing token work
 - QR code scanning
 - Backup and Restore of accounts
 - Clipboard support to automatically copy code.
 - Encrypted storage of the secret token

This code is largely based on the [authenticator-ng](https://github.com/dobey/authenticator-ng) application by the Rodney Dawes and Michael Zanetti for the Ubuntu Touch.
