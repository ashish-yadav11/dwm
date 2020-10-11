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
        int marg;
        int i, selidx;
	Client *c, *head, *tail;

	if (!selmon->sel || selmon->sel->isfloating)
                return;
        marg = selmon->lt[selmon->sellt]->arrange == deck ? -arg->i : arg->i;
        /* all clients rotate */
        if (marg > 0) {
                head = nexttiled(selmon->clients);
                for (selidx = 0, c = head;
                     c != selmon->sel;
                     c = nexttiled(c->next), selidx++);
                do
                        tail = c;
                while ((c = nexttiled(c->next)));
                marg == 1 ? moveafter(head, tail) : movebefore(tail, head);
        } else {
                for (selidx = 0, c = nexttiled(selmon->clients);
                     c != selmon->sel;
                     c = nexttiled(c->next), selidx++);
                /* master clients rotate */
                if (selidx < selmon->nmaster) {
                        tail = c = head = nexttiled(selmon->clients);
                        i = 1;
                        while (i++ < selmon->nmaster && (c = nexttiled(c->next)))
                                tail = c;
                /* stack clients rotate */
                } else {
                        for (i = 0, c = nexttiled(selmon->clients);
                             i < selmon->nmaster;
                             c = nexttiled(c->next), i++);
                        head = c;
                        do
                                tail = c;
                        while ((c = nexttiled(c->next)));
                }
                marg == -1 ? moveafter(head, tail) : movebefore(tail, head);
        }
	/* restore focus position */
	for (i = selidx, c = nexttiled(selmon->clients);
             i > 0;
             c = nexttiled(c->next), i--);
        focusalt(c);
	arrange(selmon);
}
