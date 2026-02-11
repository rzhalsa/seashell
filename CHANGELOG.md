# CHANGELOG

### v0.5.0 - 2026-02-11
- Adds the ability for SHrimp to handle an arbitrary amount of pipes per command.
- Introduces a Pipeline struct which holds the entire pipeline of current commands to execute.
- Introduces a ParseCode enum for streamlined parsing error handling and code cleanliness.
- Removes the delayed command feature from SHrimp. It may be brought back in a later release, but currently it's causing more issues than it's worth.
- Removes obsolete code for manually piping between two commands.
- Adds text coloring to the shell prompt text, as well as bright red text for error messages.
- Various other small syntax changes across the codebase to accomodat
- Improves helper function documentation.

---
### v0.4.0 - 2026-02-04
- Adds the ability for SHrimp to handle multiple commands separated by semicolons.
- Introduces a Commands struct which holds all commands in a line.
- Updates the main shell loop to support multiple commands per line.
- Makefile now supports automated installation and uninstallation of SHrimp via `make install` and `make uninstall`.
- A simple Github Actions CI test pipeline has been set up for SHrimp.

**It should be noted that this release permanently changes the license of SHrimp from the MIT license to the GPLv3 license. All future releases of SHrimp following v0.4.0 will be licensed under the GPLv3 license.**

---
### v0.3.0 - 2026-01-23
- Majorly refactors shell to be modular, easily upgradable, and maintainable.
- Adds MIT license.
- Adds Makefile for streamlined compilation.
- Shell officially renamed from SeaShell to SHrimp.

---
### Pre-v0.3.0
Unfortunately, I do not posses detailed documentation on what changes where made when to SHrimp prior to release v0.3.0.

This is what I do know:
### 2025-10-25
- Adds doxygen comments to every helper function to better document and explain the shell.
### 2025-08-17
- Repo creation.
