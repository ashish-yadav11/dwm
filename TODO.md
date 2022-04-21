# bugs?

* why no applygeomhints in configurerequest

* how would a client come back to normalstate after sendevent unmap (which sets
  it to withdrawn state) (unmapnotify)

* cleanup masks handling (no need of exposure, enter, should we have
  structurenotify on the client if we already have substructurenotify on the
  root), wmctrl might prove helpful in debugging after the changes

* see if there is any more scope of improvement or bugs to fix in systray
  implementation (see configurerequest - ICCCM and subtle wm, we might need to
  send synthetic configurenotify, also should we set border width to 0 when
  adding an icon, is property XA\_WM\_NORMAL\_HINTS handling required/is it
  handled correctly, integrate isshvalid with systray)

* improve handling of updatentiles

* update manpage


# upstreaming

* send patch regarding title padding bug in drw.c

* send patch regarding memory leaks in pertag (cleanupmon), gettextprop and
  setdesktopnames in dwm

* send patch: don't return while grabbing pointer in movemouse

* send patch to remove unnecessary XSetCloseDownMode in killclient

* send patch: why drawbars() on updatewmhints in propertynotify and not
  drawbar(c-\>mon)

* send patch: propertynotify, why default: break and why break instead of return
  and why not else if

* send patch why recover oldbw in unmanage, it becomes dirty when setfullscreen
  is called

* send patch: no need to check for override\_redirect in maprequest

* send patch: configurerequest typo else if
  (from 12ea925076c4f1c013502651b0be90c05e0febac commit)

* send improved systray, tabbed and other patches to dwm's site

* send patch: in tile function, why unsigned ints, size check won't be required
  if they were ints
