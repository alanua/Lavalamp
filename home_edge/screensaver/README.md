# Home Edge generative screensaver integration

This package installs the local `home_edge/generative_visuals` renderer as a user-session visual screensaver without changing GNOME lock/authentication or display power policy.

## Runtime contract

`skeleton-generative-saver.py` polls GNOME Mutter idle time and GNOME ScreenSaver lock state over the existing session D-Bus. It starts the local renderer only when all of these are true:

- the session is unlocked;
- GNOME idle time is above `SKELETON_SCREENSAVER_IDLE_SECONDS`;
- the canonical media-display-ownership guard explicitly reports `CLEAR`;
- post-media/unlock grace and restart backoff have elapsed.

Any confirmed media owner immediately stops the renderer. An unknown media state fails closed and also stops/blocks the renderer. When media ownership releases, the normal idle policy resumes only after `SKELETON_SCREENSAVER_POST_MEDIA_GRACE_SECONDS`; the saver is not relaunched immediately on pause/stop/end.

The media guard path defaults to `~/.local/bin/home-edge-media-display-owner`. Its bounded protocol is:

- `guard status` exits `0`: confirmed video/media session owns the display;
- exits `1`: display explicitly clear for the idle saver;
- any other exit or missing/unexecutable guard: state `UNKNOWN`, saver must not start.

The guard is intentionally an integration hook rather than a second media detector. Home Edge must wire it to the canonical Skeleton Cast/player/media-session ownership adapter. It must expose only the state class through exit status; titles, URLs, accounts, session IDs and device identifiers are not consumed here.

## Renderer

`launch-renderer.sh` serves the installed package only on `127.0.0.1` and launches Chromium/Chrome in kiosk app mode against the local generative renderer. A runtime `flock` rejects duplicate concurrent renderers. The controller starts the launcher in its own process group and terminates the whole group on activity, lock, media preemption or shutdown.

## Install / status / rollback

From a checked-out Lavalamp source tree:

```sh
home_edge/screensaver/install.sh
home_edge/screensaver/status.sh
home_edge/screensaver/rollback.sh
```

The installer copies only this package and the generative renderer into `~/.local/lib/lavalamp-home-edge`, installs a `systemd --user` service, and creates a private environment file only when one does not already exist. It does not use `sudo`, does not modify GNOME lock settings, does not change DPMS/suspend policy, and does not install/remove packages.

Rollback disables the new user service and restores the one captured pre-install snapshot when present.

## Stock visual saver policy

`status.sh` inventories known optional visual-saver packages/autostart entries such as `xscreensaver*` and `mate-screensaver`. Inventory is report-only. Package removal is deliberately not embedded in this source installer: the verified target must first prove which optional visual saver is actually installed, then any package mutation is performed later through the canonical audited Home Edge operation with rollback. GNOME Shell, GDM, authentication, lock-screen and power-management components are never removal candidates.

## Validation

Source validation requires:

```sh
python3 -m py_compile home_edge/screensaver/skeleton-generative-saver.py
bash -n home_edge/screensaver/launch-renderer.sh
bash -n home_edge/screensaver/install.sh
bash -n home_edge/screensaver/status.sh
bash -n home_edge/screensaver/rollback.sh
node --test tests/screensaver_integration.test.mjs
node --test tests/generative_visuals.test.mjs
git diff --check
```
