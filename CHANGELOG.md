# CHANGELOG
### v0.5.0 - 2026-02-10
- Adds the ability for SHrimp to handle an arbitrary amount of pipes per command.
- Removes the delayed command feature from SHrimp. It may be brought back in a later release, but currently it's causing more issues than it's worth.
- Adds text coloring to the shell prompt text, as well as bright red text for error messages.
---
### v0.4.0 - 2026-02-04
- Adds the ability for SHrimp to handle multiple commands separated by semicolons.
- Introduces a Commands struct which holds all commands in a line.
- Updates the main shell loop to support multiple commands per line.
- Maintains support for piping and redirection
- The SHrimp Makefile has also been expanded and now supports automated installation and uninstallation via make install and make uninstall.
- A simple Github Actions CI pipeline has also been set up for SHrimp.

**It should be noted that this release permanently changes the license of SHrimp from the MIT license to the GPLv3 license. All future releases of SHrimp after v0.4.0 will be licensed under the GPLv3 license**
