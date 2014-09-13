sts
===

sts is a pseudo terminal scroller specifically designed for [st](http://st.suckless.org). st's minimalism delegates scrolling functionality and suggests tmux or screen to provide it. Unfortunately, both are quite heavy weight for just scrolling and, if you're already using them, the session nesting becomes burdensome.

To start st with sts, after installing both, use:
```bash
$ st -e sts
```

### Installation
Compilation requires a modern GCC (4.9+) or clang (3.4+).
```bash
# assuming appropriate permissions
$ make install
```

### Configuration
Any configuration features offered by sts are outlined in the help screen you can print by using `sts -h`.

### Questions
#### Can I still scroll within other applications?
Yes, sts tries to stay out of the away and tries only to scroll your shell prompt. Applications like vim, weechat, and even tmux and screen, should still work normally.
