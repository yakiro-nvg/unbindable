[![License: BSD-2-Clause](https://img.shields.io/badge/License-BSD%202--Clause-orange.svg)](https://opensource.org/licenses/BSD-2-Clause)

# Typed Redux Interop

Type system is an essential tool for large scale project. As an individual developer for
indie or small project we may not see the benefit of it. However, when more people join
the party we need to start thinking about the following problems.

1. What is the current state shape?
2. Re-shape state or action → broken runtime.
3. Interoperability between programming languages.

That all could be solved with a strongly typed language which describes how state and
actions looks like. That is far easier to understand than looking at the source code and
also self documented. Actually, it could be used as a detail design document.

Now, `UBDL` is still under development.

# Language

![UBDL](docs/imgs/language.png?raw=true "Unbindable description language")

# Usages

C++ Store.
```shell
$ unbindable -s solar.ubdl -l cpp
solar.h written!
```

Redux typescript.
```shell
$ unbindable -s solar.ubdl -l redux-ts
solar.ts written!
```

# What works currently

- C++ and Redux typescript transpiler.
- Store, state and actions description.
- C++ store bridge, useful for debug utilities.

# Coming soon

- More languages support.
- Document generation.
- Tags for metadata.

# Store bridge

Used in COMPASS architecture for debug. Basically, we need that to touch the application
state and dispatch actions like a gentleman. My device is C++ based so we currently only
support this language. At the remote site we could easily use standard RESTful clients to
access these endpoints. The source code for bridges is in `bridge` folder.g

![Store bridge](docs/imgs/bridge.png?raw=true "COMPASS store bridge")
