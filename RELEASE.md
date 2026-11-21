# Release History

*****************

## Release ONDEWO CSI C++ Client 5.9.2

### New Features

* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) Keycloak authentication with offline-token start-up, the same behaviour as ondewo-csi-client-python 5.9.2: `refresh_token` in `ClientConfig` (`keycloak_url`, `realm`, `client_id`, `username`, `password`, `token_expiration_in_s`, `verify_ssl` and `refresh_token`, appended last so positional initialisation keeps its meaning) hands the client a Keycloak *offline* refresh token instead of a password. The client then starts with one `grant_type=refresh_token` exchange and never sends a password grant, not even as a fallback; a refused token is never answered with a password login. Keycloak's brute-force detection counts failed LOGINS, so N processes booting at once with one technical user are a burst the realm penalises, while N refresh grants of one offline token are not.
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) A Keycloak configuration needs a password OR a refresh token, and the Keycloak URL, realm and client id are all-or-nothing; when both credentials are set the token wins. The password and the token are redacted from every rendering of the configuration and take part (hashed) in the shared-provider registry key, so two token-only configurations of different tenants never share a provider. The `token_expiration_in_s` window is armed on both start-up paths.
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) The header-only `<ondewo/client/keycloak.h>` adds `KeycloakTokenProvider`, a weak, credential-keyed (SHA-256) provider registry, a gRPC `MetadataCredentialsPlugin`, `CreateAuthenticatedChannel()` and `AddKeycloakAuthorization()`; `<ondewo/client/keycloak_curl.h>` is the libcurl transport, in a header of its own so only its users link libcurl.

### Bug Fixes

* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) A failed token refresh never ends the background refresh: any failure of the token endpoint (a 502 from a proxy, a DNS failure, a restarting Keycloak, a dead offline session) is retried with one full-jitter backoff (5 s doubling up to 300 s, measured from the moment the failure is recorded) that the per-call path shares, and a success resets it. Concurrent callers during an outage share one in-flight refresh, so an outage costs one token request, not one per call.
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) A token that is still valid keeps being served when its refresh fails inside the 30 s expiry leeway; only an EXPIRED token raises `KeycloakAuthenticationError`, whatever failed underneath: a transport error, a non-2xx status, a body that is not JSON or not a JSON object, a missing `access_token` or a non-numeric `expires_in`, each with the original error as its cause. A malformed response changes no provider state, and a password login whose response carries no `refresh_token` keeps the previous one, as the python SDK does.

### Improvements

* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) One Keycloak contract across every ONDEWO client SDK, with the python SDK as the reference: the same constants, log lines and error texts. The token request is bounded by 10 s, quoted Keycloak error bodies are cut at 500 characters, a token response is reported by its field names only (never its values), and each failure logs one WARNING with the attempt number and the retry delay, the first success afterwards one INFO; no credential appears in either line.
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) Tests implement the cross-SDK test catalogue T01-T62 plus edge cases, on a fake token endpoint and clock, with 100 % coverage of the hand-written authentication code.
* Tracking API Version [5.9.0](https://github.com/ondewo/ondewo-csi-api/releases/tag/5.9.0) ( [Documentation](https://ondewo.github.io/ondewo-csi-api/) ); the generated code is unchanged against 5.9.0.

*****************

## Release ONDEWO CSI C++ Client 5.9.0

### Improvements

* Tracking API Version [5.9.0](https://github.com/ondewo/ondewo-csi-api/releases/tag/5.9.0) ( [Documentation](https://ondewo.github.io/ondewo-csi-api/) )
* Generated with ondewo-proto-compiler 5.17.0 (protoc 35.1); new runtime floors: unchanged, Debian trixie protobuf 3.21.12 / grpc 1.51.1.

*****************

## Release ONDEWO CSI C++ Client 5.7.0

### Improvements

* Tracking API Version [5.7.0](https://github.com/ondewo/ondewo-csi-api/releases/tag/5.7.0) ( [Documentation](https://ondewo.github.io/ondewo-csi-api/) )

*****************

## Release ONDEWO CSI C++ Client 5.6.2

### Improvements

* Tracking API Version [5.6.2](https://github.com/ondewo/ondewo-csi-api/releases/tag/5.6.2) ( [Documentation](https://ondewo.github.io/ondewo-csi-api/) )

*****************

## Release ONDEWO CSI C++ Client 5.6.0

### New Features

* Tracks [ONDEWO CSI API 5.6.0](https://github.com/ondewo/ondewo-csi-api/releases/tag/5.6.0); the stubs in `api/`
  are regenerated from it. New API surface (summarised from the API release notes, purely additive and
  wire-compatible with 5.5.0):
  * `Conversations.SetCallMediaControl`: per-call operator media control pushed by ondewo-sip (in-container token
    only). The request `CallMediaControlLevel` carries the full effective level (`bot_muted`, `listening_paused`), a
    monotonic `generation` and a bounded `reason`; the response `SetCallMediaControlResponse` reports `applied`,
    `changed`, `stale`, `bot_playback_in_flight` and `refusal_reason`.
  * `ControlStreamResponse.media_control`: set only on media-control messages of `GetControlStream`, pushed on a
    level change and seeded on every connect. Such a message must be handled as media control and its echoed
    `control_status` must not be applied.
  * `SipTrigger.INVITE` is documented as not implemented; use the ondewo-vtsi `Calls.InviteToCall` RPC.
* The NLU, S2T and T2S APIs the CSI API imports are unchanged (NLU 7.1.0, S2T 7.5.0, T2S 6.6.0).

### Improvements

* The test suite covers the new surface: `tests/product_config.cc` expects `SetCallMediaControl` and records the
  5.6.0 counts (848 messages, 98 enums, 2550 scalar fields); `tests/test_typed_api.cc` round-trips
  `CallMediaControlLevel` and `ControlStreamResponse.media_control` and dispatches `SetCallMediaControl` against a
  dead endpoint.
* The pinned `ondewo-proto-compiler` submodule moves from 5.15.2 to 5.15.5 (its fixes concern the Python, Rust and
  Node.js images; the C++ generation is unchanged).
* Tracking API Version [5.6.0](https://github.com/ondewo/ondewo-csi-api/releases/tag/5.6.0) ( [Documentation](https://ondewo.github.io/ondewo-csi-api/) )

*****************

## Release ONDEWO CSI C++ Client 5.5.1

### New Features

* TLS / mutual TLS: new header-only channel helper `ondewo/client/channel.h`, installed beside the generated
  headers and on the exported `ondewo::ondewo_csi_client` target's include path. `ondewo::client::ClientConfig`
  (`host`, `port`, `grpc_cert`, `grpc_client_cert`, `grpc_client_key`, `use_secure_channel`) takes PEM **content**,
  not file paths. `CreateChannel`, `CreateChannelCredentials`, `ValidateConfig` and `DefaultChannelArguments()`
  report errors as `grpc::Status` `INVALID_ARGUMENT`, never as exceptions.
* An empty `grpc_cert` uses the platform trust store. Half a client identity (certificate without key or key
  without certificate) and a plaintext channel with a client identity are refused before gRPC is called. A
  plaintext channel logs a warning on `std::clog` naming `host:port`. No error message renders a PEM, a key or the
  config; `ToString()` / `operator<<` print the client key as `***REDACTED***`. Bare IPv6 hosts are bracketed;
  CRLF PEMs work.
* Default channel arguments match the ONDEWO Python SDKs: keepalive every 30 s during calls with a 20 s timeout,
  `grpc.http2.max_pings_without_data=2`, reconnect backoff capped at 5 s, 2^31-1 byte send/receive message limit.
  Documented gaps: gRPC 1.51 ignores `grpc.http2.ping_timeout_ms`, and no per-method retry policy is configured.
* README: new section "TLS, mutual TLS and certificates" (modes, loading a PEM from a file, test PKI, security
  notes, troubleshooting).

### Improvements

* `tests/test_tls.cc`: real handshakes against in-process servers with a PKI generated by the openssl CLI at test
  time - TLS, mutual TLS, server rejecting a missing or foreign client certificate, wrong CA, system trust store,
  non-PEM CA (error, not crash), CRLF PEMs, server-name override, `[::1]` - plus the refusal, warning, redaction
  and default-argument cases. `make coverage` now also holds `client/` to 100 % line coverage.
* `tests/test_release_notes.sh` (CTest) pins the release-notes slice, the heading spelling, the `*****` separators
  and non-empty notes for the current version.
* The pinned `ondewo-proto-compiler` submodule moves from 5.15.1 to 5.15.2 (its fixes concern the PHP, Rust and
  JavaScript images and the release automation; the C++ generation is unchanged).
* Tracking API Version [5.5.0](https://github.com/ondewo/ondewo-csi-api/releases/tag/5.5.0) ( [Documentation](https://ondewo.github.io/ondewo-csi-api/) )

*****************

## Release ONDEWO CSI C++ Client 5.5.0

### New Features

* Initial release of `ondewo-csi-client-cpp`, the C++ gRPC client library for the
  [ONDEWO CSI API](https://github.com/ondewo/ondewo-csi-api) (Conversational Speech Interface). The stubs are generated
  from the `ondewo-csi-api` submodule by the `ondewo-cpp-proto-compiler` image of
  [ondewo-proto-compiler](https://github.com/ondewo/ondewo-proto-compiler) 5.15.1, using protoc's
  built-in `--cpp_out` for the messages and `grpc_cpp_plugin` for the service stubs.
* Ships as a consumable CMake package: `find_package(ondewo_csi_client CONFIG REQUIRED)` plus
  `target_link_libraries(my_app PRIVATE ondewo::ondewo_csi_client)`. The package installs the generated
  headers under `include/ondewo_csi_client/`, the static archive
  `lib/libondewo_csi_client.a` and `lib/cmake/ondewo_csi_client/`, and re-finds `Protobuf` and
  `gRPC` on the consumer's machine through `find_dependency`.
* `public-api.h` is an umbrella header that includes every generated `*.pb.h` and `*.grpc.pb.h`, so a single
  `#include "public-api.h"` gives access to the whole API surface.
* The generated stub sources are committed to the repository. C++ has no package registry, so a git tag is the
  distribution channel: a plain `git clone` or a CMake `FetchContent_Declare` yields a buildable project with
  no Docker and no code generation step.
* `make build` reproduces the whole pipeline - submodule checkout at the pinned tags, compiler image build,
  code generation and a host-native CMake build - and `make test` verifies that every `.proto` produced a
  header and that the installed CMake package is consumable by a downstream project.

*****************
