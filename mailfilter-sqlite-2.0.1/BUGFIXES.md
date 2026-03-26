# BUGFIXES.md

## SSL Communication Fix (socket.cc)

### Affected Version
mailfilter 0.8.9

### Problem

An issue in the socket read loop could cause instability during SSL communication
with certain mail servers (notably t-online, gmail).

The termination condition did not properly validate buffer boundaries, which could
lead to incorrect access patterns and potential crashes.

### Original Code:

	  while (bytes > 0
         || (read_header
             // The following finishes a POP3 server reply.
             && ( (input[input.length () - 1] != '\n'
                   || input[input.length () - 2] != '\r'
                   || input[input.length () - 3] != '.')
                  // The following finishes an IMAP server reply.
                  && input.find (")\r\na OK FETCH completed\r\n")
                                                == string :: npos )
             ));

The original implementation relied on unsafe index access without ensuring
minimum buffer length.

### Fix begin:

		while (bytes > 0 ||
                 (read_header &&
                  ! (input.length() >= 3 &&
                     input[input.length() - 3] == '.' &&
                     input[input.length() - 2] == '\r' &&
                     input[input.length() - 1] == '\n') &&
                  input.find(")\r\na OK FETCH completed\r\n") == string::npos
                 )); 
### Ende fix

The loop condition was rewritten to:

- ensure safe boundary checks
- validate minimum buffer length before access
- correctly detect POP3 and IMAP termination sequences

### Result

- stable SSL communication
- no crashes observed in test environments
- confirmed compatibility with affected servers

### Notes

This fix does not alter protocol logic, only improves robustness and safety.
