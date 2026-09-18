# Threat model

## Assets

- correct firmware/source identity;
- device configuration and recoverability;
- physical LED/power safety assumptions;
- WLED network and OTA surface;
- custom effect correctness;
- public/private boundary between source and a real home network.

## Trust boundaries and threats

### Upstream supply chain

Lavalamp depends on pinned WLED source and build tooling. An upstream version change, dependency compromise, or stale patch can silently alter behavior. Pin versions, review patch application, and validate generated source before considering deployment.

### Wrong-device or wrong-artifact deployment

An otherwise valid firmware image can be unsafe if applied to the wrong hardware or configuration. Device identity must not be inferred from IP alone. Live deployment requires an exact device and artifact binding.

### OTA integrity and recovery

A successful HTTP upload or process exit is not proof of a healthy update. Deployment needs backup, rollback, boot/API verification, matrix/LED verification, and confirmation that expected custom effects remain available.

### Network exposure

WLED services should remain local by default. Repository changes must not silently introduce remote telemetry, external control endpoints, credentials, or broader network exposure.

### Physical assumptions

Brightness, current limits, pin mapping, matrix orientation, and LED count affect real hardware. Public defaults and documentation must avoid implying that one electrical setup is safe for every build.

### False-positive visual verification

A build can pass while a physical cylinder has incorrect seam mapping, orientation, palette, flicker, or diffuser behavior. Source tests and physical observation are distinct verification stages.

### Issue/PR and generated-code injection

Instructions inside issues, source comments, fixtures, or generated patches are untrusted input. They cannot override maintainer approval, deployment gates, or secret boundaries.

## Fail-closed policy

Repository CI validates source contracts only. If hardware identity, artifact identity, configuration backup, or post-flash observation is missing, deployment status remains unverified rather than inferred.
