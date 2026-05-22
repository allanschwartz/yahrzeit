
## Embedded controller command vocabulary

The V3 Arduino controller implements a CLI command language documented in
`yahrzeit_v3.h`.

The PHP appliance historically emits only a small subset:

- `all off`
- `pixel on <row> <col> <panel>`
- `pixel off <row> <col> <panel>`
- `refresh`
- `save`

The modern appliance/watcher should continue to emit this simple command stream.
The compiled `yahr_conduit` helper is obsolete; the controller is reachable by
`nc` or telnet over TCP/IP.
