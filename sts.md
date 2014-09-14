STS 1 "VERSION" Linux "User Manuals"
=======================================

NAME
----
sts - simple terminal scroller

SYNOPSIS
--------
`sts` [`-u`] [`-l` *buf_limit*] [`-s` *lines*]

DESCRIPTION
-----------
`sts` is a pseudo terminal scroller for the simple terminal st(1).

OPTIONS
-------
`-h`, `--help`
  Show the terse help screen and exit.

`-u`, `--unlimited`
  Enable unlimited backlog lines (`default`). (same as `-l 0`)

`-l` *buf_limit*, `--limit` *buf_limit*
  Specify the number of lines _beyond the screen_ `sts` should remember for scrolling. Default is 0, meaning there is no limit. (see `-u`)

`-s` *lines*, `--step` *lines*
  Specify the number of lines each scroll step should take. Default is 5.

`-v`, `--version`
  Show the version information and exit.

BUGS
----
Bugs, issues, and feature requests should be sent to the issue tracker on Github: <https://github.com/jeaye/sts/issues>.

AUTHOR
------
Jeaye <contact@jeaye.com>

LICENSE
------
`sts` is under the MIT license. See the _LICENSE_ file.
