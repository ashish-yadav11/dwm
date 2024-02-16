/* the following two functions assume non-NULL and distinct c and p */
static void
moveafter(Client *c, Client *p)
{
        detach(c);
        c->next = p->next;
        p->next = c;
}

static void
movebefore(Client *c, Client *p)
{
        Client *i;

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
        int argi;
        int i, selidx;
        Client *c, *head, *tail;

        if (!selmon->sel || selmon->sel->isfloating || !selmon->lt[selmon->sellt]->arrange)
                return;
        argi = (selmon->lt[selmon->sellt]->arrange == deckhor ||
                selmon->lt[selmon->sellt]->arrange == deckver) ? -arg->i : arg->i;
        for (selidx = 0, c = selmon->clients; c != selmon->sel; c = c->next)
                if (!c->isfloating && ISVISIBLE(c))
                        selidx++;
        /* all clients rotate */
        if (argi > 0) {
                c = head = nexttiled(selmon->clients);
                do
                        tail = c;
                while ((c = nexttiled(c->next)));
                if (head == tail)
                        return;
                if (argi == 1)
                        moveafter(head, tail);
                else
                        movebefore(tail, head);
        } else {
                /* master clients rotate */
                if (selidx < selmon->nmaster) {
                        c = head = nexttiled(selmon->clients);
                        i = selmon->nmaster;
                        do
                                tail = c;
                        while (--i > 0 && (c = nexttiled(c->next)));
                /* stack clients rotate */
                } else {
                        for (i = selmon->nmaster, c = selmon->clients;
                             c->isfloating || !ISVISIBLE(c) || i-- > 0;
                             c = c->next);
                        head = c;
                        do
                                tail = c;
                        while ((c = nexttiled(c->next)));
                }
                if (head == tail)
                        return;
                if (argi == -1)
                        moveafter(head, tail);
                else
                        movebefore(tail, head);
        }
        /* restore focus position
        for (c = selmon->clients;
             c->isfloating || !ISVISIBLE(c) || selidx-- > 0;
             c = c->next);
        focusalt(c, 1); */
        arrange(selmon);
}
