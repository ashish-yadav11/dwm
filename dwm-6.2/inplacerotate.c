static void
moveafter(Client *c, Client *p)
{
        if (c == p)
                return;
        detach(c);
        c->next = p->next;
        p->next = c;
}

static void
movebefore(Client *c, Client *p)
{
        Client *i;

        if (c == p)
                return;
        detach(c);
        if (p == selmon->clients)
                attach(c);
        else {
                for (i = selmon->clients; i->next != p; i = i->next);
                i->next = c;
                c->next = p;
        }
}

static void
inplacerotate(const Arg *arg)
{
        int i, selidx;
	Client *c;
        Client *mhead, *mtail, *shead, *stail;

	if (!selmon->sel || selmon->sel->isfloating)
                return;
        /* determine mhead, mtail, shead, stail and selidx */
        c = mhead = nexttiled(selmon->clients);
	for (i = 0; c; c = nexttiled(c->next), i++) {
		if (c == selmon->sel)
                        selidx = i;
                if (i == selmon->nmaster)
                        shead = c;
                else if (i == selmon->nmaster - 1)
                        mtail = c;
		stail = c;
	}
        /* rotate */
        switch (selmon->lt[selmon->sellt]->arrange == deck ? -arg->i : arg->i) {
                case 1: /* all clients rotate anticlockwise */
                        moveafter(mhead, stail);
                        break;
                case 2: /* all clients rotate clockwise */
                        movebefore(stail, mhead);
                        break;
                case -1: /* stack xor master clients rotate anticlockwise */
                        selidx < selmon->nmaster ? moveafter(mhead, mtail) : moveafter(shead, stail);
                        break;
                case -2: /* stack xor master clients rotate clockwise */
                        selidx < selmon->nmaster ? movebefore(mtail, mhead) : movebefore(stail, shead);
                        break;
        }
	/* restore focus position */
	for (i = 0, c = nexttiled(selmon->clients); c && i != selidx; c = nexttiled(c->next), i++);
        focusalt(c);
	arrange(selmon);
}
